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

/** @file:  runapitests.cpp
 *  @brief: Test the LogBook Methods for manaing runs.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "LogBook.h"
#include "LogBookRun.h"
#undef private
#include "LogBookShift.h"

#include "logtestutils.hpp"
#include <string>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <set>


class runapitest : public CppUnit::TestFixture {
    
    
private:
    std::string m_filename;
    LogBook*    m_pLogbook;
    CSqlite*    m_db;
public:
    void setUp() {
        m_filename = tempFilename("logfile.XXXXXX");
        LogBook::create(m_filename.c_str(), "Fox", "Ron", "Testing");
        
        m_pLogbook = new LogBook(m_filename.c_str());
        m_db       = m_pLogbook->m_pConnection;
        
        // Make some people in the logbook:
        
        delete m_pLogbook->addPerson("Fox", "Ron", "Mr.");
        delete m_pLogbook->addPerson("Cerizza", "Giordano", "Dr.");
        delete m_pLogbook->addPerson("Liddick", "Sean", "Dr.");
        delete m_pLogbook->addPerson("Hughes", "Megan", "Ms.");
        
        // Make some shifts; set a current one.
        
        auto members1 = m_pLogbook->findPeople("salutation = 'Mr.' OR salutation = 'Ms.'");
        auto members2 = m_pLogbook->findPeople("salutation = 'Dr.'");
        
        auto current =  m_pLogbook->createShift("shift1", members1);
        delete          m_pLogbook->createShift("shift2", members2);
        m_pLogbook->setCurrentShift(current);
        delete current;
        
        
    }
    void tearDown() {
        delete m_pLogbook;
        unlink(m_filename.c_str());
    }
private:
    CPPUNIT_TEST_SUITE(runapitest);
    CPPUNIT_TEST(runId_1);             // Finds one.
    CPPUNIT_TEST(runId_2);             // no such throws.
    
    CPPUNIT_TEST(current_1);           // THere is a current run.
    CPPUNIT_TEST(current_2);           // there is no current run.
    
    CPPUNIT_TEST(begin_1);             // Legal begin.
    CPPUNIT_TEST(begin_2);             // illegal begin
    
    CPPUNIT_TEST(end_1);               // Legal end.
    CPPUNIT_TEST(end_2);               // Illegal end.
    
    CPPUNIT_TEST(pause_1);
    CPPUNIT_TEST(pause_2);
    
    CPPUNIT_TEST(resume_1);
    CPPUNIT_TEST(resume_2);
    
    CPPUNIT_TEST(emergency_1);
    CPPUNIT_TEST(emergency_2);
    
    CPPUNIT_TEST(list_1);             // None
    CPPUNIT_TEST(list_2);             // several/
    
    CPPUNIT_TEST(find_1);
    CPPUNIT_TEST(find_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void runId_1();
    void runId_2();
    
    void current_1();
    void current_2();
    
    void begin_1();
    void begin_2();
    
    void end_1();
    void end_2();
    
    void pause_1();
    void pause_2();
    
    void resume_1();
    void resume_2();
    
    void emergency_1();
    void emergency_2();
    
    void list_1();
    void list_2();
    
    void find_1();
    void find_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(runapitest);

void runapitest::runId_1()
{
    // Make a run the old fashioned way
    
    int id = LogBookRun::begin(*m_db, 12, "This is a title", "this is a remark");
    int foundId;
    CPPUNIT_ASSERT_NO_THROW(
        foundId = m_pLogbook->runId(12)
    );
    EQ(id, foundId);
}
void runapitest::runId_2()
{
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->runId(12),
        LogBook::Exception
    );
}

void runapitest::current_1()
{
    int id = LogBookRun::begin(*m_db, 12, "This is a title", "this is a remark");
    auto pRun = m_pLogbook->currentRun();
    ASSERT(pRun);
    
    EQ(id, pRun->getRunInfo().s_id);
}
void runapitest::current_2()
{
    int id = LogBookRun::begin(*m_db, 12, "This is a title", "this is a remark");
    LogBookRun::end(*m_db, id, "This is the end of the run as we know it");
    
    auto pRun = m_pLogbook->currentRun();
    ASSERT(!pRun);
    
}
void runapitest::begin_1()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    EQ(12, pRun->getRunInfo().s_number);
    delete pRun;
}
void runapitest::begin_2()
{
    auto pRun = m_pLogbook->begin(1, "A title", "a REMARK");
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->begin(1, "A title", "Duplicate run!!"),
        LogBook::Exception
    );
    delete pRun;
    
}
void runapitest::end_1()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    CPPUNIT_ASSERT_NO_THROW(
        m_pLogbook->end(pRun, "Ending the run")
    );
    ASSERT(!pRun->isActive());
    ASSERT(!m_pLogbook->currentRun());
    delete pRun;
}
void runapitest::end_2()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    m_pLogbook->end(pRun, "Ending the run");
    
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->end(pRun, "Ending the run"),
        LogBook::Exception
    );
    delete pRun;
}
void runapitest::pause_1()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    CPPUNIT_ASSERT_NO_THROW(
        m_pLogbook->pause(pRun, "Pausing the run")
    );
    EQ(std::string("PAUSE"), std::string(pRun->lastTransition()));
    delete pRun;
}
void runapitest::pause_2()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    m_pLogbook->pause(pRun, "Pausing the run");
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->pause(pRun, "Illegal pause the run"),
        LogBook::Exception
    );
    delete pRun;
}
void runapitest::resume_1()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    m_pLogbook->pause(pRun, "Pausing the run");
    CPPUNIT_ASSERT_NO_THROW(
        m_pLogbook->resume(pRun, "Resuming the run")
    );
    EQ(std::string("RESUME"), std::string(pRun->lastTransition()));
    delete pRun;
}
void runapitest::resume_2()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->resume(pRun, "Illegal resume - run not paused"),
        LogBook::Exception
    );
    delete pRun;
}

void runapitest::emergency_1()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    CPPUNIT_ASSERT_NO_THROW(
        m_pLogbook->emergencyStop(pRun, "Stopping due to an emergency")
    );
    ASSERT(!pRun->isActive());
    delete pRun;
}
void runapitest::emergency_2()
{
    auto pRun = m_pLogbook->begin(12, "A title", "a REMAKR");
    m_pLogbook->end(pRun, "Normal end");
    CPPUNIT_ASSERT_THROW(
        m_pLogbook->emergencyStop(
            pRun, "Illegal emergency end - run already stoppped"
        ),
        LogBook::Exception
    );
    delete pRun;
}
void runapitest::list_1()
{
    EQ(size_t(0), m_pLogbook->listRuns().size());
}
void runapitest::list_2()
{
    std::set<int> created;
    for (int i =0; i < 10; i++) {
        auto pRun = m_pLogbook->begin(i, "A title", "Beginning a run");
        m_pLogbook->end(pRun, "Ending");
        created.insert(pRun->getRunInfo().s_id);
        delete pRun;
    }
    
    auto runs = m_pLogbook->listRuns();
    std::set<int> listed;
    for (int i =0; i < runs.size(); i++) {
        listed.insert(runs[i]->getRunInfo().s_id);
        delete runs[i];
    }
    
    ASSERT(created == listed);
}

void runapitest::find_1()
{
    auto pRun = m_pLogbook->findRun(1);   // no such.
    ASSERT(!pRun);
}
void runapitest::find_2()
{

    for (int i =0; i < 10; i++) {
        auto pRun = m_pLogbook->begin(i, "A title", "Beginning a run");
        m_pLogbook->end(pRun, "Ending");
        delete pRun;
    }
    auto pRun = m_pLogbook->findRun(5);
    ASSERT(pRun);
    EQ(5, pRun->getRunInfo().s_number);
}