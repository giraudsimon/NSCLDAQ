// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "ModuleReader.h"
#define private public
#include "CHitManager.h"
#undef private

#include "ZeroCopyHit.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"


#include "Asserts.h"
#include <stdint.h>
#include <stdexcept>
#include <algorithm>
#include <stdlib.h>

#include <iostream>

class HMTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HMTest);
  CPPUNIT_TEST(construct);
  
  // These tests all involve a single call to addHits.
  
  CPPUNIT_TEST(empty_1);
  CPPUNIT_TEST(empty_2);
  
  CPPUNIT_TEST(onehit_1);
  CPPUNIT_TEST(onehit_2);
  
  CPPUNIT_TEST(onedeque_1);
  CPPUNIT_TEST(onedeque_2);
  CPPUNIT_TEST(onedeque_3);
  
  CPPUNIT_TEST(twodeque_1);
  CPPUNIT_TEST(twodeque_2);
  CPPUNIT_TEST(twodeque_3);
  
  CPPUNIT_TEST(randoms);
  
  // These tests check the ability to merge hits across more than one
  // addHits call.
  
  CPPUNIT_TEST(twoadds_1);
  CPPUNIT_TEST(twoadds_2);
  CPPUNIT_TEST(twoadds_3);
  
  CPPUNIT_TEST(randommulti);
  CPPUNIT_TEST_SUITE_END();


private:
  DDASReadout::CHitManager*  m_pTestObj;
  DDASReadout::BufferArena*  m_pBuffers;
public:
  void setUp() {
    m_pBuffers = new DDASReadout::BufferArena;
    m_pTestObj = new DDASReadout::CHitManager(1.0);
  }
  void tearDown() {
    try {             // Empty nd delete the list:
      while (1) {
        delete m_pTestObj->getHit().second;
      }
      
    } catch(std::logic_error) {}
    delete m_pTestObj;
    m_pTestObj = nullptr;
    
    delete m_pBuffers;
    m_pBuffers = nullptr;
  }
protected:
  void construct();
  
  void empty_1();
  void empty_2();
  
  void onehit_1();
  void onehit_2();
  
  void onedeque_1();
  void onedeque_2();
  void onedeque_3();
  
  void twodeque_1();
  void twodeque_2();
  void twodeque_3();
  
  void randoms();
  
  void twoadds_1();
  void twoadds_2();
  void twoadds_3();
  
  void randommulti();
private:
  void* MakeHit(
    DDASReadout::ZeroCopyHit& hit, DDASReadout::BufferArena& arena,
    DDASReadout::ReferenceCountedBuffer& buf, void* pWhere, 
    uint64_t tstamp,  int c, int s, int ch
  );
  
  void* MakeHitDeque(
    DDASReadout::ModuleReader::HitList& hits, int numHits,
    uint64_t* tstamps, int c, int s, int ch
  );
  void freeHits(DDASReadout::ModuleReader::HitList& hits);
  void freeHits(std::vector<DDASReadout::ModuleReader::HitList>& hits);
};
///////////////////////////////////////////////////////////////////////////////
// Utilities.

void
HMTest::freeHits(DDASReadout::ModuleReader::HitList& hits)
{
  while(!hits.empty()) {
    delete hits.front().second;
    hits.pop_front();
  }
}
void HMTest::freeHits(std::vector<DDASReadout::ModuleReader::HitList>& hits)
{
  for (int i =0; i < hits.size(); i++) {
    freeHits(hits[i]);
  }
  hits.clear();
}

/**
 *  Create a zero copy hit:
 *  @param hit - references the hit that will be pointed to it.
 *  @param arena - buffer allocation arena the data buffer to hold the hit comes from.
 *  @param buf   - Reference counted buffer in whih the hit will be put.
 *  @param pWhere - Pointer to where the hit will be put.
 *  @param tstamp - timestamp to assign the hit.
 *  @param c,s,ch - Crate, slot, channel for the hit.
 *  @param All other fields will be zeroes for this test.
 *  @return void* - pointer to the next free byte of the buffer.
 *  @note - the buffer will not be range checked.
 *  @note - All hits will just be a header.
 *  
 */
