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
    LogBookPerson* m_pRon;
    LogBookPerson* m_pGiordano;

public:
    void setUp() {
        m_filename = tempFilename("logfile.XXXXXX");
        LogBook::create(m_filename.c_str(), "Fox", "Ron", "Testing");

        m_pLogbook = new LogBook(m_filename.c_str());
        m_db       = m_pLogbook->m_pConnection;

        // Make some people in the logbook:

        m_pRon      = m_pLogbook->addPerson("Fox", "Ron", "Mr.");
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
    
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);       // No associated run.
    CPPUNIT_TEST(create_3);       // With images.
    CPPUNIT_TEST(create_4);      // No such image file.
    
    CPPUNIT_TEST(listrun_1);
    CPPUNIT_TEST(listrun_2);
    CPPUNIT_TEST(listrun_3);
    CPPUNIT_TEST(listrun_4);
    
    CPPUNIT_TEST(listnonrun_1);
    CPPUNIT_TEST(listnonrun_2);
    CPPUNIT_TEST(listnonrun_3);
    
    CPPUNIT_TEST(getall_1);
    CPPUNIT_TEST(getall_2);
    
    CPPUNIT_TEST(getrun_1);     // list run thoroughly tested
    CPPUNIT_TEST(getrun_2);     // much of this so cursory testing.
    
    CPPUNIT_TEST(getnonrun_1);
    CPPUNIT_TEST(getnonrun_2);
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
    
    void create_1();
    void create_2();
    void create_3();
    void create_4();
    
    void listrun_1();
    void listrun_2();
    void listrun_3();
    void listrun_4();
    
    void listnonrun_1();
    void listnonrun_2();
    void listnonrun_3();
    
    void getall_1();
    void getall_2();
    
    void getrun_1();
    void getrun_2();
    
    void getnonrun_1();
    void getnonrun_2();
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
        "INSERT INTO note (run_id, author_id, note_time, note)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    ins.bind(2, m_pRon->id());
    time_t now = time(nullptr);
    ins.bind(3, int(now));
    ins.bind(4, "This is the note text", -1, SQLITE_STATIC);
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
    EQ(m_pRon->id(), pNote->m_textInfo.s_authorId);
    
    
    delete pNote;
}
void notetest::construct_3()
{
    // Construct a note with images attached>
    
    // Root record.
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, author_id, note_time, note)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    ins.bind(2, m_pGiordano->id());
    time_t now = time(nullptr);
    ins.bind(3, int(now));
    ins.bind(4, "This is the note text", -1, SQLITE_STATIC);
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
    EQ(m_pGiordano->id(), note.m_textInfo.s_authorId);
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    // ins.bind(1, m_pRun->getRunInfo().s_id);   - Don't bind a run id.
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pGiordano->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
    ++ins;
    
    LogBookNote note(*m_db, ins.lastInsertId());
    
    EQ(size_t(0), note.imageCount());
}
void notetest::imagecount_2()
{
    // there are images:
    
    CSqliteStatement ins(
        *m_db,
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "This is the note text", -1, SQLITE_STATIC);
    ins.bind(4, m_pRon->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "![This is the text](/this/is/the/file.jpg)", -1, SQLITE_STATIC);
    ins.bind(4, m_pGiordano->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "Stuff ![This is the text](/this/is/the/file.jpg)", -1, SQLITE_STATIC);
    ins.bind(4, m_pGiordano->id());
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
        "INSERT INTO note (run_id, note_time, note, author_id)    \
            VALUES (?,?,?,?)"
    );
    
    ins.bind(1, m_pRun->getRunInfo().s_id);
    time_t now = time(nullptr);
    ins.bind(2, int(now));
    ins.bind(3, "Stuff ![This is the text](/this/is/the/file.jpg) more ![More text](/this/file2.jpg) stuff", -1, SQLITE_STATIC);
    //           0123456789012345678901234567890123456789012345678901234567890
    //                     1         2         3         4         5         6
    ins.bind(4, m_pGiordano->id());
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
void notetest::create_1()
{
    std::vector<LogBookNote::ImageInfo> images;    // No images for this run:
    
    LogBookNote* pNote(nullptr);
    CPPUNIT_ASSERT_NO_THROW(pNote = LogBookNote::create(
        *m_db, m_pRun, "This is my note", m_pRon, images
    ));
    ASSERT(pNote);
    
    auto& text = pNote->getNoteText();
    EQ(m_pRun->getRunInfo().s_id, text.s_runId);
    EQ(m_pRon->id(), text.s_authorId);
    EQ(std::string("This is my note"), text.s_contents);
    EQ(size_t(0), pNote->imageCount());
    
    LogBookRun* pRun = pNote->getAssociatedRun(*m_db);
    ASSERT(pRun);
    
    delete pRun;
    delete pNote;
}
void notetest::create_2()
{
    // create without images or associated run:
    
    std::vector<LogBookNote::ImageInfo> images;    // No images for this run:
    
    LogBookNote* pNote = LogBookNote::create(
        *m_db, static_cast<LogBookRun*>(nullptr),
        "This is my note", m_pGiordano, images
    );
    ASSERT(pNote);
    ASSERT(!pNote->getAssociatedRun(*m_db));
    
    delete pNote;
}
void notetest::create_3()
{
    // For the image file, since there's no checking that this is
    // a valid image, we'll just use the database because we know
    // it exists.
    
    std::vector<LogBookNote::ImageInfo> images;    // No images for this run:
    images.push_back({m_filename, 5});
    
    LogBookNote* pNote(nullptr);
    CPPUNIT_ASSERT_NO_THROW( pNote = LogBookNote::create(
        *m_db, m_pRun, "![this is a link](imagefile.jpg)", m_pGiordano, images
    ));
    
    EQ(size_t(1), pNote->imageCount());
    EQ(m_filename, (*pNote)[0].s_originalFilename);
    EQ(5, (*pNote)[0].s_noteOffset);
    
    delete pNote;
}
void notetest::create_4()
{
    // no such image file throws.
    
    std::vector<LogBookNote::ImageInfo> images =
    { {"/this/file/does/not/exist", 0}};   // Well I hope to hell it doesn't
    
    CPPUNIT_ASSERT_THROW(
        LogBookNote::create(
            *m_db, m_pRun, "![link text](/this/file/does/not/exist)",
            m_pRon,
            images
        ),
        LogBook::Exception
    );
}

