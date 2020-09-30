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
#define private public
#include "LogBook.h"
#undef private
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

class shiftapitest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(shiftapitest);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST(create_3);
    CPPUNIT_TEST_SUITE_END();
protected:
    void create_1();
    void create_2();
    void create_3();
    
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(shiftapitest);

void shiftapitest::create_1()
{
    // Create empty shift:
    
    LogBookShift* shift = m_pLogBook->createShift("Day");
    EQ(1, shift->id());
    EQ(std::string("Day"), std::string(shift->name()));
    EQ(size_t(0), shift->members().size());
    
    delete shift;
}
void shiftapitest::create_2()
{
    // Shift with a member.
    
    auto members = m_pLogBook->findPeople("lastname='Fox'");
    auto shift   = m_pLogBook->createShift("Day", members);
    
    EQ(1, shift->id());
    EQ(std::string("Day"), std::string(shift->name()));
    EQ(size_t(1), shift->members().size());
    
    ASSERT(equals(*(members[0]), *(shift->members()[0])));
    
    delete shift;
    
}
void shiftapitest::create_3()
{
    // Entry made it into the database:
    
    auto members = m_pLogBook->findPeople("lastname='Fox'");
    auto shift   = m_pLogBook->createShift("Day", members);
    
    // Shift root:
    
    CSqliteStatement findShift(
        *m_pLogBook->m_pConnection,
        "SELECT name FROM shift WHERE id = ?"
    );
    findShift.bind(1, shift->id());
    ++findShift;
    ASSERT(!(findShift.atEnd()));
    EQ(std::string("Day"), findShift.getString(0));
    
    ++findShift;
    ASSERT(findShift.atEnd());   // only one row.
    
    // Shift members:
    
    CSqliteStatement findMembers(
        *m_pLogBook->m_pConnection,
        "SELECT person_id FROM shift_members WHERE shift_id = ?"
    );
    findMembers.bind(1, shift->id());
    ++findMembers;
    ASSERT(!(findMembers.atEnd()));
    
    EQ(members[0]->id(), findMembers.getInt(0));
    ++findMembers;
    ASSERT(findMembers.atEnd());
    
    delete shift;
}