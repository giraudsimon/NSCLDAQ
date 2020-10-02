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
#include <iostream>
#include "logtestutils.hpp"

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
    
    CPPUNIT_TEST(remove_1);
    CPPUNIT_TEST(remove_2);
    CPPUNIT_TEST(remove_3);
    
    CPPUNIT_TEST(list_1);
    CPPUNIT_TEST(list_2);
    CPPUNIT_TEST(list_3);
    
    CPPUNIT_TEST(find_1);
    CPPUNIT_TEST(find_2);
    
    CPPUNIT_TEST(construct_1);
    
    CPPUNIT_TEST(current_1);
    CPPUNIT_TEST(current_2);
    CPPUNIT_TEST(current_3);
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
    
    void remove_1();
    void remove_2();
    void remove_3();
    
    void list_1();
    void list_2();
    void list_3();
    
    void find_1();
    void find_2();
    
    void construct_1();
    
    void current_1();
    void current_2();
    void current_3();
private:
    std::string m_filename;
    LogBook*    m_pLogBook;
public:
    // Create a logbook in a temp file with
    // some people in it:
    void setUp() {
        m_filename = tempFilename(nameTemplate);
        
        LogBook::create(m_filename.c_str(), "0400x", "Ron Fox", "Testing logbook");
        m_pLogBook = new LogBook(m_filename.c_str());
        
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

void shifttest::remove_1()
{
    // Remove if not exists is a throw:
    
    auto nonmembers = m_pLogBook->findPeople("lastname='Fox'");
    auto members    = m_pLogBook->findPeople("salutation = 'Dr.'");
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    // This will leak nonmembers person(s)
    
    CPPUNIT_ASSERT_THROW(
        shift->removeMember(*m_pLogBook->m_pConnection, nonmembers[0]),
        LogBook::Exception
    );
    
    delete shift;
}
void shifttest::remove_2()
{
    // Remove of existing member is fine and removes the member
    // from the member vector:
    
    auto members    = m_pLogBook->findPeople("salutation = 'Dr.'");
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    
    CPPUNIT_ASSERT_NO_THROW(
        shift->removeMember(*m_pLogBook->m_pConnection, members[0])
    );
    
    auto dbMembers = shift->members();
    EQ(members.size() - 1, dbMembers.size());
    
    for (int i =0; i < dbMembers.size(); i++) {
        ASSERT(members[0] != dbMembers[i]);
    }
}
void shifttest::remove_3()
{
    // ensure remove also gets it out of the shift_members table.
    
    // Remove of existing member is fine and removes the member
    // from the member vector:
    
    auto members    = m_pLogBook->findPeople("salutation = 'Dr.'");
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight", members
    );
    
    CPPUNIT_ASSERT_NO_THROW(
        shift->removeMember(*m_pLogBook->m_pConnection, members[0])
    );
    
    CSqliteStatement find(
        *m_pLogBook->m_pConnection,
        "SELECT person_id FROM shift_members WHERE shift_id = ?"
    );
    find.bind(1, shift->id());
    std::set<int> dbMembers;
    while(!((++find).atEnd())) {
        dbMembers.insert(find.getInt(0));
    }
    
    EQ(size_t(0), dbMembers.count(members[0]->id()));
}
void shifttest::list_1()
{
    // No shifts is an empty list:
    
    auto shifts = LogBookShift::list(*m_pLogBook->m_pConnection);
    EQ(size_t(0), shifts.size());
}
void shifttest::list_2()
{
    // Ther's a shift and it has the Drs. in it.
    // We should be able to list it.
    
    auto members = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto s = LogBookShift::create(
        *m_pLogBook->m_pConnection,
        "Midnight", members
    );
    
    auto shifts = LogBookShift::list(*m_pLogBook->m_pConnection);
    EQ(size_t(1), shifts.size());
    EQ(members.size(), shifts[0]->members().size());
    for (int i =0; i < members.size(); i++) {
        ASSERT(equals(*members[i], *(shifts[0]->members()[i])));
    }
    EQ(std::string("Midnight"), std::string(shifts[0]->name()));
    EQ(1, shifts[0]->id());
    
    delete shifts[0];
    delete s;
}
void shifttest::list_3()
{
    // Two shifts. one with me, the other with the Drs.
    
    auto drs = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto me  = m_pLogBook->findPeople("salutation = 'Mr.'");
    
    auto s1 =  LogBookShift::create(
        *m_pLogBook->m_pConnection,
        "Midnight", me
    );
    auto s2 =  LogBookShift::create(
        *m_pLogBook->m_pConnection,
        "Swing", drs
    );
    
    auto shifts = LogBookShift::list(*m_pLogBook->m_pConnection);
    
    EQ(size_t(2), shifts.size());
    EQ(std::string("Midnight"), std::string(shifts[0]->name()));
    
    EQ(me.size(), shifts[0]->members().size());
    for (int i =0; i < me.size(); i++) {
        ASSERT(equals(*(me[i]), *(shifts[0]->members()[i])));
    }
    
    EQ(std::string("Swing"), std::string(shifts[1]->name()));
    EQ(drs.size(), shifts[1]->members().size());
    for (int i =0; i < drs.size(); i++) {
        ASSERT(equals(*(drs[i]), *(shifts[1]->members()[i])));
    }
    
    delete shifts[0];
    delete shifts[1];
    delete s1;
    delete s2;
}
void shifttest::find_1()
{
    // no match -> nullptr.
    
    auto p = LogBookShift::find(*(m_pLogBook->m_pConnection), "Midnight");
    
    ASSERT(!p);
}
void shifttest::find_2()
{
    // found:
    
    auto me  = m_pLogBook->findPeople("salutation = 'Mr.'");
    
    auto shift =  LogBookShift::create(
        *m_pLogBook->m_pConnection,
        "Midnight", me
    );
    
    auto p = LogBookShift::find(*(m_pLogBook->m_pConnection), "Midnight");
    ASSERT(p);
    
    EQ(std::string("Midnight"), std::string(p->name()));
    auto members = p->members();
    
    EQ(me.size(), members.size());
    for (int i =0; i < me.size(); i++) {
        ASSERT(equals(*(me[i]), *(members[i])));       
    }
    delete p;
    delete shift;
}

