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


class runtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(runtest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST_SUITE_END();
    
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
protected:
    void construct_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(runtest);

void runtest::construct_1()
{
    // constructing no such run -> LogBook::Exception
    
    CPPUNIT_ASSERT_THROW(
        LogBookRun run(*m_db, 1),
        LogBook::Exception        // Note does not test form of queries.
    );
}