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
        time_t t = time(nullptr);
        transition.bind(3, (int)(t));
        auto shift = LogBookShift::getCurrent(db);
        if (!shift) {
            throw LogBook::Exception(
                "Cannot do a state transition because there's no current shift"
            );
        }
        transition.bind(4, shift->id());
        delete shift;
        transition.bind(5, note, -1, SQLITE_STATIC);
        ++transition;
        
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