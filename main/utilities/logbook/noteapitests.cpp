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

/** @file:  noteapitests.cpp
 *  @brief: Tests for the nots API in the LogBook class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "LogBookNote.h"
#include "LogBook.h"
#undef private

#include "LogBookShift.h"
#include "LogBookRun.h"

#include "CSqlite.h"
#include "CSqliteStatement.h"
#include "logtestutils.hpp"

#include <time.h>
#include <stdint.h>
#include <set>
#include <stdexcept>
#include <string.h>
#include <sstream>
#include <fstream>



class noteapitest : public CppUnit::TestFixture {
    
    
private:
    std::string m_filename;
    LogBook*    m_pLogbook;
    CSqlite*    m_db;
    LogBookRun* m_pRun;

public:
    void setUp() {
         m_filename = tempFilename("logfile.XXXXXX");
        LogBook::create(m_filename.c_str(), "Fox", "Ron", "Testing");

        m_pLogbook = new LogBook(m_filename.c_str());
        m_db       = m_pLogbook->m_pConnection;

        // Make some people in the logbook:

        delete m_pLogbook->addPerson("Fox", "Ron", "Mr.");
        delete m_pLogbook->addPerson("Cerizza", "Giordano", "Dr.");
        delete m_pLogbook->addPerson("Liddick", "Sean", "Dr.");
        delete m_pLogbook->addPerson("Hughes", "Megan", "Ms.");

        // Make some shifts; set a current one.

        auto members1 = m_pLogbook->findPeople("salutation = 'Mr.' OR salutation = 'Ms.'");
        auto members2 = m_pLogbook->findPeople("salutation = 'Dr.'");
        auto current =  m_pLogbook->createShift("shift1", members1);
        delete          m_pLogbook->createShift("shift2", members2);
        m_pLogbook->setCurrentShift(current);
        delete current;

        m_pRun = m_pLogbook->begin(123, "This is the run title", "This is the remark");
    }
    void tearDown() {
        delete m_pRun;
        delete m_pLogbook;
        unlink(m_filename.c_str());
  
    }
private:
    // Note most of functionalt testing is at the level of the
    // LogBookNote class (notetests.cpp) we therefore only need
    // cursory testing here.
    
    CPPUNIT_TEST_SUITE(noteapitest);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void create_1();
    void create_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(noteapitest);

void noteapitest::create_1()
{
    // ensure that images are marhsalled properly:
    
    std::vector<std::string> images;
    images.push_back(m_filename);
    images.push_back("/etc/group");   // just some file.
    
    std::vector<size_t> offsets = {0, 15};
    
    LogBookNote* pNote;
    CPPUNIT_ASSERT_NO_THROW(
        pNote = m_pLogbook->createNote(
            "This is a note", images, offsets, m_pRun
        )
    );
    
    
    EQ(std::string("This is a note"), pNote->getNoteText().s_contents);
    EQ(size_t(2), pNote->imageCount());
    EQ(m_filename, (*pNote)[0].s_originalFilename);
    EQ(std::string("/etc/group"), (*pNote)[1].s_originalFilename);
    
    delete pNote;
}

void noteapitest::create_2()
{
    //note without images:
    
    LogBookNote* pNote = m_pLogbook->createNote("This is a note", m_pRun);
    EQ(std::string("This is a note"), pNote->getNoteText().s_contents);
    EQ(size_t(0), pNote->imageCount());
    
    delete pNote;
}