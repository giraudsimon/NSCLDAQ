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

/** @file:  runtests.cpp
 *  @brief: Test the LogBookRun class implementation.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <stdexcept>
// Open up the guts of the next two objects:

#define private public
#include "LogBook.h"
#include "LogBookRun.h"
#undef private
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "logtestutils.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <set>

class runtest : public CppUnit::TestFixture {
    
    
private:
    std::string m_dbName;
    LogBook*    m_log;
    CSqlite*    m_db;
public:
    void setUp() {
        m_dbName = tempFilename("logbook.XXXXXX");
        LogBook::LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "LogBook Tests");
        m_log = new LogBook(m_dbName.c_str());
        
        // Make some people in the logbook:
        
        delete m_log->addPerson("Fox", "Ron", "Mr.");
        delete m_log->addPerson("Cerizza", "Giordano", "Dr.");
        delete m_log->addPerson("Liddick", "Sean", "Dr.");
        delete m_log->addPerson("Hughes", "Megan", "Ms.");
        
        // Make some shifts; set a current one.
        
        auto members1 = m_log->findPeople("salutation = 'Mr.' OR salutation = 'Ms.'");
        auto members2 = m_log->findPeople("salutation = 'Dr.'");
        
        auto current =  m_log->createShift("shift1", members1);
        delete          m_log->createShift("shift2", members2);
        m_log->setCurrentShift(current);
        delete current;
        
        m_db = m_log->m_pConnection;
    }
    void tearDown() {
        delete m_log;
        unlink(m_dbName.c_str());
    }
private:
    CPPUNIT_TEST_SUITE(runtest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    
    CPPUNIT_TEST(info_1);
    
    CPPUNIT_TEST(current_1);
    CPPUNIT_TEST(current_2);
    CPPUNIT_TEST(current_3);
    CPPUNIT_TEST(current_4);
    CPPUNIT_TEST(current_5);
    
    CPPUNIT_TEST(gettrans_1);
    CPPUNIT_TEST(gettrans_2);
    CPPUNIT_TEST(gettrans_3);
    
    CPPUNIT_TEST(last_1);
    CPPUNIT_TEST(last_2);
    
    CPPUNIT_TEST(active_1);
    CPPUNIT_TEST(active_2);
    CPPUNIT_TEST(active_3);
    
    CPPUNIT_TEST(transition_1);
    CPPUNIT_TEST(transition_2);      // Pause run.
    CPPUNIT_TEST(transition_3);      // valid resume paused run.
    CPPUNIT_TEST(transition_4);      // valid end paused run.
    CPPUNIT_TEST(transition_5);      // valid emergency end active.
    CPPUNIT_TEST(transition_6);      // valid emergency end paused
    // Resumed runs look like another state type:
    CPPUNIT_TEST(transition_7);     // Valid end of resumed run.
    CPPUNIT_TEST(transition_8);     // valid pause of resumed run.
    CPPUNIT_TEST(transition_9);     // valid emergency end of resumed run.
    // Invalid transitions - BEGIN 
    CPPUNIT_TEST(transition_10);    // BEGIN on BEGUN run.
    CPPUNIT_TEST(transition_11);    // BEGIN Ended run.
    CPPUNIT_TEST(transition_12);    // BEGIN Paused run.
    CPPUNIT_TEST(transition_13);    // BEGIN resumed run.
    CPPUNIT_TEST(transition_14);    // BEGIN Emergency ended run illegal.
    // Invalid transitions - PAUSE:
    CPPUNIT_TEST(transition_15);    // PAUSE Of PAUSED run illegal.
    CPPUNIT_TEST(transition_16);    // Pause of ended run illegal.
    CPPUNIT_TEST(transition_17);    // Pause of emergency ended run illegal.
    // Invalid transitions of RESUME:
    CPPUNIT_TEST(transition_18);    // Resume BEGIN run.
    CPPUNIT_TEST(transition_19);    // Resume ENDed run.
    CPPUNIT_TEST(transition_20);    // Resume EMERGENCY_ENDed run.
    CPPUNIT_TEST(transition_21);    // Resume resumed.
    // Invalid transitions of END:
    CPPUNIT_TEST(transition_22);    // Invalid to end an ended run.
    CPPUNIT_TEST(transition_23);    // Invalid to end an emergency ended run.
    // Invalid transitions of emergency end
    CPPUNIT_TEST(transition_24);   // ended.
    CPPUNIT_TEST(transition_25);   // Emergency ended.
    CPPUNIT_TEST(transition_26);   // invalid transition name:
    // Ended runs are no longer current:
    CPPUNIT_TEST(transition_27);    // normal end.
    CPPUNIT_TEST(transition_28);    // emergency end.
    
    CPPUNIT_TEST(runid_1);       // no runs always throws.
    CPPUNIT_TEST(runid_2);       // found with only one to chose fromm.
    CPPUNIT_TEST(runid_3);       // found with several to chose from - got the right one.
    CPPUNIT_TEST(runid_4);       // Not found with several to chose from.
    
    CPPUNIT_TEST(begin_1);      // Valid begin run.
    CPPUNIT_TEST(begin_2);      // Duplicate run .
    CPPUNIT_TEST(begin_3);      // NO current shift.
    CPPUNIT_TEST(begin_4);      // Check the begin did the right stuff.
    CPPUNIT_TEST(begin_5);      // THere's a current run.
    
    CPPUNIT_TEST(end_1);        // Valid end run.
    CPPUNIT_TEST(end_2);        // Run does not exist.
    CPPUNIT_TEST(end_3);        // RUn exists but is not current somehow.
    CPPUNIT_TEST(end_4);        // NOt valid to end (e.g. already ended).
    CPPUNIT_TEST(end_5);        // Check the end did the right stuff.
    
    // Do to the factorization - we can reduce the actual number of tests
    //  to code that can vary.
    
    CPPUNIT_TEST(pause_1);     // Valid pause.
    CPPUNIT_TEST(pause_2);     // Not current.
    CPPUNIT_TEST(pause_3);     // DId the right stuff.
    
    CPPUNIT_TEST(resume_1);    // Valid resume.
    CPPUNIT_TEST(resume_2);    // Not current.
    CPPUNIT_TEST(resume_3);    // Did the right stuff.
    
    CPPUNIT_TEST(emergencyend_1); // Valid emergency end.
    CPPUNIT_TEST(emergencyend_2); // not current (valid in this case).
    
    CPPUNIT_TEST(list_1);      // Nothing to list.
    CPPUNIT_TEST(list_2);      // one item to list.
    CPPUNIT_TEST(list_3);      // several items.
    
    // note that find depends on runId which is already throroughly tested
    // so these tests can be cursory.
    
    CPPUNIT_TEST(find_1);      // Match found.
    CPPUNIT_TEST(find_2);      // NO match found.
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    
    void info_1();
    
    void current_1();
    void current_2();
    void current_3();
    void current_4();
    void current_5();
    
    void gettrans_1();
    void gettrans_2();
    void gettrans_3();
    
    void last_1();
    void last_2();
    
    void active_1();
    void active_2();
    void active_3();
    
    void transition_1();
    void transition_2();
    void transition_3();
    void transition_4();
    void transition_5();
    void transition_6();
    void transition_7();
    void transition_8();
    void transition_9();
    void transition_10();
    void transition_11();
    void transition_12();
    void transition_13();
    void transition_14();
    void transition_15();
    void transition_16();
    void transition_17();
    void transition_18();
    void transition_19();
    void transition_20();
    void transition_21();
    void transition_22();
    void transition_23();
    void transition_24();
    void transition_25();
    void transition_26();
    void transition_27();
    void transition_28();
    
    void runid_1();
    void runid_2();
    void runid_3();
    void runid_4();
    
    void begin_1();
    void begin_2();
    void begin_3();
    void begin_4();
    void begin_5();
    
    void end_1();
    void end_2();
    void end_3();
    void end_4();
    void end_5();
    
    void pause_1();
    void pause_2();
    void pause_3();
    
    void resume_1();
    void resume_2();
    void resume_3();
    
    void emergencyend_1();
    void emergencyend_2();
    void emergencyend_3();
    
    void list_1();
    void list_2();
    void list_3();
    
    void find_1();
    void find_2();
private:
    int makeRun(int run, const char* title, const char* note);
    int addTransition(int runid, const char* type, const char* note);
};

CPPUNIT_TEST_SUITE_REGISTRATION(runtest);


int runtest::addTransition(int runid, const char* type, const char* note)
{
    // Add a transition to an existingt run root record:
    
    // Figure out the current shift:
    
    LogBookShift* pCurrent = m_log->getCurrentShift();
    ASSERT(pCurrent);
    int shift_id = pCurrent->id();
    delete pCurrent;
    
    // Insert the transition for BEGIN:
    
    CSqliteStatement insertBegin(
        *m_db,
        "INSERT INTO run_transitions                        \
        (run_id, transition_type, time_stamp, shift_id, short_comment) \
        VALUES (?, ?, ?, ?, ?)"
    );
    insertBegin.bind(1, runid);
    insertBegin.bind(2, LogBook::transitionId(*m_db, type));
    insertBegin.bind(3, time(nullptr));
    insertBegin.bind(4, shift_id);
    insertBegin.bind(5, note, -1, SQLITE_STATIC);
    ++insertBegin;

    return insertBegin.lastInsertId();
    
        
}
int runtest::makeRun(int run, const char* title, const char* note)
{
    // make a run at the current time with database actions.
    // Note:  Since we're in tests we don't bother with a transaction.
    // Returns the run id.
    
    // Insert root record:
    
    CSqliteStatement insertRoot(
        *m_db,
        "INSERT INTO run (number, title) VALUES (?,?)"
    );
    insertRoot.bind(1, run);
    insertRoot.bind(2, title, -1, SQLITE_STATIC);
    ++insertRoot;
    
    int run_id = insertRoot.lastInsertId();
    
    addTransition(run_id, "BEGIN", note);
    
    // This is now the current run too:
    
    CSqliteStatement delcurrent(
        *m_db,
        "DELETE FROM current_run"
    );
    ++delcurrent;
    CSqliteStatement inscurrent(
        *m_db,
        "INSERT INTO current_run (id) VALUES(?)"
    );
    inscurrent.bind(1, run_id);
    ++inscurrent;
    return run_id;
}

void runtest::construct_1()
{
    // constructing no such run -> LogBook::Exception
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun run(*m_db, 1),
        LogBook::Exception        // Note does not test form of queries.
    );
}

void runtest::construct_2()
{
    // construct with run is no exception:
    
    int id = makeRun(123, "Test Run", "This is a note for the test run");
    
    LogBookRun* pRun;
    CPPUNIT_ASSERT_NO_THROW(pRun = new LogBookRun(*m_db, id));
    
    delete pRun;
}
void runtest::construct_3()
{
    // Construct shoud give the right stuff:
    
    int id = makeRun(123, "Test Run", "This is a note for the test run");
    
    LogBookRun* pRun;
    CPPUNIT_ASSERT_NO_THROW(pRun = new LogBookRun(*m_db, id));
    
    EQ(id, pRun->m_run.s_Info.s_id);
    EQ(123, pRun->m_run.s_Info.s_number);
    EQ(std::string("Test Run"), pRun->m_run.s_Info.s_title);
    
    // There should be one transition:
    
    
    const std::vector<LogBookRun::Transition>& t(pRun->m_run.s_transitions);
    EQ(size_t(1), t.size());
    const LogBookRun::Transition& tr(t[0]);
    
    // It should be a begin run:
    
    EQ(std::string("BEGIN"), tr.s_transitionName);
    EQ(std::string("This is a note for the test run"), tr.s_transitionComment);
    auto shift = m_log->getCurrentShift();
    EQ(shift->id(), tr.s_onDuty->id());
    
    delete shift;
    delete pRun;
}
void runtest::construct_4()
{
    // Now add a transition (to end) and be sure it's there:
    
    int id   = makeRun(456, "Testing", "Add a second transition");
    int trid = addTransition(id, "END", "Ending the run now");
    
    LogBookRun run(*m_db, id);
    
    // Shoulod have two transtions:
    
    EQ(size_t(2), run.m_run.s_transitions.size());
    
    /// First is the begin second the end:
    
    EQ(std::string("BEGIN"), run.m_run.s_transitions[0].s_transitionName);
    
    // Second one matches our trid and is END
    
    EQ(trid, run.m_run.s_transitions[1].s_id);
    EQ(std::string("END"), run.m_run.s_transitions[1].s_transitionName);
    EQ(
        std::string("Ending the run now"),
        run.m_run.s_transitions[1].s_transitionComment
    );
}
void runtest::info_1()
{
    int id   = makeRun(456, "Testing", "Test info");
    LogBookRun run (*m_db, id);
    
    auto& info = run.getRunInfo();
    EQ(&info, const_cast<const LogBookRun::RunInfo*>(&(run.m_run.s_Info)));
}

void runtest::current_1()
{
    // initially currentRun returns nullptr:
    
    ASSERT(!LogBookRun::currentRun(*m_db));
}
void runtest::current_2()
{
    // If there is a current run it will get returned by
    // currentRun:
    
    int id = makeRun(456, "Testing", "Will be current");
    LogBookRun* pRun = LogBookRun::currentRun(*m_db);
    
    EQ(id, pRun->m_run.s_Info.s_id);
    delete pRun;
}

void runtest::current_3()
{
    // If a run object is current, isCurrent returnst true:
    
    int id = makeRun(456, "Testing", "Will be current");
    LogBookRun run(*m_db, id);
    ASSERT(run.isCurrent(*m_db));
}
void runtest::current_4()
{
    // If another run is current isCurrent -> false:
    
    int id = makeRun(456, "Testing", "Will not be current");
    makeRun(1, "Testing", "will be current");
    
    LogBookRun run(*m_db, id);
    ASSERT(!run.isCurrent(*m_db));
}
void runtest::current_5()
{
    // If there is no current run a run cannot be considered current:
    
    
    int id = makeRun(456, "Testing", "Will be current");
    CSqliteStatement rmv(
        *m_db,
        "DELETE FROM current_run"
    );
    ++rmv;
    
    LogBookRun run(*m_db, id);
    ASSERT(!run.isCurrent(*m_db));
}

void runtest::gettrans_1()
{
    // Size is correct: - five transitions:
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "END", "Ending the run now");
    
    LogBookRun run (*m_db, id);
    
    EQ(size_t(5), run.numTransitions());
}

void runtest::gettrans_2()
{
    // Can get the transitions using the indexing operation:
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "END", "Ending the run now");
    LogBookRun run (*m_db, id);
    
    EQ(std::string("BEGIN"), run[0].s_transitionName);
    EQ(std::string("PAUSE"), run[1].s_transitionName);
    EQ(std::string("RESUME"), run[2].s_transitionName);
    EQ(std::string("PAUSE"), run[3].s_transitionName);
    EQ(std::string("END"),   run[4].s_transitionName);
    
}
void runtest::gettrans_3()
{
    // out of range indexing gives std::range_error
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "END", "Ending the run now");
    LogBookRun run (*m_db, id);
    
    
    CPPUNIT_ASSERT_THROW(
        const LogBookRun::Transition& t(run[5]),
        std::out_of_range
    );
}

void runtest::last_1()
{
    // Get the last transition id:
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    LogBookRun run(*m_db, id);
    
    EQ(LogBook::transitionId(*m_db, "PAUSE"), run.lastTransitionType());
    
}
void runtest::last_2()
{
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    LogBookRun run(*m_db, id);
    
    EQ(std::string("PAUSE"), std::string(run.lastTransition()));
}
void runtest::active_1()
{
    // active:
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    LogBookRun run(*m_db, id);
    ASSERT(run.isActive());
}
void runtest::active_2()
{
    // in active by virtua of normal END
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "END", "Normal end");
    LogBookRun run(*m_db, id);
    
    ASSERT(!run.isActive());
}
void runtest::active_3()
{
    // Inactive by virtual of an emergency end:
    
    int id   = makeRun(456, "Testing", "Several transitions:");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "RESUME", "resuming the run now");
    addTransition(id, "PAUSE", "pausing the run now");
    addTransition(id, "EMERGENCY_END", "something bad happened");
    LogBookRun run(*m_db, id);
    
    ASSERT(!run.isActive());
}
void runtest::transition_1()
{
    // Legal trisntion to end a run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "END", "Ending the run")
    );
    ASSERT(!run.isActive());           // No database fetch here.
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("END"));
    
    
}

void runtest::transition_2()
{
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "PAUSE", "Pausing the run")
    );
    
    ASSERT(run.isActive());
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("PAUSE"));
}

void runtest::transition_3()
{
    // Valid end of paused run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "RESUME", "Resuming run");
    );
    
    ASSERT(run.isActive());
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("RESUME"));
}
void runtest::transition_4()
{
    // Valid end paused run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "END", "Ending paused run");
    );
    
    ASSERT(!run.isActive());
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("END"));
}
void runtest::transition_5()
{
    // Valid emergency end of running run.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "EMERGENCY_END", "Emergecny end to active run")
    );
    
    ASSERT(!run.isActive());
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("EMERGENCY_END"));
}
void runtest::transition_6()
{
    // Emergency end paused run.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "EMERGENCY_END", "EMG end to paused run")
    );
    
    ASSERT(!run.isActive());
    
    CSqliteStatement fetchlast(
        *m_db,
        "SELECT type FROM run_transitions                       \
            INNER JOIN valid_transitions on transition_type = valid_transitions.id \
            WHERE run_id = ? ORDER BY run_transitions.id DESC LIMIT 1 \
        "
    );
    fetchlast.bind(1, id);
    ++fetchlast;
    ASSERT(!(fetchlast.atEnd()));
    EQ(fetchlast.getString(0), std::string("EMERGENCY_END"));
}
void runtest::transition_7()
{
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    run.transition(*m_db, "RESUME", "Resumed run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "END", "Ending resumed")
    );
    // By now we believe the database and object are getting updated.
}
void runtest::transition_8()
{
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    run.transition(*m_db, "RESUME", "Resumed run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "PAUSE", "Pausing resumed")
    );
}
void runtest::transition_9()
{
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing run");
    run.transition(*m_db, "RESUME", "Resumed run");
    
    CPPUNIT_ASSERT_NO_THROW(
        run.transition(*m_db, "EMERGENCY_END", "Emergency ending resumed")
    );
}

void runtest::transition_10()
{
    // invalid to begin an active run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "BEGIN", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_11()
{
    // invalid to begin an ended run.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "End run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "BEGIN", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_12()
{
    // Invalid to begin a paused run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "pause run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "BEGIN", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_13()
{
    // invalid to begin a resumed run.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "pause run");
    run.transition(*m_db, "RESUME", "continuing run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "BEGIN", "Illegal"),
        LogBook::Exception
    );
}

void runtest::transition_14()
{
    // invalid to begin an emergency ended run.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "ended badly");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "BEGIN", "Illegal"),
        LogBook::Exception
    );
}

void runtest::transition_15()
{
    // ILlegal to pause paused run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "pause run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "PAUSE", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_16()
{
    // illegal to pause an ended run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "end run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "PAUSE", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_17()
{
    // illegal to pause an emergency ended run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "end run");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "PAUSE", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_18()
{
    // resume active.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "RESUME", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_19()
{
    // Resume resumed.
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "PAUSE", "Pausing");
    run.transition(*m_db, "RESUME", "Resuming");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "RESUME", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_20()
{
    // Resume end'
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "Ending");
    
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "RESUME", "Illegal"),
        LogBook::Exception
    );
}
void runtest::transition_21()
{
    // resume Emergency end:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "Ending");
    
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "RESUME", "Illegal"),
        LogBook::Exception
    );
}

void runtest::transition_22()
{
    // Invalid to end an ended run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "Ending");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "END", "Illegal"),
        LogBook::Exception
    );
    
}
void runtest::transition_23()
{
    // Invalid to end an emergency ended run:
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "Ending");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "END", "Illegal"),
        LogBook::Exception
    );
    
}

void runtest::transition_24()
{
    // Invalid to e_end an ended run:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "Ending");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "EMERGENCY_END", "Illegal"),
        LogBook::Exception
    );
    
}
void runtest::transition_25()
{
    // Invalid to e-end an emergency ended run:
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "Ending");
    
    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "EMERGENCY_END", "Illegal"),
        LogBook::Exception
    );
    
}
void runtest::transition_26()
{
    // Invalid transition name:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);

    CPPUNIT_ASSERT_THROW(
        run.transition(*m_db, "INVALID", "Illegal"),
        LogBook::Exception
    );
}

void runtest::transition_27()
{
    // normally ended runs are not current:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "END", "Normal End");
    
    ASSERT(!run.isCurrent(*m_db));
    ASSERT(!LogBookRun::currentRun(*m_db));
}
void runtest::transition_28()
{
    // abnormally rended runs are not current:
    
    int id = makeRun(456, "Testing", "Active -> end");
    LogBookRun run(*m_db, id);
    run.transition(*m_db, "EMERGENCY_END", "Normal End");
    
    ASSERT(!run.isCurrent(*m_db));
    ASSERT(!LogBookRun::currentRun(*m_db));
}

void runtest::runid_1()
{
    // NO runs a find always fails:
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::runId(*m_db, 1),
        LogBook::Exception
    );
}
void runtest::runid_2()
{
    // Found with only one choice:
    
    int id = makeRun(456, "Testing", "Testing find");
    int found;
    CPPUNIT_ASSERT_NO_THROW(
        found = LogBookRun::runId(*m_db, 456)
    );
    EQ(id, found);
}
void runtest::runid_3()
{
    // found from serveral:
    
    int id;
    makeRun(1, "Testing", "Testing find");
    makeRun(2, "Testing", "Testing find");
    id = makeRun(3, "Testing", "This one");
    makeRun(4, "Testing", "Testing find");
    makeRun(5, "Testing", "Testing find");
    
    int found = LogBookRun::runId(*m_db, 3);
    EQ(found, id);
}
void runtest::runid_4()
{
    // Not found amongst several:
    
    int id;
    makeRun(1, "Testing", "Testing find");
    makeRun(2, "Testing", "Testing find");
    id = makeRun(3, "Testing", "This one");
    makeRun(4, "Testing", "Testing find");
    makeRun(5, "Testing", "Testing find");
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::runId(*m_db, 7),
        LogBook::Exception
    );
}
void runtest::begin_1()
{
    // Valid begin run:
    
    CPPUNIT_ASSERT_NO_THROW(
        LogBookRun::begin(*m_db, 1, "This is a title", "This is a note")
    );
    
}
void runtest::begin_2()
{
    // Duplicate run:
    
    LogBookRun::begin(*m_db, 1, "This is a title", "This is a note");
    CPPUNIT_ASSERT_THROW(
        LogBookRun::begin(*m_db, 1, "This is a title", "This is a note"),
        LogBook::Exception
    );
}
void runtest::begin_3()
{
    // No current shift.
    
    CSqliteStatement rmcur(
        *m_db,
        "DELETE FROM current_shift"
    );
    ++rmcur;
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::begin(*m_db, 1, "This is a title", "This is a note"),
        LogBook::Exception
    );
}
void runtest::begin_4()
{
    // The run was properly made:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun run(*m_db, id);
    
    ASSERT(run.isCurrent(*m_db));
    ASSERT(run.isActive());
    const LogBookRun::RunInfo& info(run.getRunInfo());
    EQ(id, info.s_id);
    EQ(123, info.s_number);
    EQ(std::string("This is a title"), info.s_title);
    EQ(size_t(1), run.numTransitions());
    const LogBookRun::Transition& t(run[0]);
    EQ(std::string("BEGIN"), t.s_transitionName);
    EQ(std::string("This is a note"), t.s_transitionComment);
    
    auto shift = m_log->getCurrentShift();
    int shiftId = shift->id();
    delete shift;
    EQ(shiftId, t.s_onDuty->id());
    
}
void runtest::begin_5()
{
    // Existing runs must be closed first:
    
    LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    CPPUNIT_ASSERT_THROW(
        LogBookRun::begin(*m_db, 124, "This is a title", "This is a note"),
        LogBook::Exception
    );
}
void runtest::end_1()
{
    // Ending the current and running run is valid:
    
    int id = LogBookRun::begin(*m_db, 123, "THis is a title", "This is a note");
    CPPUNIT_ASSERT_NO_THROW(
        LogBookRun::end(*m_db, id, "Ending the run")
    );
}
void runtest::end_2()
{
    // ending a nonexistent run throws:
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::end(*m_db, 123, "ENding"),
        LogBook::Exception
    );
}
void runtest::end_3()
{
   // Run exists but is not current:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    makeRun(1234, "ANother run", "Another note");  // Displaces current
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::end(*m_db, id, "Ending the run"),
        LogBook::Exception
    );

}
void runtest::end_4()
{
    // Can't run an already ended run:
    //
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::end(*m_db, id, "Ending the run");
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::end(*m_db, id, "Already ended"),
        LogBook::Exception
    );
}
void runtest::end_5()
{
    // end run does the right things to the run.
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::end(*m_db, id, "Ending the run");
    LogBookRun run(*m_db, id);                   // Fetches from db.
    
    // We don't need an exaustive check because we've already done this for
    // transaction which end relies on:
    
    EQ(size_t(2), run.numTransitions());
    EQ(std::string("END"), std::string(run.lastTransition()));
}
void runtest::pause_1()
{
    // Legal pause:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    CPPUNIT_ASSERT_NO_THROW(
        LogBookRun::pause(*m_db, id, "Pausing the run")
    );
}
void runtest::pause_2()
{
    // Not legal if not current:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    makeRun(333, "Another run", "Stealing currency");
    CPPUNIT_ASSERT_THROW(
        LogBookRun::pause(*m_db, id, "Pausing illegaly"),
        LogBook::Exception
    );
}
void runtest::pause_3()
{
    // Make sure everthing got done right:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::pause(*m_db, id, "Legal run pause");
    
    LogBookRun run(*m_db, id);
    EQ(size_t(2), run.numTransitions());
    const LogBookRun::Transition& t(run[1]);
    
    EQ(std::string("PAUSE"), std::string(run.lastTransition()));
    EQ(std::string("Legal run pause"), t.s_transitionComment);
    
}
void runtest::resume_1()
{
    // Legal resume:
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::pause(*m_db, id, "Legal run pause");
    
    CPPUNIT_ASSERT_NO_THROW(LogBookRun::resume(*m_db, id, "Legal resume"));
}
void runtest::resume_2()
{
    // Can't resume non current run:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::pause(*m_db, id, "Legal run pause");
    makeRun(333, "Another run", "Stealing currency");
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun::resume(*m_db, id, "Not legal - not current"),
        LogBook::Exception
    );
}
void runtest::resume_3()
{
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    LogBookRun::pause(*m_db, id, "Legal run pause");
    LogBookRun::resume(*m_db, id, "Legale resume run");
    
    LogBookRun run(*m_db, id);
    
    EQ(size_t(3), run.numTransitions());
    EQ(std::string("RESUME"), std::string(run.lastTransition()));
    EQ(
        std::string("Legale resume run"),
        std::string(run[2].s_transitionComment)
    );
}
void runtest::emergencyend_1()
{
    // Legal end of current run.
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    CPPUNIT_ASSERT_NO_THROW(
        LogBookRun::emergency_end(*m_db, id, "Emergency end")
    );
}
void runtest::emergencyend_2()
{
    // Non current run can be emergency ended:
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    makeRun(333, "Another run", "Stealing currency");
    CPPUNIT_ASSERT_NO_THROW(
        LogBookRun::emergency_end(*m_db, id, "Emergency end")
    );
}
void runtest::emergencyend_3()
{
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note"); 
    LogBookRun::emergency_end(*m_db, id, "Emergency end run");
    
    LogBookRun run(*m_db, id);
    EQ(size_t(2), run.numTransitions());
    EQ(std::string("EMERGENCY_END"), std::string(run.lastTransition()));
    EQ(
        std::string("Emergency end run"),
        std::string(run[1].s_transitionComment)
    );
}

void runtest::list_1()
{
    // Initially; there are no runs.
    
    auto  listing = LogBookRun::list(*m_db);
    ASSERT(listing.empty());
}
void runtest::list_2()
{
    // After we create a single run we should see it.
    
    int id = LogBookRun::begin(*m_db, 123, "This is a title", "This is a note");
    auto list = LogBookRun::list(*m_db);
    EQ(size_t(1), list.size());
    EQ(id, list[0]->getRunInfo().s_id);
    delete list[0];
}
void runtest::list_3()
{
    // After we create several runs we should see them:
 
    std::set<int> createdIds;
    for (int i =0; i < 10; i++) {
        int id = LogBookRun::begin(*m_db, i, "A run", "Begin");
        LogBookRun::end(*m_db, id);
        createdIds.insert(id);
    }
    auto list = LogBookRun::list(*m_db);
    
    // Must have the same set of ids:
    
    std::set<int> foundIds;
    for (int i =0; i < list.size(); i++) {
        int id = list[i]->getRunInfo().s_id;
        foundIds.insert(id);
        delete list[i];
    }
    
    ASSERT(createdIds == foundIds);
}
void runtest::find_1()
{
    // Find a run.l
    
    int id = LogBookRun::begin(*m_db, 1, "A RUN", "Begin");
    LogBookRun* pRun = LogBookRun::find(*m_db, 1);
    LogBookRun  run(*m_db, id);
    
    ASSERT(pRun);
    EQ(run.getRunInfo().s_id, pRun->getRunInfo().s_id);
    delete pRun;
}
void runtest::find_2()
{
    // no match:
    
    LogBookRun* pRun = LogBookRun::find(*m_db, 1);
    ASSERT(!pRun);
}