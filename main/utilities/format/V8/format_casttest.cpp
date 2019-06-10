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
#include <V8/CScalerBuffer.h>
#include <V8/CControlBuffer.h>
#include <V8/CPhysicsEventBuffer.h>
#include <V8/CTextBuffer.h>

#include <Deserializer.h>

#include <DebugUtils.h>

#define private public
#define protected public
#include <V8/format_cast.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class format_casttest : public CppUnit::TestFixture {
private:
  bheader m_header;
  std::vector<uint16_t> m_bodyData;
  CPhysicsEventBuffer m_physicsBuffer;

public:
  CPPUNIT_TEST_SUITE(format_casttest);
  CPPUNIT_TEST( castRawToPhysics_0 );
  CPPUNIT_TEST( castRawToScaler_0 );
  CPPUNIT_TEST( castRawToControl_0 );
  CPPUNIT_TEST( castRawToText_0 );
  CPPUNIT_TEST( castTextToRaw_0 );
  CPPUNIT_TEST( castScalerToRaw_0 );
  CPPUNIT_TEST( castScalerToRaw_1 );
  CPPUNIT_TEST( castPhysicsEventToRaw_0 );
  CPPUNIT_TEST( castControlToRaw_0 );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::DATABF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 3;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    m_bodyData = std::vector<uint16_t>({3, 0, 1,
                                       2, 3,
                                       3, 4, 5});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  void tearDown()
  {
  }


void castRawToPhysics_0 () {

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;
  rawBuf.setBuffer(buffer);

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  auto  physBuf = format_cast<CPhysicsEventBuffer>(rawBuf);
  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 2nd event",
                         std::equal(physBuf.at(1)->begin(), physBuf.at(1)->end(),
                                    m_physicsBuffer.at(1)->begin()));
}

void castRawToScaler_0 () {
  std::vector<std::uint32_t> sclrs = {0, 1, 2, 3};
  m_header.nevt = sclrs.size();
  m_header.type = SCALERBF;

  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << uint32_t(1234);
  buffer << uint16_t(0);
  buffer << uint16_t(0);
  buffer << uint16_t(0);

  buffer << uint32_t(5678);
  buffer << uint16_t(0);
  buffer << uint16_t(0);
  buffer << uint16_t(0);

  buffer << sclrs;

  rawBuf.setBuffer(buffer);

  auto  sclrBuf = format_cast<CScalerBuffer>(rawBuf);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         uint32_t(1234), sclrBuf.getOffsetEnd());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         uint32_t(5678), sclrBuf.getOffsetBegin());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("RawBuffer ctor has proper 2nd event",
                         sclrs, sclrBuf.getScalers());
}

void castRawToControl_0 () {
  m_header.type = DAQ::V8::ENDRUNBF;

  CRawBuffer rawBuf(8192);
  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;

  string title("the best thing ever");
  title.resize(80, '\0');
  buffer << title.c_str();

  buffer << std::uint32_t(12345);
  buffer << bftime({12, 34, 56, 78, 90, 1, 2});
  rawBuf.setBuffer(buffer);

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CRawBuffer --> CControlBuffer is allowed",
        auto cntlBuf = format_cast<CControlBuffer>(rawBuf)
      );

}

void castRawToText_0 () {
  m_header.type = DAQ::V8::PKTDOCBF;

  CRawBuffer rawBuf(8192);

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << std::uint16_t(9);
  buffer << std::vector<std::string>({"the", "test"});
  rawBuf.setBuffer(buffer);

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CRawBuffer --> CTextBuffer is allowed",
        auto textBuf = format_cast<CTextBuffer>(rawBuf)
      );

}

void castTextToRaw_0() {
  CTextBuffer buffer;

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CTextBuffer --> CRawBuffer is allowed",
        auto rawBuf = format_cast<CRawBuffer>(buffer) );
}

void castScalerToRaw_0() {
  CScalerBuffer buffer;

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CScalerBuffer --> CRawBuffer is allowed",
        auto rawBuf = format_cast<CRawBuffer>(buffer) );
}

void castScalerToRaw_1() {
  bheader header = bheader(28, DAQ::V8::SCALERBF,
                           0, 0, 0,
                           1,
                           0, 0, 0,
                           DAQ::V8::StandardVsn,
                           DAQ::V8::BOM16, DAQ::V8::BOM32,
                           0, 0);
  CScalerBuffer buffer(header,
                       1234, 5678,
                       vector<uint32_t>({1}));


  auto rawBuf = format_cast<CRawBuffer>(buffer);

  DAQ::Buffer::Deserializer<ByteBuffer> stream(rawBuf.getBuffer(), false);

  std::uint16_t value;
  for (std::size_t i=0; i<16; ++i) {
    stream >> value;
  }

  std::uint32_t offEnd, offBegin, sclr0;
  stream >> offEnd;
  stream >> value;
  stream >> value;
  stream >> value;
  stream >> offBegin;
  stream >> value;
  stream >> value;
  stream >> value;
  stream >> sclr0;
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset begin is maintained",
                               std::uint32_t(1234), offBegin);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset end is maintained",
                               std::uint32_t(5678), offEnd);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler value is maintained",
                               std::uint32_t(1), sclr0);

  }

void castPhysicsEventToRaw_0() {
  CPhysicsEventBuffer buffer;

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CPhysicsEventBuffer --> CRawBuffer is allowed",
        auto rawBuf = format_cast<CRawBuffer>(buffer) );
}

void castControlToRaw_0() {
  CControlBuffer buffer;

  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "format_cast CControlBuffer --> CRawBuffer is allowed",
        auto rawBuf = format_cast<CRawBuffer>(buffer) );
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(format_casttest);

