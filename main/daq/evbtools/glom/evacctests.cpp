// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "CEventAccumulator.h"            // White box testing.
#undef private

#include <fragment.h>
#include <DataFormat.h>
#include <CRingScalerItem.h>
#include <CFileDataSource.h>
#include <CRingItem.h>
#include <URL.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <time.h>

static std::string FilenameTemplate="evactestXXXXXX";
static uint32_t headerSize(sizeof(RingItemHeader) + sizeof(BodyHeader));

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
  CPPUNIT_TEST(addfrag_4);   // events need flushing to make space.
  CPPUNIT_TEST(addfrag_5);   // Adding a fragment to an event would overflow.
  CPPUNIT_TEST(addfrag_6);   // Fragment bigger than buffer by itself.
  CPPUNIT_TEST(addfrag_7);   // Fragment overflows maxfrags/event.
  CPPUNIT_TEST(addfrag_8);
  CPPUNIT_TEST(addfrag_9);   // Test slide doesn't fail.
  CPPUNIT_TEST(addfrag_10);
  
  CPPUNIT_TEST(flush_1);    // Event flushing. tests.
  CPPUNIT_TEST(flush_2);
  CPPUNIT_TEST(flush_3);
  CPPUNIT_TEST(flush_4);
  CPPUNIT_TEST(flush_5);
  
  CPPUNIT_TEST(oob_1);       // out of band fragment tests.
  CPPUNIT_TEST(oob_2);
  CPPUNIT_TEST(oob_3);
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
  void addfrag_4();
  void addfrag_5();
  void addfrag_6();            // Fragment bigger than buffer.
  void addfrag_7();
  void addfrag_8();
  void addfrag_9();
  void addfrag_10();
  
  void flush_1();
  void flush_2();
  void flush_3();
  void flush_4();
  void flush_5();
  
  void oob_1();
  void oob_2();
  void oob_3();
private:
  URL makeUri();
};  

CPPUNIT_TEST_SUITE_REGISTRATION(evaccTest);


static int randint(int low, int high)
{
  int range = high - low;
  double r  = drand48() * range;   // random in range.
  return int(r + low);           // should be beweeen [low, high).
}
static std::vector<uint16_t>
randomBody(uint16_t* pBody, size_t nBytes)
{
  std::vector<uint16_t> result;
  for (int i =0; i < nBytes; i++) {
    uint16_t n = uint16_t(randint(0, 65536));
    *pBody++ = n;
    result.push_back(n);
  }
  return result;
}

URL
evaccTest::makeUri()
{
  std::string uri="file://./";
  uri += m_filename;
  URL url(uri);
  return url;
}

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
  
  EQ(uint32_t(headerSize), Ih.s_size);
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
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)),
    *pSize
  );
  EQ(0, memcmp(pFrag, pSize+1, *pSize - sizeof(uint32_t)));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t) + headerSize),
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
  EQ(uint32_t(sizeof(EVB::FragmentHeader)+sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pFrag, pSize+1, sizeof(EVB::FragmentHeader) + pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)+headerSize),
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
  EQ(uint32_t(sizeof(EVB::FragmentHeader)+sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)), *pSize);
  EQ(0, memcmp(pFrag, pSize+1, sizeof(EVB::FragmentHeader) + pItem->s_size));
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  // Insert pointer updated:
  
  uint8_t* pBase = static_cast<uint8_t*>(pInfo->s_pBodyStart);
  uint8_t* pNext = static_cast<uint8_t*>(pInfo->s_pInsertionPoint);
  EQ(
    sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t),
    size_t(pNext - pBase)
  );
  // Ring item size is updated:
  
  uint32_t sz = pInfo->s_eventHeader.s_itemHeader.s_size;
  EQ(
    uint32_t(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader) + sizeof(uint32_t)+headerSize),
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
    2*(sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader)) + sizeof(uint32_t);
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
  EQ(size + headerSize, sz);

  // There will be two fragments in the buffer:
  
  // Fragment 1:
  
  pHdr->s_timestamp = 0x12345678;           // original value
  uint8_t* pFrag1 = static_cast<uint8_t*>(m_pTestObj->m_pBuffer) + sizeof(uint32_t);
  ASSERT(memcmp(pFrag, pFrag1, sizeof(EVB::FragmentHeader) + pItem->s_size) == 0);
  
  // Fragment 2:
  
  pHdr->s_timestamp += 0x100;
  auto pFrag2      = pFrag1 +  pItem->s_size + sizeof(EVB::FragmentHeader);
  ASSERT(memcmp(pFrag, pFrag2, sizeof(EVB::FragmentHeader) + pItem->s_size)== 0);
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
  auto pInfo = m_pTestObj->m_pCurrentEvent;
 
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  uint32_t size =
    (sizeof(EVB::FragmentHeader) + sizeof(BodyHeader) + sizeof(RingItemHeader)) + sizeof(uint32_t);
  EQ(size , *pSize);
  
  // event info updated:
  
  CEventAccumulator::EventAccumulation& a(pInfo->s_eventInfo);
  EQ(size_t(size), a.s_nBytes);
  EQ(size_t(1), a.s_nFragments);
  
  uint32_t* pFrag1 = pSize+1;
  ASSERT(memcmp(pFrag, pFrag1, sizeof(EVB::FragmentHeader) + pItem->s_size) ==0);
}
void evaccTest::addfrag_4()
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
  m_pTestObj->addFragment(pFrag, 2);
  m_pTestObj->finishEvent();
  
  // Fake like threre's no space for the next fragment:
  
  m_pTestObj->m_nBufferSize = m_pTestObj->m_nBytesInBuffer + 10;
  
  m_pTestObj->addFragment(pFrag, 2);
  
  // This fragment should be at the start of the buffer;
  // as slide should have happened.
  
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  
  EQ(m_pTestObj->m_pBuffer, pInfo->s_pBodyStart);
  
  // Check body end:
  
  size_t size =
    static_cast<uint8_t*>(pInfo->s_pInsertionPoint)  -
    static_cast<uint8_t*>(pInfo->s_pBodyStart);
  EQ(sizeof(EVB::FragmentHeader) + pItem->s_size, size - sizeof(uint32_t));
  
  // The data that needed sliding was the size uint32_t:
  uint32_t* pSize = static_cast<uint32_t*>(pInfo->s_pBodyStart);
  EQ(sizeof(EVB::FragmentHeader) + pItem->s_size, *pSize - sizeof(uint32_t));
  
  
}
// If we attempt to add a fragment to the current event that would cause it to
// overflow, we terminate the event, and start a new one with the new fragment.


