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

/** @file:  noteapitests.cpp
 *  @brief: Tests for the nots API in the LogBook class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "LogBookNote.h"
#include "LogBook.h"
#undef private

#include "LogBookShift.h"
#include "LogBookRun.h"

#include "CSqlite.h"
#include "CSqliteStatement.h"
#include "logtestutils.hpp"

#include <time.h>
#include <stdint.h>
#include <set>
#include <stdexcept>
#include <string.h>
#include <sstream>
#include <fstream>



class noteapitest : public CppUnit::TestFixture {
    
    
private:
    std::string m_filename;
    LogBook*    m_pLogbook;
    CSqlite*    m_db;
    LogBookRun* m_pRun;

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

        m_pRun = m_pLogbook->begin(123, "This is the run title", "This is the remark");
    }
    void tearDown() {
        delete m_pRun;
        delete m_pLogbook;
        unlink(m_filename.c_str());
  
    }
private:
    CPPUNIT_TEST_SUITE(noteapitest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(noteapitest);

void noteapitest::test_1()
{
}