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

/** @file:  LogBookRun.h
 *  @brief: Manage runs in the logbookl.
 */
#ifndef LOGBOOKRUN_H
#define LOGBOOKRUN_H
#include <string>
#include <vector>

class CSqlite;
class LogBookShift;


/**
 * @class LogBookRun
 *   A logBookRun object encapsulates the entire history
 *   of a run and its state transitions.  Note that
 *   notes by users are not part of this class. That's
 *   something that's handled by the LogBookNote class.
 *
 *   At the top level, a run has, in addition to its primary
 *   key a run number and a title. Attached to that run
 *   are transitions.  When a run is created a Begin run transition
 *   is automatically added. Therefore, the static 'begin' method
 *   is what is used to create a new run (rather than the more normal
 *   create method).  Once begun the run becomes the current run
 *   and remains so until the run is ended either by an emergency end
 *   or by a normal end operation.  In addition to all of this there
 *   are helper tables that provide the set of transition arcs and,
 *   given the last arc (which defines the current state) which
 *   subsequent arcs are allowed (e.g. defines a finite state
 *   machines for the runs).
 *   To recap, the set of tables directly used by runs are:
 *
 *   *   run   - The runs
 *   *   run_transitions - the history of state transition arcs
 *                         for each run.
 *   *   valid_transitions - The Text to transition id lookup
 *                         table that defines all state transition
 *                         arcs.
 *   *   valid_state_transitions -the table that defines, for
 *                       each last arc the valid subsequent arcs
 *    *  current_run - documents which run is active.
 *    
 *    Runs also document which shift was active for each state transition
 *    and, as such, use the current_shift table.  Note that
 *    if there is no current shift transitions are illegal.
 */
class LogBookRun {
public:
    typedef struct _RunInfo {
        int         s_id;
        int         s_number;
        std::string s_title;
    } RunInfo, *pRunInfo;
    typedef struct _Transition {
        int         s_id;
        int         s_transition;
        std::string s_transitionName;
        int         s_transitionTime;
        std::string s_transitionComment;
        LogBookShift* s_onDuty;
    } Transition, *pTransition;
    typedef struct _Run {
        RunInfo                 s_Info;
        std::vector<Transition> s_transitions;
    } Run, *pRun;
private:
    Run     m_run;
public:
    LogBookRun(CSqlite& db, int id);
    virtual ~LogBookRun(); 

    const RunInfo& getRunInfo() const;
    bool  isCurrent(CSqlite& db) const;
    const Transition& operator[](int n) const;
    size_t numTransitions() const;
    int    lastTransitionType() const;
    const char* lastTransition() const;
    bool isActive() const;
    void transition(CSqlite& db, const char* type, const char* note);
    
    
    //
    static LogBookRun* currentRun(CSqlite& db);
    static int runId(CSqlite& db, int runNumber);
    static int begin(
        CSqlite& db,  int number,
        const char* title, const char* remark=nullptr
    );
    static void  end (
        CSqlite& db,  int runid,
        const char* remark=nullptr
    );
    static void  pause (
        CSqlite& db,  int runid,
        const char* remark=nullptr
    );
    static void  resume (
        CSqlite& db,  int runid,
        const char* remark=nullptr
    );
    static void  emergency_end (
        CSqlite& db,  int runid,
        const char* remark
    );
    static std::vector<LogBookRun*> list(CSqlite& db);
    static LogBookRun* find(CSqlite& db, int runNumber);
    
    
    static bool isLegal(CSqlite& db, int runId, int proposedTransition);
    
private:
    static bool checkTransition(CSqlite& db, int from, int  to);
    static int  addTransition(
        CSqlite& db, int run_id, int shift_id, const char* to, const char* note
    );
    static int currentShiftId(CSqlite& db);
    static void doTransition(
        CSqlite& db, int runid, const char* to, const char* remark=nullptr
    );
    
};

#endif