void*
HMTest::MakeHit(
  DDASReadout::ZeroCopyHit& hit, DDASReadout::BufferArena& arena,
  DDASReadout::ReferenceCountedBuffer& buf, void* pWhere, 
  uint64_t tstamp,  int c, int s, int ch
)
{
  uint32_t* pWords = static_cast<uint32_t*>(pWhere);
  *pWords++ = (4 << 17) | (4 << 12) | (c << 8) | (s << 4) | ch;
  *pWords++ = tstamp & 0xffffffff;
  *pWords++ = (tstamp >> 32) & 0xffff;
  *pWords++ = 0;
  
  hit.setHit(4, pWhere, &buf, &arena);
  hit.SetTime();
  hit.SetLength();
  hit.SetChannel();
  
  return pWords;
}
/**
 *  Make a dequeue of hits with timestamps given..   Most of the
 *  parameters are the same as Makehit...except that tstamps (plural)
 *  points to an array of timestamps, and the hits are put in a deque.
 *  The HitInfo's are created with nullptr for the module reader
 *  so be careful about not letting the hitmanager delete them.
 */
void*
HMTest::MakeHitDeque(
    DDASReadout::ModuleReader::HitList& hits, int numHits,
    uint64_t* tstamps, int c, int  s, int ch
)
{
  DDASReadout::ReferenceCountedBuffer* pBuf =
    m_pBuffers->allocate(numHits*10*sizeof(uint32_t));  // Too big but that's ok.
  
  void* p = pBuf->s_pData;
  for (int i =0; i < numHits; i++) {
    DDASReadout::ModuleReader::HitInfo info = {
      static_cast<DDASReadout::ModuleReader*>(nullptr),
      new DDASReadout::ZeroCopyHit
    };
    p = MakeHit(
      *info.second, *m_pBuffers, *pBuf, p, tstamps[i], 0, 0, 0
    );
    hits.push_back(info);  
  }
  
  return p;
}

// for tstamp sorting:

int tstampCompare(const void* e1, const void* e2)
{
  const uint64_t* t1 = static_cast<const uint64_t*>(e1);
  const uint64_t* t2 = static_cast<const uint64_t*>(e2);
  
  
  if (*t1 < *t2) {
    return -1;
  } else if (*t1 == *t2) {
    return 0;
  } else  {
    return 1;
  }
}

CPPUNIT_TEST_SUITE_REGISTRATION(HMTest);

void HMTest::construct() {
  EQ(1.0e9, m_pTestObj->m_emitWindow);
  ASSERT(m_pTestObj->m_sortedHits.empty());
  ASSERT(!m_pTestObj->m_flushing);
}

// empty always says there's no hits:
void HMTest::empty_1()
{
  ASSERT(!m_pTestObj->haveHit());
}

// IF empty getting a hit throws std::logic_error
void HMTest::empty_2()
{
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
}

// If I pass a single hit:
// - If not flushing haveHit is false and
// - Getting a hit throws should get the original hit.

void HMTest::onehit_1()
{
  uint64_t tstamp = 0;
  DDASReadout::ModuleReader::HitList hits;
  MakeHitDeque(hits, 1, &tstamp, 0, 1, 2);
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);
  
  m_pTestObj->addHits(hitvec);
  
  ASSERT(!m_pTestObj->haveHit());
  DDASReadout::ModuleReader::HitInfo ahit;
  CPPUNIT_ASSERT_NO_THROW(
    ahit = m_pTestObj->getHit()
  );
  
  EQ(hits[0].first, ahit.first);            // Same hit we put in.
  EQ(hits[0].second, ahit.second);
  
  /// Have to do some cleanup:
  
  delete hits[0].second;                 // Also dereferenes
  
}
// If I pass a single hit and go into flush mode, I shoulid get a
//true from haveHit and getHit won't throw.

