/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <V8/DataFormatV8.h>
#include <V8/CRawBuffer.h>
#include <DebugUtils.h>
#include <BufferPtr.h>
#include <ChangeBufferSize.h>

#define private public
#define protected public
#include <CControlBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class ccontrolbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  CControlBuffer m_buffer;
  CRawBuffer m_rawBuf;

public:
  CPPUNIT_TEST_SUITE(ccontrolbuffertest);
  CPPUNIT_TEST(title_0);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST(toRawBuffer_2);
  CPPUNIT_TEST(toRawBuffer_3);
  CPPUNIT_TEST(toRawBuffer_4);
  CPPUNIT_TEST(toRawBuffer_5);
  CPPUNIT_TEST(toRawBuffer_6);
  CPPUNIT_TEST(toRawBuffer_7);
  CPPUNIT_TEST(toRawBuffer_8);
  CPPUNIT_TEST(toRawBuffer_9);
  CPPUNIT_TEST_SUITE_END();

public:
  ccontrolbuffertest() : m_header(), m_buffer(bheader(), "", 0, bftime()),
    m_rawBuf(8192) {}

  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::BEGRUNBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 0;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    m_buffer = CControlBuffer(m_header, "my title", 1234, bftime({0, 1, 2 ,3, 4, 5, 6}));


    m_rawBuf = CRawBuffer();
    m_buffer.toRawBuffer(m_rawBuf);

  }

  void tearDown()
  {
  }


  void title_0() {
    std::string title(81, ' ');

    CPPUNIT_ASSERT_THROW_MESSAGE("Titles longer than 80 characters results in an exception",
                                 CControlBuffer buffer(m_header, title, 0, bftime() ),
                                 std::runtime_error );
  }

  void toRawBuffer_0 () {

    std::string expected("my title");
    expected.resize(80, ' '); // right pad with null character

    // locate the spot in the buffer where the title lives
    auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(uint16_t);
    auto end = beg + 80;

    // compare the entire 80 characters!
    CPPUNIT_ASSERT_MESSAGE("toRawBuffer preserves title",
                           std::equal(beg, end, expected.begin()));
}

void toRawBuffer_1 () {

  auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t) + 80;
  BufferPtr<std::uint32_t> p32(beg, false);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves offset",
                           std::uint32_t(1234), *p32);
}

  void toRawBuffer_2 () {

    auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
                + 80 // title
                + sizeof(std::uint32_t); // offset

    DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves month",
                           std::uint16_t(0), *p16);
  }
    void toRawBuffer_3 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + sizeof(std::uint16_t); // month

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves day",
                             std::uint16_t(1), *p16);
    }
    void toRawBuffer_4 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + 2*sizeof(std::uint16_t); // month, day

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves year",
                             std::uint16_t(2), *p16);
    }
    void toRawBuffer_5 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + 3*sizeof(std::uint16_t); // month, day, year

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves hour",
                             std::uint16_t(3), *p16);
    }

    void toRawBuffer_6 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + 4*sizeof(std::uint16_t); // month, day, year, hours

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves minute",
                             std::uint16_t(4), *p16);
    }

    void toRawBuffer_7 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + 5*sizeof(std::uint16_t); // month, day, year, hours, min

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves seconds",
                             std::uint16_t(5), *p16);
    }

    void toRawBuffer_8 () {

      auto beg = m_rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t)
          + 80 // title
          + sizeof(std::uint32_t)  // offset
          + 6*sizeof(std::uint16_t); // month, day, year, hours, min, tenths

      DAQ::Buffer::BufferPtr<std::uint16_t> p16(beg, false);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer preserves tenths",
                             std::uint16_t(6), *p16);
    }

    void toRawBuffer_9 () {
      DAQ::V8::Test::ChangeBufferSize bufferSizeForScope(1);

      CPPUNIT_ASSERT_THROW_MESSAGE("Fail if gBufferSize is too small",
             m_buffer.toRawBuffer(m_rawBuf),
            std::runtime_error);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ccontrolbuffertest);

