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
#include <ByteOrder.h>
#include <V8/ChangeBufferSize.h>

#define private public
#define protected public
#include <V8/CTextBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class ctextbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<std::string> m_strings;
  CRawBuffer m_rawBuf;
  CTextBuffer m_buffer;

public:
  CPPUNIT_TEST_SUITE(ctextbuffertest);
  CPPUNIT_TEST(rawCtor_0);
  CPPUNIT_TEST(rawCtor_1);
  CPPUNIT_TEST(rawCtor_2);
  CPPUNIT_TEST(rawCtor_3);
  CPPUNIT_TEST(rawCtor_4);
  CPPUNIT_TEST(rawCtor_5);
//  CPPUNIT_TEST(rawCtor_6);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST(totalBytes_0);
  CPPUNIT_TEST(appendString_0);
  CPPUNIT_TEST(appendString_1);
  CPPUNIT_TEST(appendString_2);
  CPPUNIT_TEST_SUITE_END();

public:
  ctextbuffertest() : m_header(), m_strings(), m_rawBuf(8192), m_buffer() {}
  void setUp() {

    m_header.nwds = 24;
    m_header.type = DAQ::V8::STATEVARBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 4;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    // create some strings
    m_strings = {"a", "bb", "cccc", ""}; // needs a \0\0

    // compute the total number of bytes composed by strings
    std::uint16_t totalBytes = sizeof(std::uint16_t);
    for (auto& element : m_strings) {
      if ((element.size()%2)==0) {
        totalBytes += element.size() + 2;
      } else {
        totalBytes += element.size() + 1;
      }

    }

    ByteBuffer buffer;
    buffer << m_header;
    buffer << uint16_t(totalBytes/sizeof(std::uint16_t));
    buffer << m_strings.at(0).c_str();
    buffer << m_strings.at(1).c_str();
    buffer << m_strings.at(2).c_str();
    buffer << m_strings.at(3).c_str();

    m_rawBuf.setBuffer(buffer);

    m_buffer = CTextBuffer(m_rawBuf);
  }

  void tearDown()
  {
  }

  void rawCtor_0 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First string makes sense",
                                 m_strings.at(0), m_buffer.getStrings().at(0));
  }

  void rawCtor_1 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("second string makes sense",
                                 m_strings.at(1), m_buffer.getStrings().at(1));
  }

  void rawCtor_2 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("third string makes sense",
                                 m_strings.at(2), m_buffer.getStrings().at(2));

  }

  void rawCtor_3 () {

    CPPUNIT_ASSERT_EQUAL_MESSAGE("fourth string makes sense",
                                 m_strings.at(3), m_buffer.getStrings().at(3));

  }

  void rawCtor_4 () {
    CRawBuffer buffer;

    // create a buffer and fill it with a buffer header
    DAQ::Buffer::ByteBuffer buf;
    bheader headr; headr.type = DAQ::V8::DATABF; // this is an unacceptable type...
    buf << headr;

    // it does not matter that we finish this b/c it should throw before looking at any
    // of the data past the header

    buffer.setBuffer(buf);

    CPPUNIT_ASSERT_THROW_MESSAGE(
          "Raw ctor with non text type causes a thrown exception",
          CTextBuffer text(buffer),
          std::runtime_error
          );
  }

  void rawCtor_5 () {
    CRawBuffer buffer;

    // create a buffer and fill it with a buffer header
    DAQ::Buffer::ByteBuffer buf;
    bheader headr; // this is an unacceptable type...
    headr.nwds = 0x1800;
    headr.type = 0x0400;
    headr.cks  = 0;
    headr.run  = 0x0100;
    headr.seq  = 0x02000000;
    headr.nevt = 0x0400;
    headr.nlam = 0;
    headr.cpu  = 0;
    headr.nbit = 0;
    headr.buffmt = 0x0500;
    headr.ssignature = 0x0201;
    headr.lsignature = 0x04030201;
    headr.unused[0] = 0;
    headr.unused[1] = 0;

    buf << headr;

    // compute the total number of bytes composed by strings
    std::uint16_t totalBytes = sizeof(std::uint16_t);
    for (auto& element : m_strings) {
      if ((element.size()%2)==0) {
        totalBytes += element.size() + 2;
      } else {
        totalBytes += element.size() + 1;
      }
    }
    std::uint16_t swappedTotalBytes = totalBytes/sizeof(std::uint16_t);
    DAQ::BO::swapBytes(swappedTotalBytes);

    buf << swappedTotalBytes;
    buf << m_strings.at(0).c_str();
    buf << m_strings.at(1).c_str();
    buf << m_strings.at(2).c_str();
    buf << m_strings.at(3).c_str();

    // it does not matter that we finish this b/c it should throw before looking at any
    // of the data past the header

    buffer.setBuffer(buf);
    CTextBuffer text(buffer);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Originally inverted buffer is now uninverted",
                                 m_header, text.getHeader());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("total word size is ",
                                 uint16_t(totalBytes/sizeof(std::uint16_t)), text.totalShorts());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("strings are correct ",
                                 m_strings, text.getStrings());

  }

  void toRawBuffer_0 () {
    CRawBuffer rawBuf(8192);
    m_buffer.toRawBuffer(rawBuf);

    // Because we have create m_buffer using the m_rawBuf, we sure better find that
    // we get an identical buffer back using toRawBuffer

    CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer should fill the right stuff",
                                 m_rawBuf.getBuffer(),
                                 rawBuf.getBuffer());
  }

  void toRawBuffer_1 () {

    ::DAQ::V8::Test::ChangeBufferSize sizeForThisScope(1);

    CRawBuffer rawBuf;

    CPPUNIT_ASSERT_THROW_MESSAGE("BufferSize that is too small should fail",
                                 m_buffer.toRawBuffer(rawBuf),
                                 std::runtime_error);

  }

  void totalBytes_0() {
    CTextBuffer text(bheader(), {"a", "b", "c"});
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Computing total bytes in body is correct",
                                 std::uint16_t(4), text.totalShorts());
  }


void appendString_0 () {
  DAQ::V8::Test::ChangeBufferSize bSizeForScope(35);

  CTextBuffer test(DAQ::V8::RUNVARBF);
  bool result = test.appendString("There was no way!");

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Insufficient space in buffer for new string causes failure",
                               false, result);
}

void appendString_1 () {
    DAQ::V8::Test::ChangeBufferSize bSizeForScope(38);

    CTextBuffer test(DAQ::V8::RUNVARBF);
    bool result = test.appendString("A");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sufficient space in buffer for new string causes success",
                                 true, result);

}

void appendString_2 () {
    DAQ::V8::Test::ChangeBufferSize bSizeForScope(38);

    CTextBuffer test(DAQ::V8::RUNVARBF);
    std::string msg("A");
    test.appendString(msg);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sufficient space in buffer for new string causes success",
                                 msg, test.getStrings().at(0));

}


};

CPPUNIT_TEST_SUITE_REGISTRATION(ctextbuffertest);