void shifttest::construct_1()
{
    // Create an empty shift -- construct should find it:
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight"
    );
    LogBookShift* fshift;
    CPPUNIT_ASSERT_NO_THROW(
        fshift = new LogBookShift(*m_pLogBook->m_pConnection, shift->id())
    );
    
    EQ(shift->id(), fshift->id());
    EQ(std::string(shift->name()), std::string(fshift->name()));
    EQ(size_t(0), fshift->members().size());
    
    delete shift;
    delete fshift;
}

void shifttest::current_1()
{
    // Initially there's no current shift:
    
    auto shift = LogBookShift::getCurrent(*m_pLogBook->m_pConnection);
    ASSERT(!shift);
}
void shifttest::current_2()
{
    // Setting a current shift sets the databaswe:
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight"
    );
    shift->setCurrent(*m_pLogBook->m_pConnection);
    
    CSqliteStatement find(
        *m_pLogBook->m_pConnection,
        "SELECT shift_id FROM current_shift"
    );
    ++find;
    ASSERT(!find.atEnd());
    EQ(shift->id(), find.getInt(0));
    
    delete shift;
}
void shifttest::current_3()
{
    // setting a shift allows it to be gotten:
    
    auto shift = LogBookShift::create(
        *m_pLogBook->m_pConnection, "Midnight"
    );
    shift->setCurrent(*m_pLogBook->m_pConnection);
    
    auto current = LogBookShift::getCurrent(*m_pLogBook->m_pConnection);
    ASSERT(current);
    EQ(shift->id(), current->id());
    
    delete shift;
    delete current;
     
}