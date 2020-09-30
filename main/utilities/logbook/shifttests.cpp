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

/** @file:  shiftests.cpp
 *  @brief: Test the LogBookShift class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <DebugUtils.h>
#define private public
#include "LogBook.h"
#include "LogBookPerson.h"
#include "LogBookShift.h"
#undef private

#include <CSqlite.h>
#include <CSqliteStatement.h>

#include <string>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set>

static const char* nameTemplate="logbook.XXXXXX";

class shifttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(shifttest);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST(create_3);
    CPPUNIT_TEST(create_4);
    CPPUNIT_TEST(create_5);
    CPPUNIT_TEST(create_6);
    CPPUNIT_TEST(create_7);
    
    CPPUNIT_TEST(add_1);
    CPPUNIT_TEST(add_2);
    CPPUNIT_TEST_SUITE_END();

protected:
    void create_1();
    void create_2();
    void create_3();
    void create_4();
    void create_5();
    void create_6();
    void create_7();
    
    void add_1();
    void add_2();
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

CPPUNIT_TEST_SUITE_REGISTRATION(shifttest);

void shifttest::create_1()
{
    // Create empty shift... should have an entry
    // in shifts for it.
    
    CPPUNIT_ASSERT_NO_THROW(
        delete LogBookShift::create(*(m_pLogBook->m_pConnection), "Midnight");
    );
}
void shifttest::create_2()
{
    // Empty of members:
    
    auto p = LogBookShift::create(*(m_pLogBook->m_pConnection), "Midnight");
    ASSERT(p->members().empty());
    EQ(1, p->id());
    EQ(std::string("Midnight"), std::string(p->name()));
    
    delete p;
}

void shifttest::create_3()
{
    // Duplicate fails:
    
    delete LogBookShift::create(*(m_pLogBook->m_pConnection), "Midnight");
    
    CPPUNIT_ASSERT_THROW(
        LogBookShift::create(*(m_pLogBook->m_pConnection), "Midnight"),
        LogBook::Exception
    );  
}
void shifttest::create_4()
{
    // Makes entry:
    
    delete LogBookShift::create(*(m_pLogBook->m_pConnection), "Midnight");
    CSqliteStatement find(
        *(m_pLogBook->m_pConnection),
        "SELECT id, name FROM shift WHERE name =?"
    );
    find.bind(1, "Midnight", -1, SQLITE_STATIC);
    ++find;
    ASSERT(!(find.atEnd()));
    EQ(1, find.getInt(0));
    EQ(std::string("Midnight"), find.getString(1));
    
    ++find;
    ASSERT(find.atEnd());               // ONly one.
    
}
void shifttest::create_5()
{
    // Create with one person:
    
    auto members = m_pLogBook->findPeople("lastname = 'Fox'");
    EQ(size_t(1), members.size());   //Sure we know what we're doing
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    EQ(members, shift->members());
    
    delete shift;     // Deletes members (ownership transfer).
}
void shifttest::create_6()
{
    // Multiple members.
    
    auto members = m_pLogBook->findPeople("salutation = 'Dr.'");
    EQ(size_t(4), members.size());
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    EQ(members, shift->members());
    
    delete shift;     // Deletes members (ownership transfer).
}
void shifttest::create_7()
{
    // members are added to shift_members
    
    auto members = m_pLogBook->findPeople("salutation = 'Dr.'");
    EQ(size_t(4), members.size());
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    
    CSqliteStatement find(
        *m_pLogBook->m_pConnection,
        "SELECT person_id FROM shift_members WHERE shift_id = ?"
    );
    find.bind(1, shift->id());
    std::set<int> dbMembers;        // Person ids in database.
    while(!((++find).atEnd())) {
        dbMembers.insert(find.getInt(0));  // Build a set of members.
    }
    
    // Ensure each and every member is in the set:
    
    for (int i =0; i < members.size(); i++) {
        EQ(size_t(1), dbMembers.count(members[i]->id()));
    }
}
void shifttest::add_1()
{
    // Add a person to an empty shift - they're added to the vector.
    
    auto members = m_pLogBook->findPeople("lastname = 'Fox'");
    auto shift   = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight"
    );
    
    shift->addMember(*m_pLogBook->m_pConnection, members[0]);
    
    EQ(members[0], shift->members()[0]);
    
    delete shift;     
}
void shifttest::add_2()
{
    // Adding a member to the shift adds it to shift_members
    
    auto members = m_pLogBook->findPeople("lastname = 'Fox'");
    auto shift   = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight"
    );
    
    shift->addMember(*m_pLogBook->m_pConnection, members[0]);
    
    CSqliteStatement find(
        *m_pLogBook->m_pConnection,
        "SELECT person_id FROM shift_members WHERE shift_id = ?"
    );
    find.bind(1, shift->id());
    
    ++find;
    ASSERT(!(find.atEnd()));
    
    EQ(members[0]->id(), find.getInt(0));
    
    delete shift;
}