void notetest::listrun_1()
{
    // No notes gives an empty list.
    
    std::vector<int> listing;
    CPPUNIT_ASSERT_NO_THROW(
        listing = LogBookNote::listRunNoteIds(*m_db, m_pRun->getRunInfo().s_id)
    );
    EQ(size_t(0), listing.size());
}
void notetest::listrun_2()
{
    // Make a few notes with  runs.
    
    
    std::vector<LogBookNote::ImageInfo> images;
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db,  m_pRun,
            "This is a note.", m_pGiordano, images
        );
    }
    
    auto listing = LogBookNote::listRunNoteIds(
        *m_db, m_pRun->getRunInfo().s_id
    );
    EQ(size_t(10), listing.size());
    for (int i =0; i < 10; i++) {
        EQ((i+1), listing[i]);
    }
}
void notetest::listrun_3()
{
    // Some notes match.
    // We'll make some notes without runs
    std::vector<LogBookNote::ImageInfo> images;
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db,
            (((i % 2) == 0) ? m_pRun : (static_cast<LogBookRun*>(nullptr))),
            "This is a note.", m_pRon, images
        );
    }
    
    auto listing = LogBookNote::listRunNoteIds(
        *m_db, m_pRun->getRunInfo().s_id
    );
    EQ(size_t(5), listing.size());
    for (int i =0; i < 5; i++) {
        EQ(i*2+1, listing[i]);
    }
}
void notetest::listrun_4()
{
    // Notes but not for the run requested.
    std::vector<LogBookNote::ImageInfo> images;
    m_pLogbook->end(m_pRun);             // End the run so we can make another
    
    auto run2 =  m_pLogbook->begin(7, "This is a run");
    
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db, ((i % 2) == 0) ? m_pRun : run2,
            "This is a note", m_pGiordano, images
        );
    }
    
    auto listing = LogBookNote::listRunNoteIds(*m_db, run2->getRunInfo().s_id);
    EQ(size_t(5), listing.size());
    for (int i = 0; i < 5; i++) {
        EQ(i*2+2, listing[i]);
    }
}

