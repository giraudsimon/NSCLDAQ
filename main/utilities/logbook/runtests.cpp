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