void evaccTest::addfrag_5()
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
  m_pTestObj->addFragment(pFrag, 2);
  
  // Make it so the next addfrag will overflow:
  
  m_pTestObj->m_nBufferSize = m_pTestObj->m_nBytesInBuffer + 10;
  
  m_pTestObj->addFragment(pFrag, 2);
  
  // this should be the only fragment in the event, as this caused a finish
  // followed by a flush:
  
  auto pInfo = m_pTestObj->m_pCurrentEvent;
  EQ(size_t(1), pInfo->s_eventInfo.s_nFragments);  // no over flow ths is 2.
  
  
}
void evaccTest::addfrag_6()
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
  
  m_pTestObj->m_nBufferSize =10;
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->addFragment(pFrag, 2),
    std::range_error
  );
}
void evaccTest::addfrag_7()
{
  m_pTestObj->m_nMaxFrags = 2;
  
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
  m_pTestObj->addFragment(pFrag, 2);   // forced an end of event
  
  ASSERT(!m_pTestObj->m_pCurrentEvent);  //finished implicitly.
  
  // there's one event in the buffer and it has our fragments:
  
}
void evaccTest::flush_1()
{
  // immediate flush results in no data in file:
  
  m_pTestObj->flushEvents();
  
  struct stat info;
  stat(m_filename.c_str(), &info);
  EQ(off_t(0), info.st_size);
}
void evaccTest::flush_2()
{
  // flush does not finish a partial event:
  
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
  
  m_pTestObj->flushEvents();

  struct stat info;
  stat(m_filename.c_str(), &info);
  EQ(off_t(0), info.st_size);  
}
void evaccTest::flush_3()
{
  // Flush an event with one fragment:
  
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
  m_pTestObj->flushEvents();
  
  struct stat info;
  stat(m_filename.c_str(), &info);
  ASSERT(info.st_size > 0);
  
  // Contents should be the size uint32_t, and the flattened fragment.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t readBuffer[1024];
  

  ssize_t nRead = read(fd, readBuffer, sizeof(readBuffer));
  EQ(ssize_t(
    sizeof(RingItemHeader) + sizeof(BodyHeader)  +  // Full ring item header
    sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size    // size of the 1 fragment.
  ), nRead);
  
  // First shoulid be  ring item header for the entire event:
  
  pRingItemHeader pRHdr = reinterpret_cast<pRingItemHeader>(readBuffer);
  EQ(PHYSICS_EVENT, pRHdr->s_type);
  EQ(
    uint32_t(sizeof(RingItemHeader) + sizeof(BodyHeader)  +  // Full ring item header
    sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size ), pRHdr->s_size);
  // Next the uint32_t that is the size of the remaining event:
  
  pRHdr++;                 // Points to BodyHeader....
  pBodyHeader pReadBhdr = reinterpret_cast<pBodyHeader>(pRHdr);
  pReadBhdr++;
  
  uint32_t* payloadSize = reinterpret_cast<uint32_t*>(pReadBhdr);
  EQ(
    uint32_t(sizeof(uint32_t) +                             // size field.
    sizeof(EVB::FragmentHeader) + pHdr->s_size ),
    *payloadSize
  );
  // After that is the first fragment:
  
  payloadSize++;
  EVB::pFlatFragment pReadFrag =
    reinterpret_cast<EVB::pFlatFragment>(payloadSize);
  ASSERT(memcmp(pFrag, pReadFrag, sizeof(EVB::FragmentHeader) + pHdr->s_size) == 0);
}
void evaccTest::flush_4()
{
    // One event, a couple of fragments.

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
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
  pHdr->s_sourceId = 1;           // Different source id.
  pHdr->s_timestamp = 0x12345679; // Slightly different timestamp.
  m_pTestObj->addFragment(pFrag, 2);
  
  m_pTestObj->finishEvent();
  m_pTestObj->flushEvents();
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  
  // Suck in the entire file into a single buffer:
  
  uint8_t readBuffer[2048];
  ssize_t nRead = read(fd, readBuffer, sizeof(readBuffer));
  
  // Should be one item with two identically sized fragments:
  
  ASSERT(nRead < sizeof(readBuffer));
  
  uint8_t* p = readBuffer;
  pRingItemHeader pRH = reinterpret_cast<pRingItemHeader>(p);
  EQ(PHYSICS_EVENT, pRH->s_type);
  EQ(uint32_t(
    headerSize + sizeof(uint32_t) +
    2*(sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader))
  ), pRH->s_size);
  // Following the ring item header is a body header with the latest
  // timestamp:
  
  pBodyHeader pRBH = reinterpret_cast<pBodyHeader>(pRH+1);
  EQ(uint64_t(0x12345679), pRBH->s_timestamp);
  EQ(uint32_t(2), pRBH->s_sourceId);
  
  // Next is the uint32_t size of the fragment body:
  
  uint32_t* pPayloadSize = reinterpret_cast<uint32_t*>(pRBH+1);
  EQ(uint32_t(
    sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader))
  ), *pPayloadSize);
  // Now the first ring item:
  
  EVB::pFragmentHeader pRFH =
    reinterpret_cast<EVB::pFragmentHeader>(pPayloadSize+1);
  EQ(uint64_t(0x12345678), pRFH->s_timestamp);
  EQ(uint32_t(5), pRFH->s_sourceId);
  EQ(uint32_t(sizeof(RingItemHeader) + sizeof(BodyHeader)), pRFH->s_size);
  
  // Now compare the ring item
  
  pHdr->s_timestamp = 0x12345678; // (put things back the way they were.)
  pHdr->s_sourceId  = 5;
  ASSERT(memcmp(pRFH+1, pItem, pItem->s_size) == 0);
  
  // On to the next fragment:
  
  pRH = reinterpret_cast<pRingItemHeader>(pRFH+1);
  p   = reinterpret_cast<uint8_t*>(pRH);
  p  += pRH->s_size;
  
  // p points to the next fragment header:
  
  pRFH= reinterpret_cast<EVB::pFragmentHeader>(p);
  EQ(uint64_t(0x12345679), pRFH->s_timestamp);
  EQ(uint32_t(1), pRFH->s_sourceId);
  
}