void HMTest::onehit_2()
{
  uint64_t tstamps = 1;
  DDASReadout::ModuleReader::HitList hits;
  MakeHitDeque(hits, 1, &tstamps, 0, 1, 2);
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);
  m_pTestObj->addHits(hitvec);
  
  m_pTestObj->flushing(true);
  ASSERT(m_pTestObj->haveHit());
  CPPUNIT_ASSERT_NO_THROW(
    m_pTestObj->getHit()
  );
  
  delete hits[0].second;
  
}
// Single dequeue with the data already sorted... but timestamps all in the
// window.

void HMTest::onedeque_1()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);
  
  uint64_t tstamps[] = {1,2,3,4,5,6,7,8,9,10};  // 10 hits.
  MakeHitDeque(hitvec[0], 10, tstamps, 0, 2, 4);
  m_pTestObj->addHits(hitvec);
  
  ASSERT(!m_pTestObj->haveHit());
  
  // But I can take 10 hits out and they'll have the right timestamp:
  
  for(int i =0; i < 10; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(tstamps[i]), hit.second->s_time);
    delete hit.second;
  }
  // Should  not be any more hits:
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
  
}
// Single deque with timestamps not sorted (reversed to make it easy).
// not enough spread in timestamp to be ok.   hits should come out sorted.

void HMTest::onedeque_2()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);
  
  uint64_t tstamps[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};  // 10 hits.
  MakeHitDeque(hitvec[0], 10, tstamps, 0, 2, 4);
  m_pTestObj->addHits(hitvec);
  
  
  ASSERT(!m_pTestObj->haveHit());
  
  // But I can take 10 hits out and they'll have the right timestamp:
  
  // sort my timestamps:
  
  qsort(tstamps, 10, sizeof(uint64_t), tstampCompare);
  
  for(int i =0; i < 10; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(tstamps[i]), hit.second->s_time);
    delete hit.second;
  }
  // Should  not be any more hits:
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );  
}
//  Unsorted with enough spread to give us one hit available on the basis
//  of the time window.

void HMTest::onedeque_3()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);
  
  uint64_t tstamps[] = {1000000002, 9, 8, 7, 6, 5, 4, 3, 2, 1};  // 10 hits.
  //////////////////         ^ should be releasable.
  MakeHitDeque(hitvec[0], 10, tstamps, 0, 2, 4);
  m_pTestObj->addHits(hitvec);
  
  
  ASSERT(m_pTestObj->haveHit());
  
  // But I can take 10 hits out and they'll have the right timestamp:
  
  // sort my timestamps:
  
  qsort(tstamps, 10, sizeof(uint64_t), tstampCompare);
  
  for(int i =0; i < 10; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(tstamps[i]), hit.second->s_time);
    delete hit.second;
    // NO more releasable hits since the difference is now exactly the
    // emit window and we require > than the window.
    
    ASSERT(!m_pTestObj->haveHit());
  }
  // Should  not be any more hits:
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );    
}
// Two deques -- must merge but can't emit.

void HMTest::twodeque_1()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);             // [0]
  hitvec.push_back(hits);             // [1]
  
  uint64_t tstamp0[] =  {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
  uint64_t tstamp1[] =  {19, 17, 15, 13, 11, 9, 7, 5, 3, 1};
  
  MakeHitDeque(hitvec[0], 10, tstamp0, 0, 2, 4);
  MakeHitDeque(hitvec[1], 10, tstamp1, 1, 3, 5);
  
  m_pTestObj->addHits(hitvec);
  
  std::vector<uint64_t> allstamps;
  for (int i =0; i < 10; i++) {
    allstamps.push_back(tstamp0[i]);
    allstamps.push_back(tstamp1[i]);
  }
  std::sort(allstamps.begin(), allstamps.end());  // merged sorted stamps.
  
  // the timespread isn't big enough to have a hit:
  
  ASSERT(!m_pTestObj->haveHit());
  
  for (int i =0; i < 20; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(allstamps[i]), hit.second->s_time);
    delete hit.second;
    ASSERT(!m_pTestObj->haveHit());
  }
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
  
}
// Two deques - one of the events has a ts big enough to make a
// releasable hit:

