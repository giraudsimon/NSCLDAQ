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

/** @file:  shiftapitest
 *  @brief: Test the LogBook class's shift API.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#include "logtestutils.hpp"

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char* nameTemplate="logbook.XXXXXX";

class aTestSuite : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(aTestSuite);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_filename;
    LogBook*    m_pLogBook;
public:
    // Create a logbook in a temp file with
    // some people in it:
    void setUp() {
        char filename[100];
        strncpy(filename, nameTemplate, sizeof(filename));
        int fd = mkstemp(filename);
        ASSERT(fd >= 0);
        close(fd);
        unlink(filename);
        
        m_filename = filename;
        LogBook::create(filename, "0400x", "Ron Fox", "Testing logbook");
        m_pLogBook = new LogBook(filename);
        
        // Add people to the logbook.
        
        delete m_pLogBook->addPerson("Fox", "Ron", "Mr.");
        delete m_pLogBook->addPerson("Cerizza", "Giordano", "Dr.");
        delete m_pLogBook->addPerson("Liddick", "Sean", "Dr.");
        delete m_pLogBook->addPerson("Gade", "Alexandra", "Dr.");
        delete m_pLogBook->addPerson("Weisshaar", "Dirk", "Dr.");
    }
    void tearDown() {
        delete m_pLogBook;
        unlink(m_filename.c_str());
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(aTestSuite);

void aTestSuite::test_1()
{
}