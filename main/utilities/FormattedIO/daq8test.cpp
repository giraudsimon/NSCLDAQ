/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include <CTestSourceSink.h>
#include <BufferIOV8.h>
#include <V8/DataFormatV8.h>
#include <V8/CRawBuffer.h>

#include <DebugUtils.h>

#include <cstdint>
#include <vector>
#include <iostream>

using namespace std;

// DAQ::V8::gBufferSize is defined in TestRunner.cpp to be 34. This small size makes it easier
// to run tests concisely.

// A test suite
class CFormattedIOV8Test : public CppUnit::TestFixture
{
    public:
    CFormattedIOV8Test();

    CPPUNIT_TEST_SUITE( CFormattedIOV8Test );
    CPPUNIT_TEST ( extract_0 );
    CPPUNIT_TEST ( insert_0 );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void extract_0();
    void insert_0();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFormattedIOV8Test );

CFormattedIOV8Test::CFormattedIOV8Test()
{}


void CFormattedIOV8Test::setUp()
{}

void CFormattedIOV8Test::tearDown()
{}

void CFormattedIOV8Test::extract_0()
{
  CTestSourceSink ss(34);
  std::vector<std::uint16_t> data = {0x0001, 0x0011,
                                     0, 0, 0, 0,
                                     0, 0, 0, 5,
                                     0x0102, 0x0102, 0x0304, 0, 0,
                                     0, 0};
  DAQ::Buffer::ByteBuffer buffer;
  buffer << data;

  // Load the source/sink with data
  ss.put(buffer.data(), buffer.size()*sizeof(std::uint16_t));

  DAQ::V8::CRawBuffer rawBuf;
  ss >> rawBuf;

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Extraction operator retrieves data we expect",
                               buffer, rawBuf.getBuffer());

}

void CFormattedIOV8Test::insert_0()
{
  CTestSourceSink ss(34);
  std::vector<std::uint16_t> data = {0x0001, 0x0011,
                                     0, 0, 0, 0,
                                     0, 0, 0, 5,
                                     0x0102, 0x0102, 0x0304, 0, 0,
                                     0, 0};
  DAQ::Buffer::ByteBuffer buffer;
  buffer << data;

  CTestSourceSink sink(34);
  DAQ::V8::CRawBuffer rawBuf;
  rawBuf.setBuffer(buffer);

  sink << rawBuf;

  auto record = sink.getBuffer();

  std::vector<char> charBuffer(rawBuf.getBuffer().begin(), rawBuf.getBuffer().end());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of bytes written are correct",
                               rawBuf.getBuffer().size(), record.size());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("What was written actually was written",
                               charBuffer, record);


}

