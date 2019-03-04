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
#include <ByteBuffer.h>
#include <V8/ChangeBufferSize.h>

#define private public
#define protected public
#include <V8/CScalerBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class scalerbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<std::uint32_t> m_scalers;
  CScalerBuffer m_scalerBuffer;
  ByteBuffer m_bytes;
  std::uint32_t m_offsetEnd;
  std::uint32_t m_offsetBegin;


public:
  CPPUNIT_TEST_SUITE(scalerbuffertest);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_scalers = std::vector<std::uint32_t>({3, 0, 1,
                                           2, 3,
                                           3, 4, 5});
    m_header.nwds = 42;
    m_header.type = DAQ::V8::SCALERBF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = m_scalers.size();
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;


    // create the various pieces in the body
    m_offsetBegin = 123;
    m_offsetEnd   = 456;
    std::vector<std::uint8_t>  empty(6); // empty space

    // fill a byte buffer with data
    m_bytes << m_offsetEnd;
    m_bytes << empty;
    m_bytes << m_offsetBegin;
    m_bytes << empty;
    m_bytes << m_scalers;

    m_scalerBuffer = CScalerBuffer(m_header, 123, 456, m_scalers);

  }

  void tearDown()
  {
  }

void rawBufferCtor_0() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Scalers are correct when constructed from raw buffer",
                               m_scalers, sclrBuf.getScalers());
}


void rawBufferCtor_1() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset end is correct when constructed from raw buffer",
                               m_offsetEnd, sclrBuf.getOffsetEnd());
}

void rawBufferCtor_2() {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset begin is correct when constructed from raw buffer",
                               m_offsetBegin, sclrBuf.getOffsetBegin());
}

void rawBufferCtor_3() {
  // poison the type to be something different than a scaler type
  m_header.type = BEGRUNBF;

  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bytes;
  rawBuf.setBuffer(buffer);

  CPPUNIT_ASSERT_THROW_MESSAGE( "Raw ctor from non-scaler type fails",
                                CScalerBuffer sclrBuf(rawBuf),
                                std::runtime_error);
}

// construct from a byte swapped buffer produces the correct result
void rawBufferCtor_4() {
  bheader header;
  header.nwds = 0x2a00;
  header.type = 0x0200;
  header.nevt = 0x0800;
  header.buffmt = 0x0500;
  header.ssignature = 0x0201;
  header.lsignature = 0x04030201;
  header.cks = 0;
  header.cpu = 0;
  header.nbit = 0;
  header.nlam = 0;
  header.run = 0x0100;
  header.seq = 0x02000000;
  header.unused[0] = 0;
  header.unused[1] = 0;

  std::vector<std::uint32_t> sclrs({0x03000000, 0x00000000, 0x01000000,
                                    0x02000000, 0x03000000,
                                    0x03000000, 0x04000000, 0x05000000});

  // create the various pieces in the body
  std::uint32_t offsetBegin = 0x7b000000; // reversal of 123=0x007b
  std::uint32_t offsetEnd   = 0xc8010000; // reversal of 456=0x01c8
  std::vector<std::uint8_t>  empty(6); // empty space

  // fill a byte buffer with data
  ByteBuffer bytes;
  bytes << offsetEnd;
  bytes << empty;
  bytes << offsetBegin;
  bytes << empty;
  bytes << sclrs;

  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << header;
  buffer << bytes;
  rawBuf.setBuffer(buffer);

  CScalerBuffer sclrBuf(rawBuf);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps header properly",
                         m_header,  sclrBuf.getHeader());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps offset end properly",
                         m_offsetEnd, sclrBuf.getOffsetEnd());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps offset begin properly",
                         m_offsetBegin, sclrBuf.getOffsetBegin());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps scalers properly",
                         m_scalers, sclrBuf.getScalers());

  }

void toRawBuffer_0 () {
  CRawBuffer rawBuf(8192);

  m_scalerBuffer.toRawBuffer(rawBuf);

  ByteBuffer expected;
  expected << m_header;
  expected << m_bytes;
  expected.resize(8192); // need to ensure that the buffer is a full
                         // 8192 for comparison with the raw buffer. The
                         // rawbuffer will have the proper gBufferSize
                         // according to CScalerBuffer::toRawBuffer()

  CPPUNIT_ASSERT_EQUAL_MESSAGE("toRawBuffer does what it should",
                               expected, rawBuf.getBuffer());
}

void toRawBuffer_1 () {


  // in case this fails, we want to change back the buffer size to what it was.
  ::DAQ::V8::Test::ChangeBufferSize newSize(1);

  CRawBuffer rawBuf;
  CPPUNIT_ASSERT_THROW_MESSAGE("toRawBuffer should throw if gBufferSize is too small",
          m_scalerBuffer.toRawBuffer(rawBuf),
          std::runtime_error);
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(scalerbuffertest);



