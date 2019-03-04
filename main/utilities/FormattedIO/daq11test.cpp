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
#include <RingIOV11.h>
#include <V11/CRingItem.h>
#include <V11/CRingFragmentItem.h>

#include <cstdint>
#include <vector>
#include <iostream>

using namespace std;
using namespace DAQ;

// A test suite 
class CFormattedIOV11Test : public CppUnit::TestFixture
{
    public:
    CFormattedIOV11Test();

    CPPUNIT_TEST_SUITE( CFormattedIOV11Test );
    CPPUNIT_TEST ( input_0 );
    CPPUNIT_TEST ( input_1 );
    CPPUNIT_TEST ( output_0 );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void input_0();
    void input_1();
    void output_0();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFormattedIOV11Test );

CFormattedIOV11Test::CFormattedIOV11Test()
{}


void CFormattedIOV11Test::setUp()
{}

void CFormattedIOV11Test::tearDown()
{}

void CFormattedIOV11Test::input_0()
{
  CTestSourceSink ss(128);
  // Load the source/sink with data 
  const uint32_t data[] = {28, 30, 0, 0, 1, 2, 3};
  ss.put(data, sizeof(data));

  // extract a complete ring item
  V11::CRingItem item(1);
  ss >> item;

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size after extraction should be correct",
                               uint32_t(28), item.size());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracted type should be correct",
                               uint32_t(30), item.type());    

  auto pBody = reinterpret_cast<uint32_t*>(item.getBodyPointer());

  CPPUNIT_ASSERT_MESSAGE("Body of extracted should be correct",
                          equal(pBody, pBody+4, data+3));
}

void CFormattedIOV11Test::input_1()
{
  CTestSourceSink ss(128);
  // Load the source/sink with data (i.e item with body header) 
  const uint32_t data[] = {36, 30, 20, 1, 0, 2, 3, 0, 0};
  ss.put(data, sizeof(data));

  // extract a complete ring item
  V11::CRingItem item(1);
  ss >> item;

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Size after extraction should be correct",
                               uint32_t(36), item.size());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracted type should be correct",
                               uint32_t(30), item.type());    

  auto pBody = reinterpret_cast<uint32_t*>(item.getBodyPointer());

  CPPUNIT_ASSERT_MESSAGE("Body of extracted should be correct",
                          equal(pBody, pBody, data+7));

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Body size should be correct",
                               size_t(8), item.getBodySize());    
}

void CFormattedIOV11Test::output_0()
{
  CTestSourceSink sink(128);

  vector<uint8_t> payload = {0, 1, 2, 3, 4, 5, 6};
  V11::CRingFragmentItem item(100, 2, payload.size(), payload.data(), 2);

  sink << item;

  auto record = sink.getBuffer();

  uint8_t* pOrig = reinterpret_cast<uint8_t*>(item.getItemPointer());

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of bytes written are correct",
                               size_t(item.size()), record.size());

  CPPUNIT_ASSERT_MESSAGE("What was written actually was written",
                          equal(record.begin(), record.end(), pOrig));


}