void evaccTest::flush_5()
{
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
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
  
  m_pTestObj->addFragment(pFrag, 2);   // Event 1.
  m_pTestObj->finishEvent();
  
  pHdr->s_timestamp++;
  pBh->s_timestamp++;
  m_pTestObj->addFragment(pFrag, 2);   // Event 3.
  m_pTestObj->finishEvent();
  
  pHdr->s_timestamp++;
  pBh->s_timestamp++;
  m_pTestObj->addFragment(pFrag, 2);   // Event 3.
  m_pTestObj->finishEvent();          
  
  m_pTestObj->flushEvents();
  
  // Now read it all in in one gulp:
  
  uint8_t rdBuffer[8192];
  int fd = open(m_filename.c_str(), O_RDONLY);
  ssize_t nRead = read(fd, rdBuffer, sizeof(buffer));
  ASSERT(nRead < sizeof(rdBuffer));   // Make sure we got it all.
  
  // Each event consists of a ring header a body header, a uint32_t,
  // a fragment header, a ring  item header, and a body header:
  
  ssize_t expectedSize = 3*(
    sizeof(RingItemHeader)  + sizeof(BodyHeader)
    + sizeof(uint32_t) + sizeof(EVB::FragmentHeader) +
    + sizeof(RingItemHeader) + sizeof(BodyHeader));
  EQ(expectedSize, nRead);
  
  
  uint64_t expectedTs = 0x12345678;
  uint8_t* p = reinterpret_cast<uint8_t*>(rdBuffer);
  
  for (int i =0; i < 3; i++) {   // Loop over events.
    std::stringstream strMsg;
    strMsg << "Event: " << i;
    std::string msg = strMsg.str();
    
    // Ring item header for the event:
    
    pRingItemHeader pRHdr = reinterpret_cast<pRingItemHeader>(p);
    p += pRHdr->s_size;                 // Setup for next event.
    
    EQMSG(msg, PHYSICS_EVENT, pRHdr->s_type);
    EQMSG(msg, uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
      sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader)
    ), pRHdr->s_size);
    
    // Body header for the event as a whole:
    
    pBodyHeader pRBhdr = reinterpret_cast<pBodyHeader>(pRHdr +1);
    EQMSG(msg, expectedTs, pRBhdr->s_timestamp);
    EQMSG(msg, uint32_t(2), pRBhdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pRBhdr->s_barrier);
    
    // Size field:
    
    uint32_t* pFragSize = reinterpret_cast<uint32_t*>(pRBhdr+1);
    EQMSG(msg, uint32_t(
      sizeof(uint32_t) +
      sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) + sizeof(BodyHeader)      
    ) , *pFragSize);
    
    // Fragment header for the event:
    
    EVB::pFragmentHeader pFHdr =
      reinterpret_cast<EVB::pFragmentHeader>(pFragSize+1);
    EQMSG(msg, expectedTs, pFHdr->s_timestamp);
    EQMSG(msg, uint32_t(5), pFHdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pFHdr->s_barrier);
    EQMSG(msg, uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader)  
    ), pFHdr->s_size);
    
    // Ring item header for the event.
 
    pRHdr = reinterpret_cast<pRingItemHeader>(pFHdr+1);
    EQMSG(msg,uint32_t(
      sizeof(RingItemHeader) + sizeof(BodyHeader)
    ), pRHdr->s_size);
    EQMSG(msg, PHYSICS_EVENT, pRHdr->s_type);
    
    // body header for the event.
    
    pRBhdr = reinterpret_cast<pBodyHeader>(pRHdr+1);
    EQMSG(msg, expectedTs, pRBhdr->s_timestamp);
    EQMSG(msg, uint32_t(5), pRBhdr->s_sourceId);
    EQMSG(msg, uint32_t(0), pRBhdr->s_barrier);
    
    // next event has the next timestamp..
    
    expectedTs++;
  }
}
void evaccTest::oob_1()  
{
  // oob fragment when nothing's buffered gives the oob fragment.
  
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  uint8_t buffer[1024];
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  EVB::pFlatFragment pFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pFrag, 1);
  
  // The file should have the fragment's ring item;.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t rdBuffer[2048];
  ssize_t rdSize = read(fd, rdBuffer, sizeof(rdBuffer));
  ASSERT(rdSize < sizeof(rdBuffer));
  EQ(ssize_t(pOriginalItem->s_header.s_size), rdSize);
  ASSERT(memcmp(pOriginalItem, rdBuffer, rdSize) == 0);
}
void evaccTest::oob_2()
{
  // oob item when there's a partial event -- gives only the oob
  // item and the partial event remains untouched.
  
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
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
  
  m_pTestObj->addFragment(pFrag, 2);   // The in progress fragment.
  
  
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  pFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pFrag, 1);
  
  // we should find the scaler item in the file:  

  // The file should have the fragment's ring item;.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  uint8_t rdBuffer[2048];
  ssize_t rdSize = read(fd, rdBuffer, sizeof(rdBuffer));
  ASSERT(rdSize < sizeof(rdBuffer));
  EQ(ssize_t(pOriginalItem->s_header.s_size), rdSize);
  ASSERT(memcmp(pOriginalItem, rdBuffer, rdSize) == 0);
  
  // There should be a currente event with one fragment:
  
  ASSERT(m_pTestObj->m_pCurrentEvent);
  EQ(size_t(1), m_pTestObj->m_pCurrentEvent->s_eventInfo.s_nFragments);
}
void evaccTest::oob_3()
{
  // any buffered event gets flushed befgore 
  // the OOB event.
  
  // oob item when there's a partial event -- gives only the oob
  // item and the partial event remains untouched.
  
  // Put several events (identical) into the buffer.
  // Each event has a timestamp 1 tick larger than the prior.
  // all are from sid 5

  uint8_t buffer[1024];
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr->s_timestamp = 0x12345678;
  pHdr->s_sourceId  = 5;
  pHdr->s_barrier   = 0;
  pHdr->s_size        = sizeof(RingItemHeader) + sizeof(BodyHeader);
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
  
  m_pTestObj->addFragment(pFrag, 2);   // The in progress fragment.
  m_pTestObj->finishEvent();           // Fully buffered event now.
  
  // Now put in the oob item -- that should flush both events:
  
  uint8_t oobBuffer[1024];
  std::vector<uint32_t> scalers = {1,2,3,4,5,6,7,8,9};
  CRingScalerItem scaler(0x12345678, 1, 0, time(nullptr), 0, 10, scalers);
  pRingItem pOriginalItem = scaler.getItemPointer();
  
  // Make a flat fragment from this:
  
  EVB::pFragmentHeader pFHdr =
    reinterpret_cast<EVB::pFragmentHeader>(oobBuffer);
  pFHdr->s_timestamp = 0x1245678;
  pFHdr->s_sourceId  = 1;
  pFHdr->s_barrier   = 0;
  pFHdr->s_size      = pOriginalItem->s_header.s_size;
  memcpy(pFHdr+1, pOriginalItem, pOriginalItem->s_header.s_size);
  
  // Submit as out of band -- same sid.
  
  EVB::pFlatFragment pOOBFrag= reinterpret_cast<EVB::pFlatFragment>(pFHdr);
  m_pTestObj->addOOBFragment(pOOBFrag, 1);
  
  // the file should have both events.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  
  uint8_t rdbuffer[2048];
  ssize_t n = read(fd, rdbuffer, sizeof(rdbuffer));
  
  // First we'll see a PHYSI`CS ring item -- we'll assume it's
  // right if so.
  
  pRingItemHeader pReadItem = reinterpret_cast<pRingItemHeader>(rdbuffer);
  EQ(PHYSICS_EVENT, pReadItem->s_type);
  
  // Next should be a block of data that's identical to the ring scaler item:
  
  uint8_t* p = reinterpret_cast<uint8_t*>(pReadItem);
  p += pReadItem->s_size;
  pReadItem = reinterpret_cast<pRingItemHeader>(p);
  
  EQ(pOriginalItem->s_header.s_size, pReadItem->s_size);
  ASSERT(memcmp(pReadItem, pOriginalItem, pReadItem->s_size) == 0);
}
//
// Add a fragment that exactly fills,
// add another fragment (flushes).
// Flush manuall7.
// Ensure both are fine.

