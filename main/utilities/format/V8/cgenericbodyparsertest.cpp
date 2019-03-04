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
#include <make_unique.h>

#define private public
#define protected public
#include <V8/CGenericBodyParser.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Test the parsing of data with a 32-bit wide, inclusive byte count
//
// This also serves as the base set of tests for the other scenarios. The
// major variation will be the setUp() method.
class genericbodyparsertest_InclByte32 : public CppUnit::TestFixture {
protected:
  DAQ::Buffer::ByteBuffer m_bodyData;
  CGenericBodyParser      m_parser;
  BufferPtr<uint16_t>     m_beg;
  BufferPtr<uint16_t>     m_deadend;

public:
  CPPUNIT_TEST_SUITE(genericbodyparsertest_InclByte32);
  CPPUNIT_TEST(parseOne_0);
  CPPUNIT_TEST(parseOne_1);
  CPPUNIT_TEST(parseOne_2);
  CPPUNIT_TEST(parseOne_3);
  CPPUNIT_TEST(parseOne_4);
  CPPUNIT_TEST(parse_0);
  CPPUNIT_TEST(parse_1);
  CPPUNIT_TEST(parse_2);
  CPPUNIT_TEST(parse_3);
  CPPUNIT_TEST_SUITE_END();

public:

  genericbodyparsertest_InclByte32()
    : m_parser(Inclusive32BitBytes) {}

