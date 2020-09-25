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

/** @file:  persontests.cpp
 *  @brief: UnitTests for LogBookPerson.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "LogBookPerson.h"
#include "LogBook.h"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static const char* logbookname = "logbook.XXXXXX";

class persontest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(persontest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST(create_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_dbName;
    CSqlite*    m_pDb;
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
        delete m_pDb;
        unlink(m_dbName.c_str());
    }
protected:
    void construct_1();
    void construct_2();
    
    void create_1();
    void create_2();
    void create_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(persontest);

void persontest::construct_1()
{
    // Stock the db with a person and construct from it:
    // Should be no exception.
    
    CSqliteStatement s(
        *m_pDb,
        "INSERT INTO person (lastname, firstname, salutation) \
            VALUES ('Fox', 'Ron', 'Mr.')"
    );
    ++s;
    int id = s.lastInsertId();

    CPPUNIT_ASSERT_NO_THROW(
        LogBookPerson p(*m_pDb, id)
    );
}
void persontest::construct_2()
{
    // correct data from construction:
    // Stock the db with a person and construct from it:
    // Should be no exception.
    
    CSqliteStatement s(
        *m_pDb,
        "INSERT INTO person (lastname, firstname, salutation) \
            VALUES ('Fox', 'Ron', 'Mr.')"
    );
    ++s;
    int id = s.lastInsertId();

    LogBookPerson p(*m_pDb, id);
    EQ(std::string("Fox"), std::string(p.lastName()));
    EQ(std::string("Ron"), std::string(p.firstName()));
    EQ(std::string("Mr."), std::string(p.salutation()));
    EQ(id, p.id());
}
void persontest::create_1()
{
    // Creation does not throw
    
     CPPUNIT_ASSERT_NO_THROW(
        delete LogBookPerson::create(*m_pDb, "Fox", "Ron", "Mr.")
    );
}
void persontest::create_2()
{
    // Creation creates the right person:
    
    LogBookPerson* p = LogBookPerson::create(*m_pDb, "Fox", "Ron", "Mr.");
    EQ(std::string("Fox"), std::string(p->lastName()));
    EQ(std::string("Ron"), std::string(p->firstName()));
    EQ(std::string("Mr."), std::string(p->salutation()));
    delete p;
}
void persontest::create_3()
{
    // Database entry actually made:

    delete LogBookPerson::create(*m_pDb, "Fox", "Ron", "Mr.");
    
    CSqliteStatement f(
        *m_pDb,
        "SELECT id, lastname, firstname, salutation FROM person"
    );
    ++f;
    ASSERT(!(f.atEnd()));
    
    EQ(1, f.getInt(0));
    EQ(std::string("Fox"), f.getString(1));
    EQ(std::string("Ron"), f.getString(2));
    EQ(std::string("Mr."), f.getString(3));
}