void HMTest::twodeque_2()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);             // [0]
  hitvec.push_back(hits);             // [1]
  
  uint64_t tstamp0[] =  {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
  uint64_t tstamp1[] =  {1000000001, 17, 15, 13, 11, 9, 7, 5, 3, 1};
  
  MakeHitDeque(hitvec[0], 10, tstamp0, 0, 2, 4);
  MakeHitDeque(hitvec[1], 10, tstamp1, 1, 3, 5);
  
  m_pTestObj->addHits(hitvec);
  
  std::vector<uint64_t> allstamps;
  for (int i =0; i < 10; i++) {
    allstamps.push_back(tstamp0[i]);
    allstamps.push_back(tstamp1[i]);
  }
  std::sort(allstamps.begin(), allstamps.end());  // merged sorted stamps.
  
  // the timespread isn't big enough to have a hit:
  
  ASSERT(m_pTestObj->haveHit());
  
  for (int i =0; i < 20; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(allstamps[i]), hit.second->s_time);
    delete hit.second;
    ASSERT(!m_pTestObj->haveHit());
  }
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );  
}
// Tow deques but differing number of elements.

void HMTest::twodeque_3()
{
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  hitvec.push_back(hits);             // [0]
  hitvec.push_back(hits);             // [1]
  
  uint64_t tstamp0[] =  {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
  uint64_t tstamp1[] =  {1000000001, 17, 15, 13, 11, };
  
  MakeHitDeque(hitvec[0], 10, tstamp0, 0, 2, 4);
  MakeHitDeque(hitvec[1], 5, tstamp1, 1, 3, 5);
  
  m_pTestObj->addHits(hitvec);
  
  std::vector<uint64_t> allstamps;
  for (int i =0; i < 10; i++) {
    allstamps.push_back(tstamp0[i]);
  }
  for (int i =0; i < 5; i++) {
    allstamps.push_back(tstamp1[i]);
  }
  std::sort(allstamps.begin(), allstamps.end());  // merged sorted stamps.
  
  // the timespread isn't big enough to have a hit:
  
  ASSERT(m_pTestObj->haveHit());
  
  for (int i =0; i < 15; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(allstamps[i]), hit.second->s_time);
    delete hit.second;
    ASSERT(!m_pTestObj->haveHit());
  }
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );    
}
// Random number of queues 1-10-ish.
// each has a random number of elements [0-100)
// each has a random timestamp (0 - 1.0e10).

void HMTest::randoms()
{
  // Figure out how many queues and populate the  vector
  
  int nQueues = 1 + drand48()*10;          // At least one.
  DDASReadout::ModuleReader::HitList hits;
  std::vector<DDASReadout::ModuleReader::HitList> hitvec;
  for (int i  =0; i < nQueues; i++) {
    hitvec.push_back(hits);
  }
  // Populate each dequeue:
  
  int totalHits = 0;
  for(int i =0; i < nQueues; i++) {
    int nHits = 100*drand48();      // 0 - 100 hits.
    totalHits += nHits;             // Keep track of how many  hits we have.
    if (nHits > 0) {
      uint64_t ts[nHits];
      for (int h = 0; h < nHits; h++) {
        ts[h] = 1.0e10*drand48(); 
      }
      MakeHitDeque(hitvec[i], nHits, ts, 0, 1, i);
    }
  }
  m_pTestObj->addHits(hitvec);     // Add all those hits at once.
  
  // We just want to be sure the hits are in ascending order:
  
  double lastTs = 0.0;
  for (int i =0; i < totalHits; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    double nextTime = hit.second->s_time;
    ASSERT(nextTime >= lastTs);
    lastTs = nextTime;
    delete hit.second;
    
  }
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
}
// Single dequeue for both adds nonoverlapping timestamp range -- should just
// append.

void HMTest::twoadds_1()
{
  DDASReadout::ModuleReader::HitList empty;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec1;
  hitvec1.push_back(empty);
  uint64_t ts1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  MakeHitDeque(hitvec1[0], 10, ts1, 0, 1, 1);
  
  m_pTestObj->addHits(hitvec1);
  
  // Now the second set of hits;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec2;
  hitvec2.push_back(empty);
  uint64_t ts2[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  MakeHitDeque(hitvec2[0], 10, ts2, 0, 1, 2);
  m_pTestObj->addHits(hitvec2);
  
  // should come out in nice order -- all 20 of them:
  
  for (int i =0; i < 20; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(i+1), hit.second->s_time);
    delete hit.second;
  }
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
}
// Second add should prepend the first:


