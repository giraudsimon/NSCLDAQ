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
    LogBookPerson* m_pRon;
    LogBookPerson* m_pGiordano;

public:
    void setUp() {
         m_filename = tempFilename("logfile.XXXXXX");
        LogBook::create(m_filename.c_str(), "Fox", "Ron", "Testing");

        m_pLogbook = new LogBook(m_filename.c_str());
        m_db       = m_pLogbook->m_pConnection;

        // Make some people in the logbook:

        m_pRon =  m_pLogbook->addPerson("Fox", "Ron", "Mr.");
        m_pGiordano =  m_pLogbook->addPerson("Cerizza", "Giordano", "Dr.");
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
        delete m_pRon;
        delete m_pGiordano;
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
    
    CPPUNIT_TEST(listall_1);
    CPPUNIT_TEST(listall_2);
    
    CPPUNIT_TEST(listrun_1);
    CPPUNIT_TEST(listrun_2);
    CPPUNIT_TEST(listrun_3);
    CPPUNIT_TEST(listrun_4);
    CPPUNIT_TEST(listrun_5);
    CPPUNIT_TEST(listrun_6);
    
    CPPUNIT_TEST(listnonrun_1);
    CPPUNIT_TEST(listnonrun_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void create_1();
    void create_2();
    
    void listall_1();
    void listall_2();
    
    void listrun_1();
    void listrun_2();
    void listrun_3();
    void listrun_4();
    void listrun_5();
    void listrun_6();
    
    void listnonrun_1();
    void listnonrun_2();
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
            *m_pRon, "This is a note", images, offsets, m_pRun
        )
    );
    
    
    EQ(std::string("This is a note"), pNote->getNoteText().s_contents);
    EQ(m_pRon->id(), pNote->getNoteText().s_authorId);
    EQ(size_t(2), pNote->imageCount());
    EQ(m_filename, (*pNote)[0].s_originalFilename);
    EQ(std::string("/etc/group"), (*pNote)[1].s_originalFilename);
    
    delete pNote;
}

void noteapitest::create_2()
{
    //note without images:
    
    LogBookNote* pNote = m_pLogbook->createNote(*m_pGiordano, "This is a note", m_pRun);
    EQ(std::string("This is a note"), pNote->getNoteText().s_contents);
    EQ(m_pGiordano->id(), pNote->getNoteText().s_authorId);
    EQ(size_t(0), pNote->imageCount());
    
    delete pNote;
}

void noteapitest::listall_1()
{
    // nothing to list:
    
    EQ(size_t(0), m_pLogbook->listAllNotes().size());
}
void noteapitest::listall_2()
{
    //make a couple of notes and list all:
    
    delete m_pLogbook->createNote(*m_pRon, "This is a note", m_pRun);
    delete m_pLogbook->createNote(*m_pGiordano, "This is another note");
    
    auto notes = m_pLogbook->listAllNotes();
    EQ(size_t(2), notes.size());
    EQ(std::string("This is a note"), notes[0]->getNoteText().s_contents);
    EQ(std::string("This is another note"), notes[1]->getNoteText().s_contents);
}
void noteapitest::listrun_1()
{
    // NOthing to list associated with our run.. by id.
    
    delete  m_pLogbook->createNote(*m_pRon, "This is a note"); // no associated run.
    EQ(size_t(0), m_pLogbook->listNotesForRunId(m_pRun->getRunInfo().s_id).size());
}
void noteapitest::listrun_2()
{
    // list when there is a matching note
    
    auto pNote = m_pLogbook->createNote(*m_pGiordano, "This is a note", m_pRun);
    auto notes = m_pLogbook->listNotesForRunId(m_pRun->getRunInfo().s_id);
    
    EQ(size_t(1), notes.size());
    EQ(pNote->getNoteText().s_contents, notes[0]->getNoteText().s_contents);
    
    delete notes[0];
    delete pNote;
}
void noteapitest::listrun_3()
{
    delete  m_pLogbook->createNote(*m_pRon, "This is a note"); // no associated run.
    EQ(size_t(0), m_pLogbook->listNotesForRun(m_pRun->getRunInfo().s_number).size());
}
void noteapitest::listrun_4()
{
    auto pNote = m_pLogbook->createNote(*m_pGiordano, "This is a note", m_pRun);
    auto notes = m_pLogbook->listNotesForRun(m_pRun->getRunInfo().s_number);
    
    EQ(size_t(1), notes.size());
    EQ(pNote->getNoteText().s_contents, notes[0]->getNoteText().s_contents);
    
    delete notes[0];
    delete pNote;
}
void noteapitest::listrun_5()
{
    delete  m_pLogbook->createNote(*m_pRon, "Thi sis a note"); // no associated run.
    EQ(size_t(0), m_pLogbook->listNotesForRun(m_pRun).size());
}
void noteapitest::listrun_6()
{
    auto pNote = m_pLogbook->createNote(*m_pGiordano, "This is a note", m_pRun);
    auto notes = m_pLogbook->listNotesForRun(m_pRun);
    
    EQ(size_t(1), notes.size());
    EQ(pNote->getNoteText().s_contents, notes[0]->getNoteText().s_contents);
    
    delete notes[0];
    delete pNote;
}

void noteapitest::listnonrun_1()
{
    delete m_pLogbook->createNote(*m_pRon, "This is a note", m_pRun);
    EQ(size_t(0), m_pLogbook->listNonRunNotes().size());
}

void noteapitest::listnonrun_2()
{
    auto pNote = m_pLogbook->createNote(*m_pGiordano, "This is a note");
    auto notes = m_pLogbook->listNonRunNotes();
    
    EQ(size_t(1), notes.size());
    EQ(pNote->getNoteText().s_contents, notes[0]->getNoteText().s_contents);
    
    delete notes[0];
    delete pNote;
}