class ccontrolbuffertest_rawCtor : public CppUnit::TestFixture {
private:
  bheader m_header;
  CControlBuffer m_buffer;
  CRawBuffer m_rawBuf;
  std::string m_title;
  std::uint32_t m_offset;
  bftime m_time;

public:
  CPPUNIT_TEST_SUITE(ccontrolbuffertest_rawCtor);
  CPPUNIT_TEST(rawCtor_0);
  CPPUNIT_TEST(rawCtor_1);
  CPPUNIT_TEST(rawCtor_2);
  CPPUNIT_TEST(rawCtor_3);
  CPPUNIT_TEST(rawCtor_4);
  CPPUNIT_TEST(rawCtor_5);
  CPPUNIT_TEST(rawCtor_6);
  CPPUNIT_TEST(rawCtor_7);
  CPPUNIT_TEST(rawCtor_8);
  CPPUNIT_TEST(rawCtor_9);
  CPPUNIT_TEST_SUITE_END();

public:
  ccontrolbuffertest_rawCtor() : m_header(), m_buffer(bheader(), "", 0, bftime()),
    m_rawBuf(8192), m_title() {}

  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::BEGRUNBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 0;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    m_title = "my test title";
    m_title.resize(80, ' ');
    char title[80];
    std::copy(m_title.begin(), m_title.end(), title);

    m_offset = 12345;
    m_time = {9, 8, 7, 6, 5, 4, 3};

    m_rawBuf = CRawBuffer(8192);

    ByteBuffer buffer;
    buffer << m_header;
    buffer << title;
    buffer << m_offset;
    buffer << m_time;
    m_rawBuf.setBuffer(buffer);

    m_buffer = CControlBuffer(m_rawBuf);

  }

  void tearDown()
  {
  }

  void rawCtor_0 () {

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Title is correct",
                                 m_title, m_buffer.getTitle());
  }

  void rawCtor_1 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset is correct",
                                 m_offset, m_buffer.getOffset());
  }

  void rawCtor_2 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Month is correct",
                                 m_time.month, m_buffer.getTimeStruct().month);
  }
  void rawCtor_3 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Day is correct",
                                 m_time.day, m_buffer.getTimeStruct().day);
  }
  void rawCtor_4 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Year is correct",
                                 m_time.year, m_buffer.getTimeStruct().year);
  }
  void rawCtor_5 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hour is correct",
                                 m_time.hours, m_buffer.getTimeStruct().hours);
  }
  void rawCtor_6 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Minute is correct",
                                 m_time.min, m_buffer.getTimeStruct().min);
  }
  void rawCtor_7 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second is correct",
                                 m_time.sec, m_buffer.getTimeStruct().sec);
  }
  void rawCtor_8 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tenth is correct",
                                 m_time.tenths, m_buffer.getTimeStruct().tenths);
  }

  void rawCtor_9 () {
    m_header.nwds = 0x6400;
    m_header.type = 0x0b00;
    m_header.cks  = 0x0000;
    m_header.run  = 0x0100;
    m_header.seq  = 0x02000000;
    m_header.nevt = 0;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = 0x0500;
    m_header.ssignature = 0x0201;
    m_header.lsignature = 0x04030201;

    m_title = std::string("a test title");
    m_title.resize(80, ' ');
    char title[80];
    std::copy(m_title.begin(), m_title.end(), title);

    m_offset = 0x39300000; // 12345 in reverse byte order
    m_time = {0x0900, 0x0800, 0x0700, 0x0600, 0x0500, 0x0400, 0x0300};
    DAQ::V8::bftime time = {9, 8, 7, 6, 5, 4, 3};

    m_rawBuf = CRawBuffer(8192);

    ByteBuffer buffer;
    buffer << m_header;
    buffer << title;
    buffer << m_offset;
    buffer << m_time;
    m_rawBuf.setBuffer(buffer);

    m_buffer = CControlBuffer(m_rawBuf);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Construct from swapped body must yield correct offset  ",
                                 uint32_t(12345), m_buffer.getOffset());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Construct from swapped body must yield correct title  ",
                                 m_title, m_buffer.getTitle());
    CPPUNIT_ASSERT_MESSAGE("Construct from swapped body must yield correct time  ",
                           time == m_buffer.getTimeStruct());

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(ccontrolbuffertest_rawCtor);