void HMTest::twoadds_2()
{
  DDASReadout::ModuleReader::HitList empty;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec1;
  hitvec1.push_back(empty);
  uint64_t ts1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  MakeHitDeque(hitvec1[0], 10, ts1, 0, 1, 1);
  
  
  // Now the second set of hits;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec2;
  hitvec2.push_back(empty);
  uint64_t ts2[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  MakeHitDeque(hitvec2[0], 10, ts2, 0, 1, 2);

  m_pTestObj->addHits(hitvec2);
  m_pTestObj->addHits(hitvec1);
  
  // should come out in nice order -- all 20 of them:
  
  for (int i =0; i < 20; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(i+1), hit.second->s_time);
    delete hit.second;
  }
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
}
// partially overlapping rangs:

void HMTest::twoadds_3()
{
  DDASReadout::ModuleReader::HitList empty;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec1;
  hitvec1.push_back(empty);
  uint64_t ts1[] = {1, 2, 3, 4, 5, 6, 11, 12, 13, 14};
  MakeHitDeque(hitvec1[0], 10, ts1, 0, 1, 1);
  
  m_pTestObj->addHits(hitvec1);
  
  // Now the second set of hits;
  
  std::vector<DDASReadout::ModuleReader::HitList> hitvec2;
  hitvec2.push_back(empty);
  uint64_t ts2[] = {7, 8, 9, 10, 15, 16, 17, 18, 19, 20};
  MakeHitDeque(hitvec2[0], 10, ts2, 0, 1, 2);
  m_pTestObj->addHits(hitvec2);
  
  // should come out in nice order -- all 20 of them:
  
  for (int i =0; i < 20; i++) {
    DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
    EQ(double(i+1), hit.second->s_time);
    delete hit.second;
  }
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );  
}

/// Several adds of a random number of deques with random sizes and
//  timestamps that roughly increase but not strictly.
//  between 10 and 25 inserts:.
//  up to 10 hit lists.
//  Up to 20 hits per list.
//  Timestamps are from (base = insertnum*100) to base  + 500.

void HMTest::randommulti()
{
  DDASReadout::ModuleReader::HitList empty;
  unsigned addCount = 10 + 20*drand48();         //Number of adds.
  // Do the inserts:
  
  std::vector<uint64_t> allTimestamps;
  for (int ins = 0; ins < addCount; ins++) {
    std::vector<DDASReadout::ModuleReader::HitList> hitvec;
    unsigned queues = 1 + 10*drand48();    // At least 1 queue.
    int ele =0;
    for (int q =0; q < queues; q++) {
      unsigned hits = 20*drand48();
      if (hits > 0) {
        hitvec.push_back(empty);
        uint64_t tstamps[hits];
        for (int h = 0;  h < hits; h++) {
          tstamps[h] = ins*100 + 500*drand48();
          allTimestamps.push_back(tstamps[h]);
        }
        MakeHitDeque(hitvec[ele], hits, tstamps, 0, 1, q);
        ele++;
      }
      
    }
    
    m_pTestObj->addHits(hitvec);
  }
  // All the hits have been added, allTimestamps is the set of timestamps
  // and its size is the number of hits. Sort it:
  
  std::sort(allTimestamps.begin(), allTimestamps.end());
  
  // The hits should come out in the order of the allTimestamps vector:

  for (int i =0; i < allTimestamps.size(); i++) {
    
      DDASReadout::ModuleReader::HitInfo hit = m_pTestObj->getHit();
      EQ(double(allTimestamps[i]), hit.second->s_time);
      delete hit.second;
  }
  // should be no more hits:
  
  CPPUNIT_ASSERT_THROW(
    m_pTestObj->getHit(),
    std::logic_error
  );
}
