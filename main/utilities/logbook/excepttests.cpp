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

/** @file:  excepttests.cpp
 *  @brief: Test LogBook::Exception class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "LogBook.h"
#include <CSqliteException.h>
#include <sqlite3.h>


class excepttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(excepttest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    
    CPPUNIT_TEST(copy_1);
    
    CPPUNIT_TEST(assign_1);
    
    CPPUNIT_TEST(rethrow_1);
    CPPUNIT_TEST(rethrow_2);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    
    void copy_1();
    
    void assign_1();
    
    void rethrow_1();
    void rethrow_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(excepttest);

void excepttest::construct_1()
{
    LogBook::Exception e("Testing");
    EQ(std::string("Testing"), std::string(e.what()));
    
}
void  excepttest::construct_2()
{
    std::string msg = ("Test message");
    LogBook::Exception e(msg);
    EQ(msg, std::string(e.what()));
    
}
void excepttest::construct_3()
{
    std::string doing("testing");
    CSqliteException serr(SQLITE_ERROR);
    LogBook::Exception e(serr, doing.c_str());
    std::string expected("CSqlite Exception caught while testing : SQL error or missing database");
    
    EQ(expected, std::string(e.what()));
}
void excepttest::construct_4()
{
    std::string doing("testing");
    CSqliteException serr(SQLITE_ERROR);
    LogBook::Exception e(serr, doing);
    std::string expected("CSqlite Exception caught while testing : SQL error or missing database");
    
    EQ(expected, std::string(e.what()));
}
void excepttest::copy_1()
{
    std::string msg = ("Test message");
    LogBook::Exception e(msg);
    
    LogBook::Exception c(e);
    EQ(msg, std::string(c.what()));
}

void excepttest::assign_1()
{
    std::string msg = ("Test message");
    LogBook::Exception e(msg);
    
    LogBook::Exception c("junk");
    c = e;
    
    EQ(msg, std::string(c.what()));
}

void excepttest::rethrow_1()
{
    CSqliteException serr(SQLITE_ERROR);
    bool threw(false);
    try {
        LogBook::Exception::rethrowSqliteException(serr, "Testing");
    }
    catch (LogBook::Exception& e) {
        threw = true;
        std::string expected(
            "CSqlite Exception caught while Testing : SQL error or missing database"
        );
        EQ(expected, std::string(e.what()));
    }
    catch (...) {
        CPPUNIT_FAIL("Incorrect exception");
    }
    ASSERT(threw);
}
void excepttest::rethrow_2()
{
    CSqliteException serr(SQLITE_ERROR);
    bool threw(false);
    try {
        std::string msg("Testing");
        LogBook::Exception::rethrowSqliteException(serr, msg);
    }
    catch (LogBook::Exception& e) {
        threw = true;
        std::string expected(
            "CSqlite Exception caught while Testing : SQL error or missing database"
        );
        EQ(expected, std::string(e.what()));
    }
    catch (...) {
        CPPUNIT_FAIL("Incorrect exception");
    }
    ASSERT(threw);
}