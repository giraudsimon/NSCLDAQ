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

/** @file:  bookstatictests.cpp
 *  @brief: Tests for static methods of LogBook.
 *  
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "LogBook.h"
#undef private
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include "logtestutils.hpp"


static const char* pFileTemplate="logbook.XXXXXX";

static std::string dirpath()
{
    const char* home = getenv("HOME");
    ASSERT(home != nullptr);
    
    std::string path(home);
    path += "/";
    path += ".nscl-logbook";
    return path;
}

class bookstaticstest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(bookstaticstest);
    CPPUNIT_TEST(tempdir_1);
    CPPUNIT_TEST(tempdir_2);
    
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST(create_3);
    CPPUNIT_TEST(create_4);
    CPPUNIT_TEST(create_5);
    CPPUNIT_TEST(create_6);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_dbName;
public:
    void setUp() {
        rmdir(dirpath().c_str());  // In case logbook has been used.
        m_dbName = tempFilename(pFileTemplate);
        
    }
    void tearDown() {
        unlink(m_dbName.c_str());
    }
protected:
    void tempdir_1();
    void tempdir_2();
    
    void create_1();
    void create_2();
    void create_3();
    void create_4();
    void create_5();
    void create_6();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bookstaticstest);

void bookstaticstest::tempdir_1()
{
    
    
    auto path = dirpath();
    
    EQ(path, LogBook::m_tempdir);
}
void bookstaticstest::tempdir_2()
{
    auto path = LogBook::computeTempDir();
    int status = access(path.c_str(), F_OK);
    EQ(0, status);
}

void bookstaticstest::create_1()
{
    // New database is made.
    
    LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook test");
    
    int status = access(m_dbName.c_str(), F_OK);
    EQ(0, status);
}


void bookstaticstest::create_2()
{
    LogBook::create(
        m_dbName.c_str(), "0400x", "Ron Fox", "Logbook Test"
    );
    
    // database is initialized with the description
    
    CSqlite c(m_dbName.c_str(), CSqlite::readonly);
    CSqliteStatement s(
        c,
        "SELECT value FROM kvstore WHERE key=?"
    );
    s.bind(1, "experiment", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    std::string value(reinterpret_cast<const char*>(s.getText(0)));
    EQ(std::string("0400x"), value);
    
    s.reset();
    s.bind(1, "spokesperson", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    value = (reinterpret_cast<const char*>(s.getText(0)));
    EQ(std::string("Ron Fox"), value);
    
    s.reset();
    s.bind(1, "purpose", -1, SQLITE_STATIC);
    ++s;
    ASSERT(!s.atEnd());
    value = (reinterpret_cast<const char*>(s.getText(0)));
    EQ(std::string("Logbook Test"), value);
    
}

void bookstaticstest::create_3()
{
    // Create on an existing file not allowed.
    
    LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook test");
    
    CPPUNIT_ASSERT_THROW(
        LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook test"),
        LogBook::Exception
    );
}

void bookstaticstest::create_4()
{
    // Person table was created.
    
    LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook Test");
    CSqlite db(m_dbName.c_str(), CSqlite::readonly);
    CPPUNIT_ASSERT_NO_THROW(
        CSqliteStatement::execute(
            db,
            "SELECT * FROM person"
        )
    );                // Would throw if table does not exist.
}

void bookstaticstest::create_5()
{
    // Shift tables created:
    
    LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook Test");
    CSqlite db(m_dbName.c_str(), CSqlite::readonly);
    
    CPPUNIT_ASSERT_NO_THROW(
        CSqliteStatement::execute(
            db, "SELECT * FROM shift"
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        CSqliteStatement::execute(
            db, "SELECT * FROM shift_members"
        )
    );
}
void bookstaticstest::create_6()
{
    // current-shift table exists.
    
    LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Logbook test");
    CSqlite db(m_dbName.c_str(), CSqlite::readonly);
    
    
    CPPUNIT_ASSERT_NO_THROW(
        CSqliteStatement::execute(
            db, "SELECT * FROM current_shift"
        )
    );
}
