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
    bool matched(false);
    try {
        CSqliteStatement fetchrun(
            db,
            "SELECT run.id AS run_id, number, title,                              \
                run_transitions.id AS transition_id, transition_type, time_stamp, \
                time_stamp, shift_id, short_comment, type                                     \
            FROM run_transitions                              \
            LEFT JOIN run ON run_id = transition_id           \
            INNER JOIN valid_transitions ON transition_type = valid_transitions.id \
            WHERE run_id = ?"
        );
        // Should  give me a record for each transition - and there must
        // be at least one.  Attached to the first record will be the
        // run information and the name of the transition will be attached
        //  to all (I think).
        // We'll have to get the shift separately from the shift_id
        // in each shift...that's just a matter of constructing a
        // LogBookShift:
        
        fetchrun.bind(1, id);                  // ID we're looking for
        while (! (++fetchrun).atEnd()) {
            matched = true;
            // if the run_id is not null, we can fetch the RunInfo chunk of this:
            
            if (fetchrun.columnType(0) != CSqliteStatement::null) {
                m_run.s_Info.s_id = fetchrun.getInt(0);
                m_run.s_Info.s_number = fetchrun.getInt(1);
                m_run.s_Info.s_title  = fetchrun.getString(2);
            }
            // Now we need to construct a Transition struct; fill it in
            // and add it to the vector of transitions in the run:
            
            m_run.s_transitions.emplace_back();
            Transition& t(m_run.s_transitions.back());
            t.s_id = fetchrun.getInt(3);
            t.s_transition = fetchrun.getInt(4);
            t.s_transitionName = fetchrun.getString(8);
            t.s_transitionTime = fetchrun.getInt(5);
            t.s_transitionComment = fetchrun.getString(7);
            t.s_onDuty     = new LogBookShift(db, fetchrun.getInt(6));
            
        }
    }
    catch (CSqliteException &e) {
        LogBook::Exception::rethrowSqliteException(e, "Constructing a LogBookRun");
    }
    
    // If we didn't have any matches to the query above,
    // we throw:
    
    if (!matched) {
        std::stringstream msg;
        msg << "There is no run with the primary key : " << id;
        std::string m(msg.str());
        throw LogBook::Exception(m);
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