void notetest::listnonrun_1()
{
    // No notes so no list:
    
    std::vector<int> listing;
    CPPUNIT_ASSERT_NO_THROW(
        LogBookNote::listNonRunNotes(*m_db);
    );
    EQ(size_t(0), listing.size());
}
void notetest::listnonrun_2()
{
    // Yeah there are notes, but they all are associated with a run:
    
    std::vector<LogBookNote::ImageInfo> images;
    std::vector<int> listing;
    for (int i = 0; i < 10; i++) {
        delete LogBookNote::create(*m_db, m_pRun, "A note", m_pRon, images);
    }
    listing = LogBookNote::listNonRunNotes(*m_db);
    EQ(size_t(0), listing.size());
}
void notetest::listnonrun_3()
{
    // Half the notes are not associated with a run (the odd half).
    
    std::vector<LogBookNote::ImageInfo> images;
    std::vector<int> listing;
    for (int i = 0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db,
            (((i % 2) == 0) ? m_pRun : static_cast<LogBookRun*>(nullptr)),
            "A note", m_pGiordano, images
        );
    }
    listing = LogBookNote::listNonRunNotes(*m_db);
    EQ(size_t(5), listing.size());
    for (int i =0; i < 5; i++) {
        EQ(i*2+2, listing[i]);
    }
}   

void notetest::getall_1()
{
    // there are no notes to get.
    
    EQ(size_t(0), LogBookNote::getAllNotes(*m_db).size());
}
void notetest::getall_2()
{
    // while there are a mix of notes associated and not associated
    // with runs all are returned.
    
    std::vector<LogBookNote::ImageInfo> images;

    for (int i = 0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db,
            (((i % 2) == 0) ? m_pRun : static_cast<LogBookRun*>(nullptr)),
            "A note", m_pRon, images
        );
    }
    auto notes = LogBookNote::getAllNotes(*m_db);
    EQ(size_t(10), notes.size());
    
    for (int i = 0;  i < 10; i++) {
        EQ(i+1, notes[i]->getNoteText().s_id);
        delete notes[i];        // no longer needed.
    }
}

void notetest::getrun_1()
{
    // No notes associated with runs. no matches.
    std::vector<LogBookNote::ImageInfo> images;
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db, static_cast<LogBookRun*>(nullptr),
            "A note", m_pGiordano, images
        );
    }
    EQ(size_t(0), LogBookNote::getRunNotes(*m_db, m_pRun->getRunInfo().s_id).size());
}
void notetest::getrun_2()
{
    // Matches to our runs in mixed env.
    
    // Notes but not for the run requested.
    std::vector<LogBookNote::ImageInfo> images;
    m_pLogbook->end(m_pRun);             // End the run so we can make another
    
    auto run2 =  m_pLogbook->begin(7, "This is a run");
    
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db, ((i % 2) == 0) ? m_pRun : run2,
            "This is a note", m_pRon, images
        );
    }
    
    auto notes = LogBookNote::getRunNotes(*m_db, run2->getRunInfo().s_id);
    EQ(size_t(5), notes.size());
    for (int i = 0; i < 5; i++) {
        EQ(i*2+2, notes[i]->getNoteText().s_id);
        delete notes[i];
    }
     
    delete run2;
}

void notetest::getnonrun_1()
{
    // no non run notes.
    
    // Notes but not for the run requested.
    std::vector<LogBookNote::ImageInfo> images;
    m_pLogbook->end(m_pRun);             // End the run so we can make another
    
    auto run2 =  m_pLogbook->begin(7, "This is a run");
    
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db, ((i % 2) == 0) ? m_pRun : run2,
            "This is a note", m_pGiordano, images
        );
    }
    
    EQ(size_t(0), LogBookNote::getNonRunNotes(*m_db).size());
}
void notetest::getnonrun_2()
{
    // 1/2 of the notes are non run.
    
    std::vector<LogBookNote::ImageInfo> images;
    for (int i =0; i < 10; i++) {
        delete LogBookNote::create(
            *m_db, ((i % 2) == 0) ? m_pRun : static_cast<LogBookRun*>(nullptr),
            "This is a note", m_pRon, images
        );
    }
    auto runs = LogBookNote::getNonRunNotes(*m_db);
    EQ(size_t(5), runs.size());
    for (int i =0; i < 5; i++) {
        EQ(i*2+2, runs[i]->getNoteText().s_id);
        delete runs[i];
    }
}