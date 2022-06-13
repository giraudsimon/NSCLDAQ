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

#include "logtestutils.hpp"

static const char* logbookname = "logbook.XXXXXX";

class personapitest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(personapitest);
    CPPUNIT_TEST(create_1);
    
    CPPUNIT_TEST(find_1);   // No where
    CPPUNIT_TEST(find_2);   // Where
    CPPUNIT_TEST(find_3);   // No match.
                        
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST_SUITE_END();
    
private:
std::string m_dbName;
    LogBook*                    m_book;
    std::vector<LogBookPerson*> m_matches;
public:
    void setUp() {
        m_dbName = tempFilename(logbookname);
        LogBook::create(m_dbName.c_str(), "0400x", "Ron Fox", "Person tests");
        m_book = new LogBook(m_dbName.c_str());
    }
    void tearDown() {
        for (int i =0; i < m_matches.size(); i++) {
            delete m_matches[i];
        }
        m_matches.clear();
        delete m_book;
        unlink(m_dbName.c_str());
    }
protected:
    void create_1();
    
    void find_1();
    void find_2();
    void find_3();
    
    void get_1();
    void get_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(personapitest);

void personapitest::create_1()
{
    auto person = m_book->addPerson("Fox", "Ron", "Mr.");
    EQ(std::string("Fox"), std::string(person->lastName()));
    EQ(std::string("Ron"), std::string(person->firstName()));
    EQ(std::string("Mr."), std::string(person->salutation()));
    EQ(1, person->id());
    
    delete person;
}
void personapitest::find_1()
{
    // find with no WHERE;
    
    delete m_book->addPerson("Fox", "Ron", "Mr.");
    delete m_book->addPerson("Cerizza", "Giordano", "Dr.");
    
    m_matches = m_book->findPeople();
    EQ(size_t(2), m_matches.size());
    EQ(std::string("Ron"), std::string(m_matches[0]->firstName()));
    EQ(std::string("Dr."), std::string(m_matches[1]->salutation()));
    
}
void personapitest::find_2()
{
    // Selective match:
    
    delete m_book->addPerson("Fox", "Ron", "Mr.");
    delete m_book->addPerson("Cerizza", "Giordano", "Dr.");
    
    m_matches = m_book->findPeople("salutation = 'Dr.'");
    EQ(size_t(1), m_matches.size());
    EQ(std::string("Cerizza"), std::string(m_matches[0]->lastName()));
}
void personapitest::find_3()
{
    // no match
    
    delete m_book->addPerson("Fox", "Ron", "Mr.");
    delete m_book->addPerson("Cerizza", "Giordano", "Dr.");
    
    m_matches = m_book->findPeople("firstname = 'Alexandra'");
    EQ(size_t(0), m_matches.size());
}
void personapitest::get_1()
{
    // Get matching:
    
    delete m_book->addPerson("Fox", "Ron", "Mr.");
    auto giordano = m_book->addPerson("Cerizza", "Giordano", "Dr.");
    int id = giordano->id();
    delete giordano;
    
    LogBookPerson* p;
    CPPUNIT_ASSERT_NO_THROW(
        p = m_book->getPerson(id)
    );
    delete p;
}
void personapitest::get_2()
{
    // no match
    
    CPPUNIT_ASSERT_THROW(
        m_book->getPerson(1),
        LogBook::Exception
    );
}