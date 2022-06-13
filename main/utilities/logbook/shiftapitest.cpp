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
    
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    
    CPPUNIT_TEST(list_1);
    CPPUNIT_TEST(list_2);
    
    CPPUNIT_TEST(find_1);
    CPPUNIT_TEST(find_2);
    
    CPPUNIT_TEST(current_1);
    CPPUNIT_TEST(current_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void create_1();
    void create_2();
    void create_3();
    
    void get_1();
    void get_2();
    
    void list_1();
    void list_2();
    
    void find_1();
    void find_2();
    
    void current_1();
    void current_2();
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
void shiftapitest::get_1()
{
    // Get with no such:
    
    CPPUNIT_ASSERT_THROW(
        m_pLogBook->getShift(1),
        LogBook::Exception
    );
}

void shiftapitest::get_2()
{
    // Actually can get a shift if there is one:
    
    auto members = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto shift   = m_pLogBook->createShift("Day", members);
    
    LogBookShift* found;
    CPPUNIT_ASSERT_NO_THROW(found = m_pLogBook->getShift(shift->id()));
    
    EQ(shift->id(), found->id());
    EQ(std::string(shift->name()), std::string(found->name()));
    auto smembers =  shift->members();
    auto fmembers = found->members();
    
    EQ(smembers.size(), fmembers.size());
    for (int i =0; i < smembers.size(); i++) {
        ASSERT(equals(*smembers[i], *fmembers[i]));
    }
    
    delete shift;
    delete found;
}
void shiftapitest::list_1()
{
    // No shifts to list:
    
    auto shifts = m_pLogBook->listShifts();
    
    EQ(size_t(0), shifts.size());
}
void shiftapitest::list_2()
{
    // Make a couple of shifts:
    
    auto members1 = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto members2 = m_pLogBook->findPeople("lastname = 'Fox'");
    
    auto s1 = m_pLogBook->createShift("shift1", members1);
    auto s2 = m_pLogBook->createShift("shift2", members2);
    
    std::vector<LogBookShift*> madeShifts;
    madeShifts.push_back(s1);
    madeShifts.push_back(s2);
    
    auto shifts = m_pLogBook->listShifts();
    EQ(madeShifts.size(), shifts.size());
       
    
    for (int i = 0; i < madeShifts.size(); i++) {
        // Given the tests done on the shifts class, this is
        // probably sufficient:
        
        EQ(madeShifts[i]->id(), shifts[i]->id());
        
        delete madeShifts[i];
        delete shifts[i];
    }
    
}

void shiftapitest::find_1()
{
    // no matching shift:
    
    auto members1 = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto members2 = m_pLogBook->findPeople("lastname = 'Fox'");
    
    auto s1 = m_pLogBook->createShift("shift1", members1);
    auto s2 = m_pLogBook->createShift("shift2", members2);
    
    LogBookShift* p = m_pLogBook->findShift("shift3");
    ASSERT(!p);
    
    delete s1;
    delete s2;
}
void shiftapitest::find_2()
{
    auto members1 = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto members2 = m_pLogBook->findPeople("lastname = 'Fox'");
    
    auto s1 = m_pLogBook->createShift("shift1", members1);
    auto s2 = m_pLogBook->createShift("shift2", members2);
    
    LogBookShift* p = m_pLogBook->findShift("shift2");
    ASSERT(p);
    
    EQ(s2->id(), p->id());   // Assume shiftests checked the rest.
    
    delete s1;
    delete s2;    
}

void shiftapitest::current_1()
{
    // No current shift implies a nullptr from get:
    
    ASSERT(!(m_pLogBook->getCurrentShift()));
    
}
void shiftapitest::current_2()
{
    // Setting a current shift implies we can get it back:
    
    auto members1 = m_pLogBook->findPeople("salutation = 'Dr.'");
    auto members2 = m_pLogBook->findPeople("lastname = 'Fox'");
    
    auto s1 = m_pLogBook->createShift("shift1", members1);
    auto s2 = m_pLogBook->createShift("shift2", members2);
    
    m_pLogBook->setCurrentShift(s2);
    auto current = m_pLogBook->getCurrentShift();
    ASSERT(current);
    EQ(s2->id(), current->id());
    
    
    delete s1;
    delete s2;
    delete current;
}