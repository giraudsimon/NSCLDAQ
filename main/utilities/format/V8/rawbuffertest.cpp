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

#define private public
#define protected public
#include <V8/CScalerBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;





class rawbuffertest : public CppUnit::TestFixture {
private:
  bheader m_header;
  ByteBuffer m_bytes;
  CRawBuffer m_buffer;

public:
  CPPUNIT_TEST_SUITE(rawbuffertest);
  CPPUNIT_TEST(setBuffer_0);
  CPPUNIT_TEST(setBuffer_1);
  CPPUNIT_TEST(setBuffer_2);
  CPPUNIT_TEST(setBuffer_3);
  CPPUNIT_TEST(setBuffer_4);
  CPPUNIT_TEST(setBuffer_5);
  CPPUNIT_TEST(setBuffer_6);
  CPPUNIT_TEST_SUITE_END();

public:
  rawbuffertest() : m_header(), m_bytes(), m_buffer(gBufferSize) {}

  void setUp() {

    m_header.nwds = 100;
    m_header.type = DAQ::V8::SCALERBF;
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

    m_bytes << m_header;
    for (int i=0; i<10; ++i) {
      m_bytes << std::uint16_t(0);
    }

    m_buffer.setBuffer(m_bytes);

  }

  void tearDown()
  {
  }

void setBuffer_0() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs size properly",
                               std::uint16_t(100), m_buffer.getHeader().nwds );
}

void setBuffer_1() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs type properly",
                               DAQ::V8::SCALERBF, DAQ::V8::BufferTypes(m_buffer.getHeader().type) );
}

void setBuffer_2() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs run",
                               std::uint16_t(1), m_buffer.getHeader().run );
}

void setBuffer_3() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs seq",
                               std::uint32_t(2), m_buffer.getHeader().seq );
}


void setBuffer_4() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs buffmt",
                               DAQ::V8::StandardVsn, DAQ::V8::BufferVersion(m_buffer.getHeader().buffmt) );
}

void setBuffer_5() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs short byte order mark",
                               std::uint16_t(DAQ::V8::BOM16), m_buffer.getHeader().ssignature );
}

void setBuffer_6() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("setBuffer header constructs short byte order mark",
                               std::uint32_t(DAQ::V8::BOM32), m_buffer.getHeader().lsignature );
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(rawbuffertest);