  virtual void setUp() {

    std::vector<uint16_t> data({10, 0, 1, 2, 3, 4, 0});

    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << data;

    m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), false);
    m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), false);

    m_parser = CGenericBodyParser(Inclusive32BitBytes);

}

  virtual void tearDown()
  {
  }

  void parseOne_0() {

    auto result = m_parser.parseOne(m_beg, m_deadend);

    CPPUNIT_ASSERT_MESSAGE("Iterator after parsing 1 is correct",
                           (m_beg+5) == std::get<1>(result));
  }

  void parseOne_1() {
    auto result = m_parser.parseOne(m_beg, m_deadend);

    CPPUNIT_ASSERT_MESSAGE("Parse one produces proper body",
                           std::equal(m_beg, m_beg+5, std::get<0>(result)->begin()));
  }

  void parseOne_2() {

    // create a bad set of data (size = 20 but there are not 20 bytes words)
    vector<uint16_t> data({20, 0, 1, 2, 3});

    // load the buffer with the bad data
    ByteBuffer buffer;
    buffer << data;

    m_beg     = BufferPtr<uint16_t>(buffer.begin(), false);
    m_deadend = BufferPtr<uint16_t>(buffer.end(), false);

    CPPUNIT_ASSERT_THROW_MESSAGE("Inconsistent size of buffer causes throw",
                                 m_parser.parseOne(m_beg,  m_deadend),
                                 std::runtime_error);
  }

  void parseOne_3() {
    // create a bad set of data (size = 10 but there are not 10 words)
    vector<uint16_t> data({0, 0, 1, 2, 3});

    // load the buffer with the bad data
    ByteBuffer buffer;
    buffer << data;

    m_beg     = BufferPtr<uint16_t>(buffer.begin(), false);
    m_deadend = BufferPtr<uint16_t>(buffer.end(), false);

    CPPUNIT_ASSERT_THROW_MESSAGE("Zero buffer size causes throw",
                               m_parser.parseOne(m_beg, m_deadend),
                               std::runtime_error);
  }

    void parseOne_4() {
      m_deadend = m_beg-1;
      CPPUNIT_ASSERT_THROW_MESSAGE("Begin iter is beyond deadend iter throws",
                                 m_parser.parseOne(m_beg, m_deadend),
                                 std::runtime_error);
    }

    void parse_0() {

      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "parsing 2 physics event should produce 2 physics events",
            size_t(2), result.size());
    }

    void parse_1() {
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "first physics event makes sense",
            std::equal(m_beg, m_beg+5, result.at(0)->begin()));
    }

    void parse_2() {
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense",
            std::equal(m_beg+5, m_deadend, result.at(1)->begin()));
    }

    void parse_3() {

      // test that we can handle parsing byte-reversed data
      std::vector<uint16_t> expected({10, 0, 1, 2, 3, 4, 0});

      std::vector<uint16_t> data({0x0000, 0x0a00, 0x0100, 0x0200, 0x0300,
                                 0x0000, 0x0400});

      m_bodyData = DAQ::Buffer::ByteBuffer();
      m_bodyData << data;

      m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), true);
      m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), true);

      m_parser = CGenericBodyParser(Inclusive32BitBytes);
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense even when byte swapped",
            std::equal(m_beg+5, m_deadend, result.at(1)->begin()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(genericbodyparsertest_InclByte32);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Test the parsing of data with a 32-bit wide, inclusive 16-bit word count

class genericbodyparsertest_InclWord32 : public genericbodyparsertest_InclByte32 {

public:
  CPPUNIT_TEST_SUITE(genericbodyparsertest_InclWord32);
  CPPUNIT_TEST(parseOne_0);
  CPPUNIT_TEST(parseOne_1);
  CPPUNIT_TEST(parseOne_2);
  CPPUNIT_TEST(parseOne_3);
  CPPUNIT_TEST(parseOne_4);
  CPPUNIT_TEST(parse_0);
  CPPUNIT_TEST(parse_1);
  CPPUNIT_TEST(parse_2);
  CPPUNIT_TEST(parse_4);
  CPPUNIT_TEST_SUITE_END();

public:

  genericbodyparsertest_InclWord32() {}

  void setUp() {

    std::vector<uint16_t> data({5, 0, 1, 2, 3, 2, 0});

    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << data;

    m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), false);
    m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), false);

    m_parser = CGenericBodyParser(Inclusive32BitWords);

}

  void tearDown()
  {
  }

  void parse_4() {

      // test that we can handle parsing byte-reversed data
      std::vector<uint16_t> expected({5, 0, 1, 2, 3, 2, 0});

      std::vector<uint16_t> data({0x0000, 0x0500, 0x0100, 0x0200, 0x0300,
                                 0x0000, 0x0200});

      m_bodyData = DAQ::Buffer::ByteBuffer();
      m_bodyData << data;

      m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), true);
      m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), true);

      m_parser = CGenericBodyParser(Inclusive32BitWords);
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense even when byte swapped",
            std::equal(m_beg+5, m_deadend, result.at(1)->begin()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(genericbodyparsertest_InclWord32);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Test the parsing of data with a 16-bit wide, exclusive word count


class genericbodyparsertest_ExclWord16 : public genericbodyparsertest_InclByte32 {

public:
  CPPUNIT_TEST_SUITE(genericbodyparsertest_ExclWord16);
  CPPUNIT_TEST(parseOne_0);
  CPPUNIT_TEST(parseOne_1);
  CPPUNIT_TEST(parseOne_2);
  CPPUNIT_TEST(parseOne_4);
  CPPUNIT_TEST(parse_0);
  CPPUNIT_TEST(parse_1);
  CPPUNIT_TEST(parse_2);
  CPPUNIT_TEST(parse_5);
  CPPUNIT_TEST_SUITE_END();

public:

  genericbodyparsertest_ExclWord16() {}

  void setUp() {

    std::vector<uint16_t> data({4, 0, 1, 2, 3, 1, 0});

    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << data;

    m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), false);
    m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), false);

    m_parser = CGenericBodyParser(Exclusive16BitWords);

}

  void tearDown()
  {
  }

  void parse_5() {

      // test that we can handle parsing byte-reversed data
      std::vector<uint16_t> expected({4, 0, 1, 2, 3, 1, 0});

      std::vector<uint16_t> data({0x0400, 0x0000, 0x0100, 0x0200, 0x0300,
                                 0x0100, 0x0000});

      m_bodyData = DAQ::Buffer::ByteBuffer();
      m_bodyData << data;

      m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), true);
      m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), true);

      m_parser = CGenericBodyParser(Exclusive16BitWords);
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense even when byte swapped",
            std::equal(m_beg+5, m_deadend, result.at(1)->begin()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(genericbodyparsertest_ExclWord16);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Test the parsing of data with a 16-bit wide, inclusive 16-bit word count

class genericbodyparsertest_InclWord16 : public genericbodyparsertest_InclByte32 {

public:
  CPPUNIT_TEST_SUITE(genericbodyparsertest_InclWord16);
  CPPUNIT_TEST(parseOne_0);
  CPPUNIT_TEST(parseOne_1);
  CPPUNIT_TEST(parseOne_2);
  CPPUNIT_TEST(parseOne_3);
  CPPUNIT_TEST(parseOne_4);
  CPPUNIT_TEST(parse_0);
  CPPUNIT_TEST(parse_1);
  CPPUNIT_TEST(parse_2);
  CPPUNIT_TEST(parse_6);
  CPPUNIT_TEST_SUITE_END();

public:

  genericbodyparsertest_InclWord16() {}

  void setUp() {

    std::vector<uint16_t> data({5, 0, 1, 2, 3, 2, 0});

    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << data;

    m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), false);
    m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), false);

    m_parser = CGenericBodyParser(Inclusive16BitWords);

}

  void tearDown()
  {
  }

  void parse_6() {

      // test that we can handle parsing byte-reversed data
      std::vector<uint16_t> expected({4, 0, 1, 2, 3, 1, 0});

      std::vector<uint16_t> data({0x0500, 0x0000, 0x0100, 0x0200, 0x0300,
                                 0x0200, 0x0000});

      m_bodyData = DAQ::Buffer::ByteBuffer();
      m_bodyData << data;

      m_beg     = BufferPtr<uint16_t>(m_bodyData.begin(), true);
      m_deadend = BufferPtr<uint16_t>(m_bodyData.end(), true);

      m_parser = CGenericBodyParser(Inclusive16BitWords);
      auto result = m_parser(2, m_beg, m_deadend);

      CPPUNIT_ASSERT_MESSAGE(
            "second physics event makes sense even when byte swapped",
            std::equal(m_beg+5, m_deadend, result.at(1)->begin()));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(genericbodyparsertest_InclWord16);




