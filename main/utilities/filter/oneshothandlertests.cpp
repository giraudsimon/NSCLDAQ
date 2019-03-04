/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

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


#include <cppunit/extensions/HelperMacros.h>
#include <algorithm>
#include <COneShotException.h>
#include <stdint.h>
#include <limits>
#include <CRingStateChangeItem.h>
#include <CDataFormatItem.h>

#define private public
#define protected public
#include "COneShotHandler.h"
#undef private
#undef protected 

// A test suite 
class COneShotHandlerTest : public CppUnit::TestFixture
{
  public:
    COneShotHandlerTest();

    CPPUNIT_TEST_SUITE( COneShotHandlerTest );
    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testWaitForBegin );
    CPPUNIT_TEST ( testCount );
    CPPUNIT_TEST ( testSkipUntilBegin );
    CPPUNIT_TEST ( testThrowOnExtraStateChange );
    CPPUNIT_TEST ( testThrowOnRunNoChange );
    CPPUNIT_TEST ( testBecomesComplete );
    CPPUNIT_TEST ( testTooManyBegins );
    CPPUNIT_TEST ( testWaitForBegin_1 );
    CPPUNIT_TEST ( testUpdateNull );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();
    void testWaitForBegin();
    void testCount();
    void testComplete();
    void testSkipUntilBegin();
    void testThrowOnExtraStateChange();
    void testThrowOnRunNoChange();
    void testBecomesComplete();
    void testTooManyBegins();
    void testWaitForBegin_1();
    void testUpdateNull();
};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( COneShotHandlerTest );

COneShotHandlerTest::COneShotHandlerTest()
{}

void COneShotHandlerTest::setUp()
{
}

void COneShotHandlerTest::tearDown()
{
}


void COneShotHandlerTest::testConstructor()
{
  uint32_t nsources= 1;
  COneShotHandler handler(nsources);

  CPPUNIT_ASSERT_EQUAL(nsources, handler.m_nExpectedSources);
  CPPUNIT_ASSERT_EQUAL(4, (int)handler.m_stateCounts.size());
  CPPUNIT_ASSERT_EQUAL(false, handler.m_complete);
  CPPUNIT_ASSERT_EQUAL(std::numeric_limits<uint32_t>::max(), handler.m_cachedRunNo);

  std::map<uint32_t,uint32_t>::iterator it = handler.m_stateCounts.begin();
  std::map<uint32_t,uint32_t>::iterator itend = handler.m_stateCounts.end();
  
  int i=1;
  while (it != itend) {
    CPPUNIT_ASSERT_EQUAL(i, (int) it->first);
    CPPUNIT_ASSERT_EQUAL(0, (int) it->second);

    ++it;
    ++i;
  } 
  
}

void COneShotHandlerTest::testWaitForBegin()
{
  uint32_t nsources= 2;
  COneShotHandler handler(nsources);
  
  CRingStateChangeItem begin0(BEGIN_RUN);
  begin0.setRunNumber(40);
  
  // We should be waiting
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());
  
  // Handle an event
  handler.update(&begin0);

  // we should not longer be waiting
  CPPUNIT_ASSERT_EQUAL(false, handler.waitingForBegin());
  

}


void COneShotHandlerTest::testCount()
{
  COneShotHandler handler(0);
  handler.m_stateCounts[BEGIN_RUN]=1; 
  handler.m_stateCounts[END_RUN]=2; 
  handler.m_stateCounts[PAUSE_RUN]=3; 
  handler.m_stateCounts[RESUME_RUN]=4;

  CPPUNIT_ASSERT_EQUAL(1, (int) handler.getCount(BEGIN_RUN));
  CPPUNIT_ASSERT_EQUAL(2, (int) handler.getCount(END_RUN));
  CPPUNIT_ASSERT_EQUAL(3, (int) handler.getCount(PAUSE_RUN));
  CPPUNIT_ASSERT_EQUAL(4, (int) handler.getCount(RESUME_RUN));
}


void COneShotHandlerTest::testComplete()
{
  COneShotHandler handler(1);
  
  CRingStateChangeItem begin(BEGIN_RUN);
  handler.update(&begin);

  CPPUNIT_ASSERT_EQUAL(false,handler.complete());
}

void COneShotHandlerTest::testSkipUntilBegin()
{
  COneShotHandler handler(1);
  
  CRingStateChangeItem pause(PAUSE_RUN);
  std::cout << "Before : " << handler.getCount(PAUSE_RUN) << std::endl;
  handler.update(&pause);

  CPPUNIT_ASSERT_EQUAL(0, (int)handler.getCount(PAUSE_RUN));
}


void COneShotHandlerTest::testThrowOnExtraStateChange()
{
  COneShotHandler handler(1);
  handler.m_complete = true;
  
  CRingStateChangeItem pause(PAUSE_RUN);
  CPPUNIT_ASSERT_THROW(handler.update(&pause), COneShotException);
  
}

void COneShotHandlerTest::testThrowOnRunNoChange()
{
  COneShotHandler handler(1);
  handler.m_cachedRunNo = 3;
  
  CRingStateChangeItem end(END_RUN);
  end.setRunNumber(30);
  CPPUNIT_ASSERT_THROW(handler.update(&end), COneShotException);
  
  CRingStateChangeItem begin(BEGIN_RUN);
  begin.setRunNumber(30);
  CPPUNIT_ASSERT_THROW(handler.update(&begin), COneShotException);

  CRingStateChangeItem pause(PAUSE_RUN);
  pause.setRunNumber(30);
  CPPUNIT_ASSERT_THROW(handler.update(&pause), COneShotException);

  CRingStateChangeItem resume(RESUME_RUN);
  resume.setRunNumber(30);
  CPPUNIT_ASSERT_THROW(handler.update(&resume), COneShotException);
}


void COneShotHandlerTest::testBecomesComplete()
{
  COneShotHandler handler(1);
  // make sure that we are not waitingForBegin
  handler.m_stateCounts[BEGIN_RUN] = 1;

  CPPUNIT_ASSERT_EQUAL(false, handler.complete());

  CRingStateChangeItem end(END_RUN);
  handler.update(&end);
  CPPUNIT_ASSERT_EQUAL(true, handler.complete());

}

void COneShotHandlerTest::testTooManyBegins()
{
  COneShotHandler handler(1);
  handler.m_stateCounts[BEGIN_RUN] = 1;

  CRingStateChangeItem begin(BEGIN_RUN);
  
  CPPUNIT_ASSERT_THROW(handler.update(&begin),COneShotException);
}

void COneShotHandlerTest::testUpdateNull()
{
  COneShotHandler handler(1);

  CPPUNIT_ASSERT_THROW(handler.update(0),COneShotException);
}

void COneShotHandlerTest::testWaitForBegin_1()
{
  uint32_t nsources= 2;
  COneShotHandler handler(nsources);
  
  CDataFormatItem format;
  CRingStateChangeItem begin0(BEGIN_RUN);
  begin0.setRunNumber(40);
  
  // We should be waiting
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());
  
  // Handle an event
  handler.update(&format);

  // we should still be waiting 
  CPPUNIT_ASSERT_EQUAL(true, handler.waitingForBegin());

  handler.update(&begin0);

  // we should not longer be waiting
  CPPUNIT_ASSERT_EQUAL(false, handler.waitingForBegin());
  

}
