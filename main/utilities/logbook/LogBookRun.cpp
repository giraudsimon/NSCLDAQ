/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  LogBookRun.cpp
 *  @brief: Provide an interface to runs.
 */
#include "LogBookRun.h"
#include "LogBookShift.h"
#include "LogBook.h"

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <CSqliteTransaction.h>
#include <CSqliteException.h>
#include <sstream>
#include <string>
#include <time.h>

/**
 * constructor
 *    Given a run id fetches all the data associated with that
 *    run and constructs a new LogBookRun object.
 *  @param db   - references the database connection object.
 *  @param id   - primary key of the run in the run table.
 *  @throws     - LogBook::Exception if there's no such run.
 *  @throws     - LogBook::Exception if there were CSqlite Exceptions.
 */
LogBookRun::LogBookRun(CSqlite& db, int id)
{
    // Since there's at least one transition we can use the
    // following single. monster queryto get everything but the shift
    // details (I think).
    
    try {
        CSqliteStatement fetchrun(
            db,
            "SELECT number, title FROM run WHERE id = ?"
        );
        fetchrun.bind(1, id);
        ++fetchrun;
        if (fetchrun.atEnd()) {
            std::stringstream msg;
            msg << "There is no run with the primary key : " << id;
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
        m_run.s_Info.s_id = id;
        m_run.s_Info.s_number = fetchrun.getInt(0);
        m_run.s_Info.s_title  = fetchrun.getString(1);
        
        // Now get all the transitions for this run:
        
        CSqliteStatement fetchtrans(
            db,
            "SELECT run_transitions.id, transition_type, time_stamp, \
            shift_id, short_comment, type \
            FROM run_transitions                                          \
            INNER JOIN valid_transitions ON transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id ASC"
        );
        fetchtrans.bind(1, id);
        while (!(++fetchtrans).atEnd()) {
            m_run.s_transitions.emplace_back();
            Transition& t(m_run.s_transitions.back());
            t.s_id = fetchtrans.getInt(0);
            t.s_transition = fetchtrans.getInt(1);
            t.s_transitionTime = fetchtrans.getInt(2);
            t.s_onDuty         = new LogBookShift(db, fetchtrans.getInt(3));
            t.s_transitionComment = fetchtrans.getString(4);
            t.s_transitionName    = fetchtrans.getString(5);
        }
        
    }
    catch (CSqliteException &e) {
        LogBook::Exception::rethrowSqliteException(e, "Constructing a LogBookRun");
    }

}
/**
 * destructor
 *   We need to free the LogBookShift objects in our transitions:
 */
LogBookRun::~LogBookRun()
{
    for(int i =0; i < m_run.s_transitions.size(); i++) {
        delete m_run.s_transitions[i].s_onDuty;
    }
}
/**
 * runInfo
 *   Returns the run information:
 * @return const RunInfo&
 */
const LogBookRun::RunInfo&
LogBookRun::getRunInfo() const
{
    return m_run.s_Info;
}
/**
 * isCurrent
 *   Determines if this is the current run:
 */
bool
LogBookRun::isCurrent(CSqlite& db) const
{
    LogBookRun* pCurrent = currentRun(db);
    //  Note that the edge case that there is nocurrent run means this is not
    // the current run.
    
    bool result = pCurrent && (m_run.s_Info.s_id == pCurrent->m_run.s_Info.s_id);
    
    delete pCurrent;
    return result;
}

/**
 * numTransitions
 *   Returns the number of transitions:
 * @return size_t
 */
size_t
LogBookRun::numTransitions() const
{
    return m_run.s_transitions.size();
}
/**
 * operator[]
 *   Returns one of the transitions.
 *
 * @param index - index into the transitions vector to return.
 * @return const Transition&
 * @throw std::range_error if index out of range.
 */
const LogBookRun::Transition&
LogBookRun::operator[](int index) const
{
    return m_run.s_transitions.at(index);
}






/**
 *lastTransitionType
 *  Returns the id of the last (Most recent) transition type.  If you want the
 *  text name see last Transition.
 *
 * @return int
 */
int
LogBookRun::lastTransitionType() const
{
    return m_run.s_transitions.back().s_transition;
}
/**
 * lastTransition
 *   Returns the textual name of the last transition (e.g. BEGIN).
 *
 * @return const char*
 */
const char*
LogBookRun::lastTransition() const
{
    return m_run.s_transitions.back().s_transitionName.c_str();
}
/**
 * isActive
 *    -- nnly if the last transition was not "END" or EMERGENCY_END
 *  @return bool
 */
bool
LogBookRun::isActive() const
{
    std::string last=lastTransition();
    
    // DeMorgan's theorem applied:
    
    return (last != std::string("END")) && (last != std::string("EMERGENCY_END"));
}
/**
 * transition
 *    Add a transition to this run.  Note that in order to support
 *    post run corrections, we don't require that the run be
 *    current.
 * @param db - Reference to database connection.
 * @param type - Type of transition requested.
 * @param note - comment to attach to the transition.
 * @throws LogBook::Exception if the transition is forbidden by the current
 *               state of the object.
 */
void
LogBookRun::transition(CSqlite& db, const char* type, const char* note)
{
    int typeId = LogBook::transitionId(db, type);  // throws if invalid
    int fromId = lastTransitionType();
    if (!checkTransition(db, fromId, typeId)) {
        std::stringstream msg;
        msg << "Run  " << m_run.s_Info.s_number << " is not allowed to  perform a "
            << type << " since last transition was " << lastTransition();
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    
    // Do this in a transaction because, in addition to entering the
    // transition, we may need to remove this as the current run.
    
    CSqliteTransaction t(db);
    try {
        CSqliteStatement transition(
            db,
            "INSERT INTO run_transitions                       \
            (run_id, transition_type, time_stamp, shift_id, short_comment) \
            VALUES (?,?,?,?,?)"
        );
        transition.bind(1, m_run.s_Info.s_id);
        transition.bind(2, typeId);
        time_t now = time(nullptr);
        transition.bind(3, (int)(now));
        auto shift = LogBookShift::getCurrent(db);
        if (!shift) {
            throw LogBook::Exception(
                "Cannot do a state transition because there's no current shift"
            );
        }
        transition.bind(4, shift->id());
        transition.bind(5, note, -1, SQLITE_STATIC);
        ++transition;
        
        // Add the transition to the object's cache of the transition history.
        
        m_run.s_transitions.emplace_back();
        Transition& t(m_run.s_transitions.back());
        t.s_id = transition.lastInsertId();
        t.s_transition = typeId;
        t.s_transitionName = type;
        t.s_transitionTime = now;
        t.s_transitionComment = note;
        t.s_onDuty = shift;
        
        
        
        // Now if the transition was an END or EMERGENCY_END
        // and we are current, we need to set that there's no current run.
        
        if (
            ((std::string("END") == type) ||
            (std::string("EMERGENCY_END") == type)) &&
            isCurrent(db)
        ) {
            CSqliteStatement::execute(db, "DELETE FROM current_run ");
        }
    }
    catch(CSqliteException& e) {
        t.scheduleRollback();
        LogBook::Exception::rethrowSqliteException(
            e, "Attempting a run state transition"
        );
        
    }
    catch (...) {
        t.scheduleRollback();
        throw;
    }
    
}

///////////////////////////////////////////////////////////////
// static methods:


/**
 * currentRun
 *    Returns the current run object.
 * @param db - Database object reference.
 * @return LogBookRun* - dynamically allocated so delete if done with it.
 * @retval nullptr     - There is no currently active run.
 */
LogBookRun*
LogBookRun::currentRun(CSqlite& db)
{
    LogBookRun* result = nullptr;
    CSqliteStatement cid(
        db,
        "SELECT id FROM current_run"
    );
    ++cid;
    
    if (!cid.atEnd()) {
        result  = new LogBookRun(db, cid.getInt(0));
    }
    
    return result;
    
}
/**
 * runId
 *    Returns the run id that corresponds to a specific run numgber.
 * @param db - reference to the database connection object.
 * @param run - Run Number.
 * @return int - primary key of the id.
 * @throw LogBook::Exception - if there is no match
 * @note As a user may well look up a non-existent run, exceptions thrown by this
 *       method should be handled.  If what is "No such run nnn" this indicates
 *       there is no matching run. 
 */
int
LogBookRun::runId(CSqlite& db, int run)
{
    CSqliteStatement find(
        db,
        "SELECT id FROM run WHERE number = ?"
    );
    find.bind(1, run);
    ++find;
    if (find.atEnd()) {
        std::stringstream msg;
        msg << "No such run " << run;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    return find.getInt(0);
}
/**
 * begin
 *   Creates a new run in the 'begin' state.  Returns the id of that run.
 *
 * @param db   - references the database connection object.
 * @param number - run number.
 * @param title  - run title.
 * @param remark - remark to use for the run.  If null and empty string
 *                 is used.
 * @return id    - Id of the run's primary key (can be used to construct the obj).
 * @throw LogBook::Exception - if the run already exists.
 * @throw LogBook::Exception - if there's a failure inserting the run records.
 * @note   The run is inserted atomically.
 */
int
LogBookRun::begin(
    CSqlite& db, int number, const char* title, const char* remark
)
{
    const char* note = "";
    if (remark) note = remark;
    
    // Throw if the run already exists:
    
    bool exists(true);
    try {
        LogBookRun::runId(db, number);
    }
    catch (LogBook::Exception& e) {
        exists = false;
    }
    if (exists) {
        std::stringstream msg;
        msg << "Attempting to create a run that already exists: " << number;
        std::string m(msg.str());
        throw LogBook::Exception(m);
    }
    // Throw if there's a current run:
    
    auto pRun = currentRun(db);
    if (pRun) {
        std::stringstream msg;
        msg << "There is already an active run: run number: "
            << pRun->m_run.s_Info.s_number << " title: "
            << pRun->m_run.s_Info.s_title;
        std::string m(msg.str());
        delete pRun;
        throw LogBook::Exception(m);
    }
    
    // make the run and its initial transition->Begin in a transaction:
    
    CSqliteTransaction t(db);
    int run_id;
    try {
        CSqliteStatement insertRoot(
            db,
            "INSERT INTO run (number, title) VALUES (?,?)"
        );
        insertRoot.bind(1, number);
        insertRoot.bind(2, title, -1, SQLITE_STATIC);
        ++insertRoot;
        
        run_id = insertRoot.lastInsertId();
        
        int shift_id = currentShiftId(db);
        
        addTransition(db, run_id, shift_id, "BEGIN", note);
        
        // This is now the current run too:
        
        CSqliteStatement delcurrent(
            db,
            "DELETE FROM current_run"
        );
        ++delcurrent;
        CSqliteStatement inscurrent(
             db,
            "INSERT INTO current_run (id) VALUES(?)"
        );
        inscurrent.bind(1, run_id);
        ++inscurrent;
        return run_id;
    }
    catch(CSqliteException& e) {
        t.scheduleRollback();
        LogBook::Exception::rethrowSqliteException(e, "Making a new run");
    }
    catch(...) {
        t.scheduleRollback();
        throw;
    }
    return run_id;                  // Commits the transaction.
}
/**
 * end
 *    End the specified run. Run must be the current run and it must
 *    be legal to end the run.  The run will no longer be current after this
 *    call.
 * @param db  - database connection reference.
 * @param runid - Id of the run to end (Primary key; not run number).
 * @param remark - Remark to associate with the end of the run.
 *                (optional, defaults to no comment).
 * @throw LogBook::Exception -run id does not exist.
 * @throw LogBook::Exception - run is not current.
 * @throw LogBook::Exception - Run is not in the proper state to end.
 * 
 */
void
LogBookRun::end(CSqlite& db, int runid, const char* remark)
{
    
    doTransition(db, runid, "END", remark);
    
       
}
/**
 * pause
 *    Pause a run.  See end; all the same restrictions apply.
 *
 * @param db - database connection reference.
 * @param runid - id (primary key) of the run.
 * @param remark - the remark defaults to empty.
 */
void
LogBookRun::pause(CSqlite& db, int runid, const char* remark)
{
    doTransition(db, runid, "PAUSE", remark);
       
}
/**
 * resume:
 *
 *  @param db   - database.
 *  @param runid - run id.
 *  @param remark - comment (if null [default] empty string)
 */
void
LogBookRun::resume(CSqlite& db, int runid, const char* remark)
{
    doTransition(db, runid, "RESUME", remark);
}
/**
 * emergency_end
 *    Do an emergency end operation.
 * 
 *  @param db   - database.
 *  @param runid - run id.
 *  @param remark - comment (if null [default] empty string)
 */
void
LogBookRun::emergency_end(CSqlite& db, int runid, const char* remark)
{
    doTransition(db, runid, "EMERGENCY_END", remark);
}
/**
 * list
 *   Lists the runs
 *
 *  @param db - database reference.
 *  @return std::vector<LogBookRun*>  - all runs. Note that the
 *       pointer point to dynamically allocated runs and must therefore
 *       have delete applied to them
 */
std::vector<LogBookRun*>
LogBookRun::list(CSqlite& db)
{
    std::vector<LogBookRun*> result;
    
    CSqliteStatement list(
        db,
        "SELECT id FROM run"
    );
    while(!(++list).atEnd()) {
        int id = list.getInt(0);
        result.push_back(new LogBookRun(db, id));
    }
    
    return result;
}
 /**
  * find
  *    Returns the log book that matches the run number:
  *
  * @param db  - database reference.
  * @return LogBookRun* - pointer to the found object. Null if not found.
  */
 LogBookRun*
 LogBookRun::find(CSqlite& db, int run)
 {
    int id;
    LogBookRun* result;
    try {
        id = runId(db, run);
        result = new LogBookRun(db, id);
    }
    catch (...) {
        return nullptr;
    
    }
    return result;
 }
/////////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * checkTransition
 *   @param db   - reference to database connection
 *   @param from - from state
 *   @param to   - to state.
 *   @return bool - true if allowed.
 */
bool
LogBookRun::checkTransition(CSqlite& db, int from, int to)
{
    CSqliteStatement s(
        db,
        "SELECT COUNT(*) FROM valid_state_transitions    \
         WHERE from_id = ? AND to_id = ?"
    );
    s.bind(1, from);
    s.bind(2, to);
    ++s;
    int result = s.getInt(0);
    return result != 0;
}
/**
 * addTransition
 *    Does all the needful stuff to insert a transition record for a run
 *    The caller must have verified the existence of the run and the
 *    legality of the tranition.  This method does not create a transaction
 *    and therefore callers may need to have one running if this is part of
 *    a larger picture.
 *
 *  @param db  - references the database connection object.
 *  @param run_id - Primary key of the run that's having this transition added.
 *  @param shift_id - the id of the shift that's logging this transition.
 *  @param to     - Name of the transition edge.
 *  @param note  - transition note.
 *  @return int - the id of the transition (its primary key).
 */
int
LogBookRun::addTransition(
    CSqlite& db, int run_id, int shift_id,  const char* to, const char* note
)
{
    // Insert the transition record.
    
    CSqliteStatement insertTrans(
        db,
        "INSERT INTO run_transitions                        \
        (run_id, transition_type, time_stamp, shift_id, short_comment) \
        VALUES (?, ?, ?, ?, ?)"
    );
    insertTrans.bind(1, run_id);
    insertTrans.bind(2, LogBook::transitionId(db, to));
    insertTrans.bind(3, time(nullptr));
    insertTrans.bind(4, shift_id);
    insertTrans.bind(5, note, -1, SQLITE_STATIC);
    ++insertTrans;
    return insertTrans.lastInsertId();       
}
/**
 * currentShiftId
 *    Returns the curent shift id or throws if there isn't one.
 * @param db database connection object.
 * @return int - shift id.
 */
int
LogBookRun::currentShiftId(CSqlite& db)
{
    LogBookShift* pCurrent = LogBookShift::getCurrent(db);
    if (!pCurrent) {
        throw LogBook::Exception(
            "There's no current shift while trying to insert a transition"
        );
    }
    int shift_id = pCurrent->id();
    delete pCurrent;
    return shift_id;
}

/*
 * doTransition
 *   DO a transition for a run.  See end for more information
 * @param db  - database.
 * @param runid - id of the run.
 * @param to    - transition to attempt.
 * @param remark - run note, defaults to empty string
 */
void
LogBookRun::doTransition(
    CSqlite& db, int runid, const char* to, const char* remark
)
{
    const char* note="";
    if (remark) note = remark;
    
    // Does the run even exist (throws if no such)
    
    LogBookRun run(db, runid);
    
    // If the transition is not an emergency end it's only allowed
    // for the current run:
    
    if (std::string("EMERGENCY_END") != to) {
        if (!run.isCurrent(db)) {
            std::stringstream  msg;
            msg << "Attempting to " << to  << " run number "
                <<  run.getRunInfo().s_number << " but this run is not current";
            std::string m(msg.str());
            throw LogBook::Exception(m);
        }
    }
    // Now we can just perform the transition on the run:
    
    run.transition(db, to , note);
       
}