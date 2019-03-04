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


#include <Asserts.h>

#include <cppunit/extensions/HelperMacros.h>

#include <CRingItem.h>
#include <CPhysicsEventItem.h>
#include <CRingStateChangeItem.h>
#include <DataFormat.h>
#include <CTestRingBuffer.h>
#define private public
#define protected public
#include "CRingSource.h"
#undef private
#undef protected

#include <array>
#include <vector>
#include <limits>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

static uint64_t tstamp(_PhysicsEventItem*) { return 1; }

// A test suite 
class CRingSourceTest : public CppUnit::TestFixture
{

  private:
    CRingSource* m_pSource;
    CTestRingBuffer *m_pRing;
    bool m_ownRing;

  public:

    CPPUNIT_TEST_SUITE( CRingSourceTest );
    CPPUNIT_TEST(getEvent_0);
    CPPUNIT_TEST(getEvent_1);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp() {
      try {
        CRingBuffer::create("__test__");
        m_ownRing = true;
      } catch (...) {
        m_ownRing = false;
      }

      std::vector<uint32_t> okids;
      okids.push_back(2);
      m_pRing = new CTestRingBuffer("__test__");
      m_pSource = (new CRingSource(m_pRing,
            okids, 2, tstamp));
    }

    void tearDown() {
      delete m_pSource;
      delete m_pRing;

      if (m_ownRing) {
	return;
        CRingBuffer::remove("__test__");
      }
    }
protected:
  void getEvent_0();
  void getEvent_1();
private:
  void fillBody(CRingItem& item);
};
// Register it with the test factory

CPPUNIT_TEST_SUITE_REGISTRATION( CRingSourceTest );

    
void CRingSourceTest::getEvent_0() {
      CPhysicsEventItem item;
      item.setBodyHeader(1, 2, 0);
      fillBody(item);
      item.updateSize();

      m_pRing->put(item.getItemPointer(), item.size());

      auto pBuf = new uint8_t[1000];
      m_pSource->transformAvailableData(pBuf);
      delete [] pBuf;

      ASSERT( m_pSource->getFragmentList().size() == 1);
    }

void CRingSourceTest::getEvent_1() {

      m_pSource->setOneshot(true);
      m_pSource->setNumberOfSources(2);
      CRingStateChangeItem begin(BEGIN_RUN);
      CRingStateChangeItem end(END_RUN);
      m_pRing->put(end.getItemPointer(), end.size());
      m_pRing->put(end.getItemPointer(), end.size());

      auto pBuf = new uint8_t[1000];
      m_pSource->transformAvailableData(pBuf);
      delete [] pBuf;

      EQMSG("Observation of 2 end runs for 2 sources, oneshot -> complete",
          true, m_pSource->oneshotComplete());
    }


void CRingSourceTest::fillBody(CRingItem& item) {
      vector<uint8_t> data = {0, 1, 2, 3, 4, 5, 6, 7};
      uint8_t* pData = reinterpret_cast<uint8_t*>(item.getBodyPointer()); 
      pData = copy(data.begin(), data.end(), pData);
      item.setBodyCursor(pData);
      item.updateSize();

    }





