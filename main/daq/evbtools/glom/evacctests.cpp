// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CEventAccumulator.h"            // White box testing.
#undef private

#include <fragment.h>
#include <DataFormat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

std::string FilenameTemplate="evactestXXXXXX";

class evaccTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(evaccTest);
  CPPUNIT_TEST(construct);
  
  CPPUNIT_TEST(allocinfo_1);
  CPPUNIT_TEST(allocinfo_2);
  CPPUNIT_TEST(freeinfo_1);
  
  CPPUNIT_TEST(sizeiov_1);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string        m_filename;
  int                m_fd;
  CEventAccumulator* m_pTestObj;
public:
  void setUp() {
    char fnameTemplate[FilenameTemplate.size() +1];
    strcpy(fnameTemplate, FilenameTemplate.c_str());
    m_fd = mkstemp(fnameTemplate);
    m_filename = fnameTemplate;
    
    m_pTestObj =
      new CEventAccumulator(m_fd, 1, 1024, 10, CEventAccumulator::last);
    
  }
  void tearDown() {
    close(m_fd);
    delete m_pTestObj;
    unlink(m_filename.c_str());
  }
protected:
  void construct();
  
  void allocinfo_1();
  void allocinfo_2();
  void freeinfo_1();
  
  void sizeiov_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(evaccTest);

void evaccTest::construct() {
  EQ(m_fd, m_pTestObj->m_nFd);
  EQ(time_t(1), m_pTestObj->m_maxFlushTime);
  
  time_t now = time_t(nullptr);
  ASSERT((now - m_pTestObj->m_lastFlushTime) <= 1);
  EQ(CEventAccumulator::last, m_pTestObj->m_tsPolicy);
  ASSERT(m_pTestObj->m_pBuffer);
  EQ(size_t(1024), m_pTestObj->m_nBufferSize);
  EQ(size_t(0), m_pTestObj->m_nBytesInBuffer);
  
  EQ(size_t(0), m_pTestObj->m_fragsInBuffer.size());
  EQ(m_pTestObj->m_nMaxFrags, m_pTestObj->m_freeFrags.size());
  ASSERT(!m_pTestObj->m_pCurrentEvent);
  
  ASSERT(!m_pTestObj->m_pIoVectors);
  EQ(size_t(0), m_pTestObj->m_nMaxIoVecs);
  EQ(size_t(0), m_pTestObj->m_nIoVecs);
}


void evaccTest::allocinfo_1()
{
  // Make us a fragment:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  CEventAccumulator::pEventInformation pInfo;
  CPPUNIT_ASSERT_NO_THROW(
    pInfo =
      m_pTestObj->allocEventInfo(reinterpret_cast<EVB::pFlatFragment>(pHdr), 1)
  );
  
  // Now check the contents:
  
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pBodyStart);
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pInsertionPoint);
  
  CEventAccumulator::EventAccumulation& Ac(pInfo->s_eventInfo);
  EQ(size_t(0), Ac.s_nBytes);
  EQ(size_t(0), Ac.s_nFragments);
  EQ(uint64_t(0), Ac.s_TimestampTotal);
  
  RingItemHeader& Ih(pInfo->s_eventHeader.s_itemHeader);
  BodyHeader&     Bh(pInfo->s_eventHeader.s_bodyHeader);
  
  EQ(uint32_t(0), Ih.s_size);
  EQ(PHYSICS_EVENT, Ih.s_type);
  
  EQ(NULL_TIMESTAMP, Bh.s_timestamp);
  EQ(uint32_t(1), Bh.s_sourceId);
  EQ(uint32_t(0), Bh.s_barrier);
  EQ(uint32_t(sizeof(BodyHeader)), Bh.s_size);
}

void evaccTest::allocinfo_2() // If ts policy is first, init with ts.
{
  m_pTestObj->m_tsPolicy  = CEventAccumulator::first;
  
    // Make us a fragment:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_sourceId    = 1;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);;
  pRingItemHeader pItem =
    reinterpret_cast<pRingItemHeader>(pHdr+1);
  pItem->s_type = PHYSICS_EVENT;
  pItem->s_size = pHdr->s_size;
  
  pBodyHeader pBh    = reinterpret_cast<pBodyHeader>(pItem+1);
  pBh->s_timestamp   = pHdr->s_timestamp;
  pBh->s_sourceId    = pHdr->s_sourceId;
  pBh->s_size        = sizeof(BodyHeader);
  pBh->s_barrier     = 0;
  
  CEventAccumulator::pEventInformation pInfo;
  CPPUNIT_ASSERT_NO_THROW(
    pInfo =
      m_pTestObj->allocEventInfo(reinterpret_cast<EVB::pFlatFragment>(pHdr), 1)
  );
  
  // For now assume all the rest is ok:
  BodyHeader&     Bh(pInfo->s_eventHeader.s_bodyHeader);
  EQ(pHdr->s_timestamp, Bh.s_timestamp);
}

void evaccTest::freeinfo_1()
{
  size_t nFree = m_pTestObj->m_freeFrags.size();
  nFree++;                              // expected value.
  
  CEventAccumulator::pEventInformation pI = new CEventAccumulator::EventInformation;
  m_pTestObj->freeEventInfo(pI);
  EQ(nFree, m_pTestObj->m_freeFrags.size());
}
void evaccTest::sizeiov_1()
{
  CPPUNIT_ASSERT_NO_THROW(
    m_pTestObj->sizeIoVecs(100)
  );
  ASSERT(m_pTestObj->m_pIoVectors);
  
  EQ(size_t(100), m_pTestObj->m_nMaxIoVecs);
}