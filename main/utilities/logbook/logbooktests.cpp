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

/** @file: logbooktests.cpp
 *  @brief:  Test non-static methods of LogBook class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "LogBook.h"
#undef private

#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "logtestutils.hpp"

static const char* dbTemplate="logbook.XXXXXX";

class logtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(logtest);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_DbName;
public:
    void setUp() {
        m_DbName = tempFilename(dbTemplate);
        LogBook::create(m_DbName.c_str(), "0400x", "Ron Fox", "Test Logbook");
    }
    void tearDown() {
        rmdir(LogBook::m_tempdir.c_str());
        unlink(m_DbName.c_str());
    }
protected:
    void create_1();
    void create_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(logtest);

void logtest::create_1()
{
    CPPUNIT_ASSERT_NO_THROW(
        LogBook lb(m_DbName.c_str())
    );
}
void logtest::create_2()
{
    LogBook lb(m_DbName.c_str());
    
    // Now make sure we're connected to a good database:
    
    CSqlite* pCon = lb.m_pConnection;
    
    CSqliteStatement s(
        *pCon,
        "SELECT value FROM kvstore WHERE key=?"
    );
    
    s.bind(1, "experiment", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    std::string value = reinterpret_cast<const char*>(s.getText(0));
    EQ(std::string("0400x"), value);
    
    s.reset();
    s.bind(1, "spokesperson", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    value = reinterpret_cast<const char*>(s.getText(0));
    EQ(std::string("Ron Fox"), value);
    
    s.reset();
    s.bind(1, "purpose", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    value = reinterpret_cast<const char*>(s.getText(0));
    EQ(std::string("Test Logbook"), value);
    
}