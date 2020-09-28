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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "LogBook.h"
#include "LogBookPerson.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <CSqlite.h>
#include <CSqliteStatement.h>


static const char* logbookname = "logbook.XXXXXX";

class personapitest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(personapitest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
std::string m_dbName;
    CSqlite*    m_pDb;
    std::vector<LogBookPerson*> m_matches;
public:
    void setUp() {
        char name[100];
        strncpy(name, logbookname, sizeof(name));
        int fd = mkstemp(name);
        close(fd);
        unlink(name);
        m_dbName = name;
        LogBook::create(name, "0400x", "Ron Fox", "Person tests");
        m_pDb = new CSqlite(name, CSqlite::readwrite);
    }
    void tearDown() {
        for (int i =0; i < m_matches.size(); i++) {
            delete m_matches[i];
        }
        m_matches.clear();
        delete m_pDb;
        unlink(m_dbName.c_str());
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(personapitest);

void personapitest::test_1()
{
}