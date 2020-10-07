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

class notetest : public CppUnit::TestFixture {
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
    CPPUNIT_TEST_SUITE(notetest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    
    CPPUNIT_TEST(assocrun_1);
    CPPUNIT_TEST(assocrun_2);
    
    CPPUNIT_TEST(notetext_1);
    
    CPPUNIT_TEST(imagecount_1);
    CPPUNIT_TEST(imagecount_2);
    
    CPPUNIT_TEST(index_1);
    CPPUNIT_TEST(index_2);
    
    CPPUNIT_TEST(image_1);
    CPPUNIT_TEST(image_2);
    CPPUNIT_TEST(image_3);
    CPPUNIT_TEST(image_4);
    CPPUNIT_TEST_SUITE_END();
    

protected:
    void construct_1();
    void construct_2();
    void construct_3();
    
    void assocrun_1();
    void assocrun_2();
    
    void notetext_1();
    
    void imagecount_1();
    void imagecount_2();
    
    void index_1();
    void index_2();
    
    void image_1();
    void image_2();
    void image_3();
    void image_4();
    
};


CPPUNIT_TEST_SUITE_REGISTRATION(notetest);

void notetest::construct_1()
{
    // Construct if no-such failes:
    
    CPPUNIT_ASSERT_THROW(
        LogBookNote(*m_db, 1),
        LogBook::Exception
    );
}
void notetest::construct_2()
{
    // can find/construct a note with no images:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    
    LogBookNote* pNote(nullptr);
    CPPUNIT_ASSERT_NO_THROW(
        pNote = new LogBookNote(*m_db, ins.lastInsertId());
    );
    
    // Should be no images and the note  data should be correct:
    
    EQ(m_pRun->getRunInfo().s_id, pNote->m_textInfo.s_runId);
    EQ(now, pNote->m_textInfo.s_noteTime);
    EQ(std::string("This is the note text"), pNote->m_textInfo.s_contents);
    EQ(size_t(0), pNote->m_imageInfo.size());
    
    
    delete pNote;
}
void notetest::construct_3()
{
    // Construct a note with images attached>
    
    // Root record.
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
 
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 10);
    img.bind(3, "/usr/opt/fox/testing/test.img", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    img.reset();
    img.bind(2, 100);
    img.bind(3, "~/images/barney.img", -1, SQLITE_STATIC);
    ++img;
    
    std::set<int> offsets = {10,100};
    std::set<std::string> names = {"/usr/opt/fox/testing/test.img", "~/images/barney.img"};
    // Construct the note:
    
    LogBookNote note(*m_db, id);
    
    // Root record:
    
    EQ(id, note.m_textInfo.s_id);
    EQ(m_pRun->getRunInfo().s_id, note.m_textInfo.s_runId);
    EQ(now, note.m_textInfo.s_noteTime);
    EQ(std::string("This is the note text"), note.m_textInfo.s_contents);
    
    // For images we'll check the offsets and the image names of the
    // images as well as that we have two of them.
    
    EQ(size_t(2), note.m_imageInfo.size());
    std::set<int> foundOffsets;
    std::set<std::string> foundNames;
    for (int i = 0; i < 2; i++) {
        foundOffsets.insert(note.m_imageInfo[i].s_noteOffset);
        foundNames.insert(note.m_imageInfo[i].s_originalFilename);
        EQ(sizeof(randomData), note.m_imageInfo[i].s_imageLength);
    }
    ASSERT(offsets == foundOffsets);
    ASSERT(names == foundNames);
}

void notetest::assocrun_1()
{
    // There's an associated run:
    
    // Root record.
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    
    LogBookNote note(*m_db, ins.lastInsertId());
    
    LogBookRun* pRun;
    CPPUNIT_ASSERT_NO_THROW(
        pRun = note.getAssociatedRun(*m_db)
    );
    ASSERT(pRun);                  // Not null.
    
    EQ(123, pRun->getRunInfo().s_number);    // Right run.
    
    delete pRun;
}

void notetest::assocrun_2()
{
    // Root record.
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    // ins.bind(1, m_pRun->getRunInfo().s_id);   - Don't bind a run id.
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    
    LogBookNote note(*m_db, ins.lastInsertId());
    
    LogBookRun* pRun(nullptr);
    CPPUNIT_ASSERT_NO_THROW(
        pRun = note.getAssociatedRun(*m_db)
    );
    ASSERT(!pRun);
}

void notetest::notetext_1()
{
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    
    LogBookNote note(*m_db, ins.lastInsertId());
    
    auto& text = note.getNoteText();
    EQ(ins.lastInsertId(), text.s_id);
    EQ(m_pRun->getRunInfo().s_id, text.s_runId);
    EQ(now, text.s_noteTime);
    EQ(std::string("This is the note text"), text.s_contents);
}

void notetest::imagecount_1()
{
    // no images:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    
    LogBookNote note(*m_db, ins.lastInsertId());
    
    EQ(size_t(0), note.imageCount());
}
void notetest::imagecount_2()
{
    // there are images:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
 
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 10);
    img.bind(3, "/usr/opt/fox/testing/test.img", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    img.reset();
    img.bind(2, 100);
    img.bind(3, "~/images/barney.img", -1, SQLITE_STATIC);
    ++img;
    
    std::set<int> offsets = {10,100};
    std::set<std::string> names = {"/usr/opt/fox/testing/test.img", "~/images/barney.img"};
    // Construct the note:
    
    LogBookNote note(*m_db, id);
    EQ(size_t(2), note.imageCount());
}

void notetest::index_1()
{
    // valid index:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
 
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 10);
    img.bind(3, "/usr/opt/fox/testing/test.img", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    img.reset();
    img.bind(2, 100);
    img.bind(3, "~/images/barney.img", -1, SQLITE_STATIC);
    ++img;

    LogBookNote note(*m_db, id);
    
    CPPUNIT_ASSERT_NO_THROW(
        auto& junk(note[0])
    );
    auto& imageInfo(note[0]);          // Now that we know it won't blow up:
    
    EQ(id, imageInfo.s_noteId);
    EQ(10, imageInfo.s_noteOffset);
    EQ(std::string("/usr/opt/fox/testing/test.img"), imageInfo.s_originalFilename);
    EQ(sizeof(randomData), imageInfo.s_imageLength);
    EQ(0, memcmp(randomData, imageInfo.s_pImageData, imageInfo.s_imageLength));
    
}
void notetest::index_2()
{
    // valid index:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
 
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 10);
    img.bind(3, "/usr/opt/fox/testing/test.img", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    img.reset();
    img.bind(2, 100);
    img.bind(3, "~/images/barney.img", -1, SQLITE_STATIC);
    ++img;

    LogBookNote note(*m_db, id);
    
    CPPUNIT_ASSERT_THROW(
        note[2], std::out_of_range
    );
}

void notetest::image_1()
{
    // If there are no images substituteImages just gives the
    // original text:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
 
    LogBookNote note(*m_db, id);
    
    std::string text = note.substituteImages();
    EQ(note.getNoteText().s_contents, text);
}
void notetest::image_2()
{
    // The entire note is just an image link:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "![This is the text](/this/is/the/file.jpg)", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
    
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 0);
    img.bind(3, "/this/is/the/file.jpg", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    int imageid = img.lastInsertId();
    
    LogBookNote note(*m_db, id);
    std::string text;
    CPPUNIT_ASSERT_NO_THROW(text = note.substituteImages());
    
    // The text should be
    // ![This is the text](tempdir/note1_image1_file.jpg)
    
    std::stringstream s;
    s << "![This is the text](";
    s << LogBook::m_tempdir << "/note" << id <<"_image" << imageid
        << "_file.jpg)";
    std::string sb(s.str());
    EQ(sb, text);
    
}
void notetest::image_3()
{
    // Image link is at the end of the text with stuff in front.
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "Stuff ![This is the text](/this/is/the/file.jpg)", -1, SQLITE_STATIC);
    ++ins;
    int id = ins.lastInsertId();          // note id.
    
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 6);
    img.bind(3, "/this/is/the/file.jpg", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    int imageid = img.lastInsertId();
    
    LogBookNote note(*m_db, id);
    std::string text;
    CPPUNIT_ASSERT_NO_THROW(text = note.substituteImages());
    
    std::stringstream s;
    s << "Stuff ![This is the text](";
    s << LogBook::m_tempdir << "/note" << id <<"_image" << imageid
        << "_file.jpg)";
    std::string sb(s.str());
    
    EQ(sb, text);
}
void notetest::image_4()
{
    // Two images in the middle of the text.
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note)    \
            VALUES (?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "Stuff ![This is the text](/this/is/the/file.jpg) more ![More text](/this/file2.jpg) stuff", -1, SQLITE_STATIC);
    //           0123456789012345678901234567890123456789012345678901234567890
    //                     1         2         3         4         5         6   
    ++ins;
    int id = ins.lastInsertId();          // note id.
    
    uint8_t randomData[100];             // 'image' data.
    for (int i =0; i < 100; i++) randomData[i] = i;
    
    CSqliteStatement img (
        *m_db,
        "INSERT INTO note_image                               \
            (note_id, note_offset, original_filename, image)   \
            VALUES(?,?,?,?)"
    );
    img.bind(1, id);
    img.bind(2, 6);
    img.bind(3, "/this/is/the/file.jpg", -1, SQLITE_STATIC);
    img.bind(4, (void*)(randomData), sizeof(randomData), SQLITE_STATIC);
    ++img;    // one image.
    int imageid1 = img.lastInsertId();
    
    img.reset();
    img.bind(2, 54);
    img.bind(3, "/dome/image.gif", -1, SQLITE_STATIC);
    ++img;
    int imageid2 = img.lastInsertId();
    
    LogBookNote note(*m_db, id);
    std::stringstream s;
    s << "Stuff ![This is the text](";
    s << LogBook::m_tempdir << "/note" << id <<"_image" << imageid1
        << "_file.jpg)";
    s   << " more ![More text]("
        <<   LogBook::m_tempdir << "/note" << id <<"_image" << imageid2
        << "_image.gif) stuff";
    std::string sb(s.str());
    std::string is = note.substituteImages();
    EQ(sb, is);
}