void
evaccTest::addfrag_8()
{
    uint8_t local[1020];              // For the whole buffer:
    EVB::pFragmentHeader pHdr =
      reinterpret_cast<EVB::pFragmentHeader>(local);
    pRingItemHeader pRHdr =
      reinterpret_cast<pRingItemHeader>(pHdr+1);
    pBodyHeader pBHdr =
      reinterpret_cast<pBodyHeader>(pRHdr+1);
    uint8_t* pBody =
      reinterpret_cast<uint8_t*>(pBHdr+1);
    
    // Compute the body size:
    
    size_t totalHeaderSize =
      sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader)
      + sizeof(BodyHeader);
    size_t totalBodySize = sizeof(local) - totalHeaderSize;
    
    // Fill in realistic shit:
    
    //     Fragment header:
    
    pHdr->s_timestamp = 1;
    pHdr->s_sourceId  = 1;
    pHdr->s_size     =
      totalBodySize + sizeof(BodyHeader) + sizeof(RingItemHeader);
    pHdr->s_barrier = 0;
    
    //    Ring item header:
    
    pRHdr->s_size = pHdr->s_size;
    pRHdr->s_type = PHYSICS_EVENT;
    
    //    Body Header -- no extension.
    
    pBHdr->s_size      = sizeof(BodyHeader);
    pBHdr->s_timestamp = pHdr->s_timestamp;
    pBHdr->s_sourceId  = pHdr->s_sourceId;
    pBHdr->s_barrier   = pHdr->s_barrier;
    
    //    Body are a counting pattern:
    
    for (int i =0; i < totalBodySize; i++) {
      pBody[i] = i;
    }
    // Submit the fragment:
    
    m_pTestObj->addFragment(
      reinterpret_cast<EVB::pFlatFragment>(pHdr), 1
    );
    m_pTestObj->finishEvent();       // Event is done.
    m_pTestObj->flushEvents();      // Force the slide.
    
    // Submit a second (smaller) fragment and flush.
    
    pHdr->s_timestamp =2;
    pHdr->s_sourceId = 2;
    pHdr->s_size     =
      sizeof(BodyHeader) + sizeof(RingItemHeader) + totalBodySize/2;
    pRHdr->s_size    = pHdr->s_size;
    pBHdr->s_timestamp = pHdr->s_timestamp;
    pBHdr->s_sourceId  = pHdr->s_sourceId;
    
    for (int i =0; i < totalBodySize/2; i++) {
      pBody[i] = 255-i;
    }
    m_pTestObj->addFragment(
      reinterpret_cast<EVB::pFlatFragment>(pHdr), 1
    );
    m_pTestObj->finishEvent();
    m_pTestObj->flushEvents();
    
    // The file should have two events with a single fragment each.
    // Let's look at it:
    
    URL url = makeUri();
    std::vector<uint16_t> dummy;
    CFileDataSource itemSource(url, dummy);
    CRingItem* pItem1 = itemSource.getItem();
    CRingItem* pItem2 = itemSource.getItem();
    
    // SHouldn't be any more items:
    
    CRingItem* pNoMore = itemSource.getItem();
    EQ(static_cast<CRingItem*>(nullptr), pNoMore);
    
    // Let's look at the items:
    //     First item
    //  The ring item should be the original size +
    //  an additional uint32_t, ring item and body headers.
    
    pRingItem pRawItem1= pItem1->getItemPointer();
    uint32_t sbSize =
      totalBodySize + sizeof(uint32_t) + 2*sizeof(RingItemHeader) +
        2*sizeof(BodyHeader) + sizeof(EVB::FragmentHeader);
        
    
    // Total ring item header and body header:
    
    EQ(sbSize, pRawItem1->s_header.s_size);
    EQ(PHYSICS_EVENT, pRawItem1->s_header.s_type);
    EQ(uint64_t(1), pRawItem1->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(1), pRawItem1->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pRawItem1->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    // Body size word in the body:
    
    uint32_t* pItem1Body =
      reinterpret_cast<uint32_t*>(pRawItem1->s_body.u_hasBodyHeader.s_body);
    uint32_t evbodSize =
      sizeof(uint32_t) + sizeof(EVB::FragmentHeader) +
      sizeof(RingItemHeader) + sizeof(BodyHeader)    + totalBodySize;
    EQ(evbodSize, *pItem1Body);

    // THere's one fragment right after thant:
    
    EVB::pFragmentHeader pFHdr =
      reinterpret_cast<EVB::pFragmentHeader>(pItem1Body+1);
    EQ(uint64_t(1), pFHdr->s_timestamp);
    EQ(uint32_t(1), pFHdr->s_sourceId);
    EQ(uint32_t(0), pFHdr->s_barrier);
    evbodSize =
      totalBodySize + sizeof(RingItemHeader) + sizeof(BodyHeader);
    EQ(evbodSize, pFHdr->s_size);
  
    pRingItemHeader pRHdr1 = reinterpret_cast<pRingItemHeader>(pFHdr+1);
    EQ(evbodSize, pRHdr1->s_size);
    EQ(PHYSICS_EVENT, pRHdr1->s_type);
    
    pBodyHeader pBHdr1 = reinterpret_cast<pBodyHeader>(pRHdr1+1);
    EQ(uint64_t(1), pBHdr1->s_timestamp);
    EQ(uint32_t(1), pBHdr1->s_sourceId);
    EQ(uint32_t(0), pBHdr1->s_barrier);
    uint8_t* pBody1 = reinterpret_cast<uint8_t*>(pBHdr1+1); //no ext.
    for (int i =0; i < totalBodySize; i++) {
      EQ(uint8_t(i), pBody1[i]);
    }
    
    //     Second item
    
    pRingItem pRawItem2
      = reinterpret_cast<pRingItem>(pItem2->getItemPointer());
    uint32_t item2Size = totalBodySize/2 + sizeof(EVB::FragmentHeader)
      + sizeof(uint32_t) + 2*sizeof(RingItemHeader) + 2*sizeof(BodyHeader);
    EQ(item2Size, pRawItem2->s_header.s_size);
    EQ(PHYSICS_EVENT, pRawItem2->s_header.s_type);
    EQ(uint64_t(2), pRawItem2->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(1), pRawItem2->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pRawItem2->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    uint32_t* pBSize2 = reinterpret_cast<uint32_t*>(
      pRawItem2->s_body.u_hasBodyHeader.s_body
    );
    item2Size = totalBodySize/2 + sizeof(uint32_t) +
      sizeof (EVB::FragmentHeader) + sizeof(RingItemHeader) +
      sizeof(BodyHeader);
    EQ(item2Size, *pBSize2);
    
    EVB::pFragmentHeader pFHdr2 =
      reinterpret_cast<EVB::pFragmentHeader>(pBSize2+1);
    EQ(uint64_t(2), pFHdr2->s_timestamp);
    EQ(uint32_t(2), pFHdr2->s_sourceId);
    EQ(uint32_t(0),  pFHdr2->s_barrier);
    item2Size = totalBodySize/2 + sizeof(RingItemHeader) +
      sizeof(BodyHeader);
    EQ(item2Size, pFHdr2->s_size);
    
    pRingItemHeader pRHdr2 =
      reinterpret_cast<pRingItemHeader>(pFHdr2+1);
    EQ(item2Size, pRHdr2->s_size);
    EQ(PHYSICS_EVENT, pRHdr2->s_type);
    pBodyHeader pBHdr2 = reinterpret_cast<pBodyHeader>(pRHdr2+1);
    EQ(uint64_t(2), pBHdr2->s_timestamp);
    EQ(uint32_t(2), pBHdr2->s_sourceId);
    EQ(uint32_t(0), pBHdr2->s_barrier);
    
    uint8_t* pB2 = reinterpret_cast<uint8_t*>(pBHdr2+1);
    for (int i = 0; i < totalBodySize/2; i++) {
      EQ(uint8_t(255-i), pB2[i]);
    }
    
    
    
    // Free the read items.
    
    delete pItem1;
    delete pItem2;
    
}
// Create a single fragment event.
// create a double fragment event but force a flush between
// the fragments.
//  This exercises the slideCurrentEventToFront.
//
void
evaccTest::addfrag_9()
{
  // First fragment
  
  uint8_t local[100];
  EVB::pFragmentHeader f1Fhdr
    = reinterpret_cast<EVB::pFragmentHeader>(local);
  pRingItemHeader f1Rhdr = reinterpret_cast<pRingItemHeader>(f1Fhdr + 1);
  pBodyHeader     f1Bhdr = reinterpret_cast<pBodyHeader>(f1Rhdr+1);
  uint8_t* pBody1 = reinterpret_cast<uint8_t*>(f1Bhdr + 1);
  uint32_t bodySize1 = sizeof(local) - sizeof(EVB::FragmentHeader) -
    sizeof(RingItemHeader) - sizeof(BodyHeader);
  
  //     Fill in fragment header.
  
  f1Fhdr->s_size = sizeof(local) - sizeof(EVB::FragmentHeader);
  f1Fhdr->s_timestamp = uint64_t(0x11111111);
  f1Fhdr->s_sourceId  = 12;
  f1Fhdr->s_barrier   = 0;
  
  //    Fill in the ring item header.
  
  f1Rhdr->s_size = f1Fhdr->s_size;
  f1Rhdr->s_type = PHYSICS_EVENT;
  
  //     Fill in the body header
  
  f1Bhdr->s_timestamp = f1Fhdr->s_timestamp;
  f1Bhdr->s_sourceId  = f1Fhdr->s_sourceId;
  f1Bhdr->s_barrier   = 0;
  f1Bhdr->s_size      = sizeof(BodyHeader);
  
  // Fill the body:
  
  for (int i =0; i < bodySize1; i++) {
    pBody1[i] = i;
  }
  m_pTestObj->addFragment(
    reinterpret_cast<EVB::pFlatFragment>(f1Fhdr), 1
  );
  
  
  // finish but don't flush the event.
  
  m_pTestObj->finishEvent();      // A one fragment event.
  
  // Second event first fragemtn
  
  //   We'll use the same sizes etc. but different body contents.
  //   Different timestamps and body header contents.
  
  f1Fhdr->s_timestamp = uint64_t(0x11111222);
  f1Fhdr->s_sourceId  = 10;
   
  f1Bhdr->s_timestamp = f1Fhdr->s_timestamp;
  f1Bhdr->s_sourceId  = f1Fhdr->s_sourceId;
  f1Bhdr->s_barrier   = 0;
  f1Bhdr->s_size      = sizeof(BodyHeader);
  
  for (int i =0; i <bodySize1; i++) {
    pBody1[i] = 255-i;
  }
  m_pTestObj->addFragment(
    reinterpret_cast<EVB::pFlatFragment>(f1Fhdr), 1
  );
  
  // flush events to force the slide.
  
  m_pTestObj->flushEvents();              // Forces a slide.
  
  // Second event second fragment
  
  f1Fhdr->s_timestamp = uint64_t(0x11111234);
  f1Fhdr->s_sourceId  = 12;
  
  f1Bhdr->s_timestamp = f1Fhdr->s_timestamp;
  f1Bhdr->s_sourceId  = f1Fhdr->s_sourceId;
  f1Bhdr->s_barrier   = 0;
  f1Bhdr->s_size      = sizeof(BodyHeader);
  
  for (int i =0; i <bodySize1; i++) {
    pBody1[i] = i*2;
  }
  m_pTestObj->addFragment(
    reinterpret_cast<EVB::pFlatFragment>(f1Fhdr), 1
  );
  
  
  // finish evend and flush fragments.
  
  m_pTestObj->finishEvent();
  m_pTestObj->flushEvents();    // Put it all out to file.
  
  // Check the events in the output file.
  
  URL uri = makeUri();
  std::vector<uint16_t> dummy;
  CFileDataSource itemSource(uri, dummy);
  CRingItem* pItem1 = itemSource.getItem(); // One fragment.
  CRingItem* pItem2 = itemSource.getItem(); // Two fragments.
  
  // SHouldn't be any more items:
  
  CRingItem* pNoMore = itemSource.getItem();
  EQ(static_cast<CRingItem*>(nullptr), pNoMore);
  
  // First event has one fragment painstakingly check everything:
  
  uint32_t evSize1 =
    sizeof(local) + sizeof(uint32_t) + sizeof(RingItemHeader) +
      sizeof(BodyHeader);
  pRingItem pRaw1 = pItem1->getItemPointer();
  EQ(evSize1, pRaw1->s_header.s_size);
  EQ(PHYSICS_EVENT, pRaw1->s_header.s_type);
  EQ(uint64_t(0x11111111), pRaw1->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
  EQ(uint32_t(1), pRaw1->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
  EQ(uint32_t(0), pRaw1->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);

  //   There's a size long and  one fragment.
  
  uint32_t* pSize = reinterpret_cast<uint32_t*>(
    pRaw1->s_body.u_hasBodyHeader.s_body
  );
  evSize1 = sizeof(local) + sizeof(uint32_t);
  EQ(evSize1, *pSize);
  
  EVB::pFragmentHeader pFh1 =
    reinterpret_cast<EVB::pFragmentHeader>(pSize+1);
  EQ(uint64_t(0x11111111), pFh1->s_timestamp);
  EQ(uint32_t(12), pFh1->s_sourceId);
  EQ(uint32_t(0), pFh1->s_barrier);
  evSize1 = sizeof(local) - sizeof(EVB::FragmentHeader);
  EQ(evSize1, pFh1->s_size);
  
  pRingItemHeader pH1 = reinterpret_cast<pRingItemHeader>(pFh1+1);
  EQ(evSize1, pH1->s_size);
  EQ(PHYSICS_EVENT, pH1->s_type);
  
  pBodyHeader pB1 = reinterpret_cast<pBodyHeader>(pH1+1);
  EQ(uint64_t(0x11111111), pB1->s_timestamp);
  EQ(uint32_t(12), pB1->s_sourceId);
  EQ(uint32_t(0), pB1->s_barrier);
  
  uint8_t* p  = reinterpret_cast<uint8_t*>(pB1+1);
  for (int i =0; i < bodySize1; i++) {
    EQ(uint8_t(i), p[i]);
  }
  
  // Second event has a pair of Fragments.
  
  pRingItem pRaw2 = pItem2->getItemPointer();
  uint32_t evSize2 =
    sizeof(local)*2 + sizeof(uint32_t) +
    sizeof(RingItemHeader) + sizeof(BodyHeader);
  EQ(evSize2, pRaw2->s_header.s_size);
  EQ(PHYSICS_EVENT, pRaw2->s_header.s_type);
 
  EQ(uint64_t(0x11111234), pRaw2->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
  EQ(uint32_t(1), pRaw2->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
  EQ(uint32_t(0), pRaw2->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
  EQ(uint32_t(sizeof(BodyHeader)), pRaw2->s_body.u_hasBodyHeader.s_bodyHeader.s_size);
  
  evSize2 = sizeof(local)*2 + sizeof(uint32_t);
  pSize   = reinterpret_cast<uint32_t*>(pRaw2->s_body.u_hasBodyHeader.s_body);
  EQ(evSize2, *pSize);
  
  //     First fragment:
  
  EVB::pFragmentHeader pFrag1 = reinterpret_cast<EVB::pFragmentHeader>(pSize+1);
  EQ(uint64_t(0x11111222), pFrag1->s_timestamp);
  EQ(uint32_t(10),         pFrag1->s_sourceId);
  EQ(uint32_t(0),          pFrag1->s_barrier);
  uint32_t fragSize = sizeof(local) - sizeof(EVB::FragmentHeader);
  EQ(fragSize, pFrag1->s_size);
    
  pRingItemHeader pRf1 = reinterpret_cast<pRingItemHeader>(pFrag1+1);
  EQ(PHYSICS_EVENT, pRf1->s_type);
  EQ(fragSize, pRf1->s_size);
  
  pBodyHeader pBf1 = reinterpret_cast<pBodyHeader>(pRf1+1);
  EQ(pFrag1->s_timestamp, pBf1->s_timestamp);
  EQ(pFrag1->s_sourceId,  pBf1->s_sourceId);
  EQ(pFrag1->s_barrier, pBf1->s_barrier);
  EQ(uint32_t(sizeof(BodyHeader)), pBf1->s_size);
  uint8_t* pf1 = reinterpret_cast<uint8_t*>(pBf1+1);
  fragSize -= (sizeof(RingItemHeader) + sizeof(BodyHeader)); // body payload size
  for (int i =0; i < fragSize; i++) {
    EQ(uint8_t(255-i), pf1[i]);
  }
  
  //   Second fragment
  
  EVB::pFragmentHeader pFrag2 = reinterpret_cast<EVB::pFragmentHeader>(
    pf1 + fragSize
  );
  EQ(uint64_t(0x11111234), pFrag2->s_timestamp);
  EQ(uint32_t(12), pFrag2->s_sourceId);
  EQ(uint32_t(0), pFrag2->s_barrier);
  fragSize = sizeof(local) - sizeof(EVB::FragmentHeader);
  EQ(fragSize, pFrag2->s_size);
  
  pRingItemHeader pRf2 = reinterpret_cast<pRingItemHeader>(pFrag2 + 1);
  EQ(pFrag2->s_size, pRf2->s_size);
  EQ(PHYSICS_EVENT, pRf2->s_type);
  pBodyHeader pBf2 = reinterpret_cast<pBodyHeader>(pRf2+1);
  EQ(pFrag2->s_timestamp, pBf2->s_timestamp);
  EQ(pFrag2->s_sourceId,  pBf2->s_sourceId);
  EQ(pFrag2->s_barrier,   pBf2->s_barrier);
  EQ(uint32_t(sizeof(BodyHeader)), pBf2->s_size);
  
  uint8_t* pf2 = reinterpret_cast<uint8_t*>(pBf2+1);
  fragSize -= (sizeof(RingItemHeader) + sizeof(BodyHeader));
  for (int i =0; i < fragSize; i++) {
    EQ(uint8_t(i*2), pf2[i]);
  }
  
  
  // Release the ring items that getItem created.
  delete pItem1;
  delete pItem2;
        
}
void
evaccTest::addfrag_10()
{
  // Randomly sized fragments (total max 100 uint16_t words.
  // Random number of fragments/event (max fragments 5).
  // Each fragment will have random contents.
  // Each fragment will have a random sourceid from 1-5.
  // timestamps will increment by a random amount from 0-500.
  // We'll keep track of all that information in vectors for the
  // comparison later.
  // We'll make 100 events.
  // Note we don't seed the randomizer so we should get
  // reproducible (from run to run) events.
  // An event is characterized by the following information:
  
  struct Event {
    uint32_t s_bodySize;
    size_t   s_nFrags;
    std::vector<EVB::FragmentHeader>        s_fragmentHeaders;
    std::vector<RingItemHeader>        s_ringItemHeaders;
    std::vector<BodyHeader>            s_bodyHeaders;
    std::vector<std::vector<uint16_t>> s_payloads;
  };
  
  uint64_t  stamp = 0;
  std::vector<Event> events;
  int       numEvents = 100;
  for (int evt =0; evt < numEvents; evt++) {
    int nFrags = randint(1, 6);
    Event event;
    event.s_nFrags = nFrags;
    event.s_bodySize = sizeof(uint32_t);           // Leading size.
    for (int frag = 0; frag < nFrags; frag++) {
      int nBodySize= randint(1, 100);    // bytes.
      nBodySize = (nBodySize/sizeof(uint16_t) ) * sizeof(uint16_t); // full words.
      stamp += randint(0, 500);
      uint32_t sid = randint(0, 5);
      
      // Storage needed for the fragment is the size of the payload,
      // with a body header, ring item header and a fragment header added on
      
      size_t totalStorage = nBodySize + sizeof(EVB::FragmentHeader) +
        sizeof(RingItemHeader) + sizeof(BodyHeader);
      uint8_t storage[totalStorage];
      
      uint32_t eventSize = nBodySize + sizeof(BodyHeader) + sizeof(RingItemHeader);
      event.s_bodySize += totalStorage;
      
      // Let's fill in the fragment header:
      
      EVB::pFragmentHeader pFh = reinterpret_cast<EVB::pFragmentHeader>(storage);
      pFh->s_timestamp = stamp;
      pFh->s_sourceId  = sid;
      pFh->s_barrier   = 0;
      pFh->s_size      = eventSize;
      event.s_fragmentHeaders.push_back(*pFh);
      
      // Now the ring item header:
      
      pRingItemHeader pRh = reinterpret_cast<pRingItemHeader>(pFh+1);
      pRh->s_size = eventSize;
      pRh->s_type = PHYSICS_EVENT;
      event.s_ringItemHeaders.push_back(*pRh);
      
      // The body header:
      
      pBodyHeader pBh  = reinterpret_cast<pBodyHeader>(pRh+1);
      pBh->s_size = sizeof(BodyHeader);
      pBh->s_timestamp = stamp;
      pBh->s_sourceId  = sid;
      pBh->s_barrier   = 0;
      event.s_bodyHeaders.push_back(*pBh);
      
      // The payload:
      
      uint16_t* pBody = reinterpret_cast<uint16_t*>(pBh+1);
      event.s_payloads.push_back(randomBody(pBody, nBodySize/sizeof(uint16_t)));
      
      // add the fragment to the current event:
      
      m_pTestObj->addFragment(
        reinterpret_cast<EVB::pFlatFragment>(pFh), 100  // output sid <- 100.  
      );
      
    }
    m_pTestObj->finishEvent();
    events.push_back(event);          // Save for comparison
  }
  m_pTestObj->flushEvents();          // Finish writing the output file.

  // Now Compare the data with the events we stored in the
  // events  vector of structs.
  
  URL uri = makeUri();
  std::vector<uint16_t> dummy;
  CFileDataSource src(uri, dummy);
  
  for (int evt = 0; evt < 100; evt++) {
    CRingItem* pItem = src.getItem();
    pRingItem pRawItem = pItem->getItemPointer();
    Event& e = events[evt];
    uint32_t totalEventSize = e.s_bodySize + sizeof(RingItemHeader) +
      sizeof(BodyHeader);
    EQ(totalEventSize, pRawItem->s_header.s_size);
    EQ(PHYSICS_EVENT, pRawItem->s_header.s_type);
    
    // Timestamp comes from the last item:
    
    uint64_t estamp = e.s_fragmentHeaders.back().s_timestamp;
    EQ(estamp, pRawItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(100), pRawItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(uint32_t(0), pRawItem->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    EQ(
       uint32_t(sizeof(BodyHeader)),
       pRawItem->s_body.u_hasBodyHeader.s_bodyHeader.s_size
    );
    uint32_t* pbSize = reinterpret_cast<uint32_t*>(
      pRawItem->s_body.u_hasBodyHeader.s_body
    );
    EQ(e.s_bodySize, *pbSize);
    
    EVB::pFragmentHeader pFrag =
      reinterpret_cast<EVB::pFragmentHeader>(pbSize+1);
    uint32_t bytesLeft = *pbSize - sizeof(uint32_t);
    for (int f =0; f < e.s_nFrags; f++) {
      ASSERT(bytesLeft >= sizeof(EVB::FragmentHeader)); // space for headr.
      // Check the header then check that there's enough space
      // for the full fragment:
      
      EQ(e.s_fragmentHeaders[f].s_timestamp, pFrag->s_timestamp);
      EQ(e.s_fragmentHeaders[f].s_sourceId,  pFrag->s_sourceId);
      EQ(e.s_fragmentHeaders[f].s_barrier,   pFrag->s_barrier);
      EQ(e.s_fragmentHeaders[f].s_size,     pFrag->s_size);
      
      ASSERT(
        bytesLeft >=
        (sizeof(EVB::FragmentHeader) + e.s_fragmentHeaders[f].s_size)
      );
      // Check Ring item header contents.
      
      pRingItemHeader pRh = reinterpret_cast<pRingItemHeader>(pFrag+1);
      EQ(e.s_ringItemHeaders[f].s_size, pRh->s_size);
      EQ(e.s_ringItemHeaders[f].s_type, pRh->s_type);
      
      // Body header check:
      
      pBodyHeader pBh = reinterpret_cast<pBodyHeader>(pRh+1);
      EQ(e.s_bodyHeaders[f].s_size, pBh->s_size);
      EQ(e.s_bodyHeaders[f].s_timestamp, pBh->s_timestamp);
      EQ(e.s_bodyHeaders[f].s_sourceId,  pBh->s_sourceId);
      EQ(e.s_bodyHeaders[f].s_barrier,   pBh->s_barrier);
      
      uint16_t* payload = reinterpret_cast<uint16_t*>(pBh+1);
      std::vector<uint16_t>& epayload(e.s_payloads[f]);
      for (int i =0; i < epayload.size(); i++) {
        EQ(epayload[i], payload[i]);
      }
      // Next fragment:
      
      pFrag =reinterpret_cast<EVB::pFragmentHeader>(
        payload + epayload.size()
      );
      
    }
    delete pItem;
  }
  CRingItem* pItem = src.getItem();
  EQ((CRingItem*)(nullptr), pItem);
  
}