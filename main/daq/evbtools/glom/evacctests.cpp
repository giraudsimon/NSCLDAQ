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
  
  CPPUNIT_TEST(freespace_1);
  CPPUNIT_TEST(freespace_2);
  
  CPPUNIT_TEST(itemtype_1);
  
  CPPUNIT_TEST(reservesize_1);
  
  CPPUNIT_TEST(appendf_1);
  CPPUNIT_TEST(appendf_2);
  
  CPPUNIT_TEST(addfrag_1);
  CPPUNIT_TEST(finish_1);    // Need this to test other branches of addFragment
  CPPUNIT_TEST(addfrag_2);   // fragment with same type appends.
  CPPUNIT_TEST(addfrag_3);   // Fragment with different type finishes prior.
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
    delete m_pTestObj;    // This first as we may flush.
    close(m_fd);
    unlink(m_filename.c_str());
  }
protected:
  void construct();
  
  void allocinfo_1();
  void allocinfo_2();
  void freeinfo_1();
  
  void sizeiov_1();
  
  void freespace_1();
  void freespace_2();
  
  void itemtype_1();
  
  void reservesize_1();
  
  void appendf_1();
  void appendf_2();
  
  void addfrag_1();
  void finish_1();
  void addfrag_2();
  void addfrag_3();
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
void evaccTest::freespace_1()  // initially the whole buffer is free:
{
  EQ(m_pTestObj->m_nBufferSize, m_pTestObj->freeSpace());
}
void evaccTest::freespace_2()
{
  // Indicate some is used:
  
  m_pTestObj->m_nBytesInBuffer = 100;
  EQ(m_pTestObj->m_nBufferSize - 100, m_pTestObj->freeSpace());
}
void evaccTest::itemtype_1()
{
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
    
  EQ(PHYSICS_EVENT, m_pTestObj->itemType(reinterpret_cast<EVB::pFlatFragment>(pHdr)));
}

void evaccTest::reservesize_1()
{
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  
  EQ(sizeof(uint32_t), m_pTestObj->m_nBytesInBuffer);
  
  // Now the info block:
  
  uint8_t* pBeg = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(sizeof(uint32_t), size_t(pNext - pBeg));
  EQ(sizeof(uint32_t), pInfo->s_eventInfo.s_nBytes);
  EQ(size_t(0), pInfo->s_eventInfo.s_nFragments);
  
  // The actual size field has sizeof(uint32_t) as well:
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(uint32_t)), *pSize);
}
void evaccTest::appendf_1()
{
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  m_pTestObj->appendFragment(pFrag);
  
  // let's see how pInfo was updated.  The data in the buffer is the
  // ring item body and body header.... preceded by an updated event size
  
  // Event size in buffer; and ring item copied to buffer.
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pItem, pSize+1, pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)),
    sz
  );
  
  // Body header timestamp should match the fragment's since policy defaults
  // was set to be last:
  
  EQ(pBh->s_timestamp, pInfo->s_eventHeader.s_bodyHeader.s_timestamp);
}
void evaccTest::appendf_2()
{
  m_pTestObj->m_tsPolicy=CEventAccumulator::average;
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  
  CEventAccumulator::pEventInformation pInfo =
    m_pTestObj->allocEventInfo(pFrag, 1);
  m_pTestObj->m_pCurrentEvent = pInfo;
  m_pTestObj->reserveSize();
  m_pTestObj->appendFragment(pFrag);

  // The info's timestamp sum should be reflected:
  
  EQ(uint64_t(0x12345678), pInfo->s_eventInfo.s_TimestampTotal);
  
  // If we throw the item at it again:
  
  m_pTestObj->appendFragment(pFrag);
  EQ(2*uint64_t(0x12345678), pInfo->s_eventInfo.s_TimestampTotal);
}
void evaccTest::addfrag_1()
{
  // New fragment -- check
  
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  // There'd better be a current event:
  
  ASSERT(m_pTestObj->m_pCurrentEvent);
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  // The current event content should just be like in appendf_1():
  
   // let's see how pInfo was updated.  The data in the buffer is the
  // ring item body and body header.... preceded by an updated event size
  
  // Event size in buffer; and ring item copied to buffer.
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pItem, pSize+1, pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)),
    sz
  );
}
// we need to see an event finished properly before we can
// check the other branches of addFragment so:

void evaccTest::finish_1()
{
  // New fragment -- check
  
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  m_pTestObj->finishEvent();
  
  // There should be 1 frags in buffer - and it should match addfrag_1
  
  EQ(size_t(1), m_pTestObj->m_fragsInBuffer.size());
  auto pInfo = m_pTestObj->m_fragsInBuffer.front();
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pItem, pSize+1, pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)),
    sz
  );
}
void evaccTest::addfrag_2()    // append fragment current:
{
  // New fragment -- check
  
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  pHdr->s_timestamp+= 0x100;     // 0x12345778
  m_pTestObj->addFragment(pFrag, 2);
  
  EQ(size_t(0), m_pTestObj->m_fragsInBuffer.size());
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  // The event has two frags equally sized, identical other than for the
  // timestamps.
  
  
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  uint32_t size =
    2*(sizeof(BodyHeader) + sizeof(RingItemHeader)) + sizeof(uint32_t);
  EQ(size , *pSize);
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(size_t(size), a.s_nBytes);
  EQ(size_t(2), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(size_t(size), size_t(pNext - pBase));
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(size, sz);

  // There will be two fragments in the buffer:
  
  // Fragment 1:
  
  pHdr->s_timestamp = 0x12345678;           // original value
  uint8_t* pFrag1 = static_cast<uint8_t*>(m_pTestObj->m_pBuffer) + sizeof(uint32_t);
  ASSERT(memcmp(pItem, pFrag1, pItem->s_size) == 0);
  
  // Fragment 2:
  
  pHdr->s_timestamp += 0x100;
  auto pFrag2      = pFrag1 +  pItem->s_size;
  ASSERT(memcmp(pItem, pFrag2, pItem->s_size)== 0);
}
void evaccTest::addfrag_3()   // Adding a fragment of a different type ends event
{
  // New fragment -- check
  
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
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pHdr);
  m_pTestObj->addFragment(pFrag, 2);
  
  pItem->s_type = END_RUN;
  m_pTestObj->addFragment(pFrag, 2);
  
  // There should be a fragment in the buffer and a current fragment
  // that is our stub of an end run item:
  
  EQ(size_t(1), m_pTestObj->m_fragsInBuffer.size());
  ASSERT(m_pTestObj->m_pCurrentEvent);
}