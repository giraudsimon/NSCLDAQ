// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#include "HitManager.h"
#undef private

#include "ReferenceCountedBuffer.h"
#include "BufferArena.h"
#include "ZeroCopyHit.h"

#include <deque>
#include <iostream>

std::ostream& operator<<(std::ostream& o, const std::deque<DDASReadout::ZeroCopyHit*>& hits)
{
  for (int i = 0; i < hits.size(); i++) {
    o << hits[i]->s_time << std::endl;
  }
  return o;

}

class hitmgrtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(hitmgrtest);
  CPPUNIT_TEST(initial_1);
  CPPUNIT_TEST(initial_2);
  
  CPPUNIT_TEST(sort_1);
  CPPUNIT_TEST(sort_2);
  
  CPPUNIT_TEST(merge_1);
  CPPUNIT_TEST(merge_2);
  CPPUNIT_TEST(merge_3);
  CPPUNIT_TEST(merge_4);
  
  CPPUNIT_TEST(add_1);
  CPPUNIT_TEST(add_2);
  CPPUNIT_TEST(add_3);
  CPPUNIT_TEST(add_4);
  
  CPPUNIT_TEST(havehit_1);
  CPPUNIT_TEST(havehit_2);
  CPPUNIT_TEST(havehit_3);
  
  CPPUNIT_TEST(nexthit_1);
  CPPUNIT_TEST_SUITE_END();


private:
  HitManager* m_pTestObject;
  DDASReadout::BufferArena* m_pArena;
public:
  void setUp() {
    m_pTestObject = new HitManager(10*1.0e9);
    m_pArena      = new DDASReadout::BufferArena;
  }
  void tearDown() {
    delete m_pTestObject;
    delete m_pArena;
  }
protected:
  void initial_1();
  void initial_2();
  
  void sort_1();
  void sort_2();
  
  void merge_1();
  void merge_2();
  void merge_3();
  void merge_4();
  
  void add_1();
  void add_2();
  void add_3();
  void add_4();
  
  void havehit_1();
  void havehit_2();
  void havehit_3();
  
  void nexthit_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(hitmgrtest);

void hitmgrtest::initial_1() {        // initially haveHit -> false.
  ASSERT(!m_pTestObject->haveHit());
}

void hitmgrtest::initial_2()         // initially nextHit -> nullptr.
{
  ASSERT(!m_pTestObject->nextHit());
}

void hitmgrtest::sort_1()
{
  // A couple of hits get sorted in time order:
  
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  
  m_pTestObject->sortHits(hits);
  
  // hits get timestamp ordered.
  
  EQ(double(1111), hits[0]->s_time);
  EQ(double(1234), hits[1]->s_time);
}
void hitmgrtest::sort_2()
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.

  DDASReadout::ZeroCopyHit hit3(100, buf->s_pData, buf, m_pArena);
  hit3.s_time = 1222;
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  hits.push_back(&hit3);
  
  m_pTestObject->sortHits(hits);
  
  EQ(double(1111), hits[0]->s_time);
  EQ(double(1222), hits[1]->s_time);
  EQ(double(1234), hits[2]->s_time);
}
void hitmgrtest::merge_1() //  assign.
{
    auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1111;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1222;         // newer.

  DDASReadout::ZeroCopyHit hit3(100, buf->s_pData, buf, m_pArena);
  hit3.s_time = 1234;
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  hits.push_back(&hit3);
  
  m_pTestObject->mergeHits(hits);
  EQ(hits, m_pTestObject->m_sortedHits);
}

void hitmgrtest::merge_2()          // append.
{
  auto buf = m_pArena->allocate(1024);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;                 // Everything's going to append to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  hits.push_back(&hit1);
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 6789;
  hits.push_back(&hit2);
   
  m_pTestObject->mergeHits(hits);
  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(6789), m_pTestObject->m_sortedHits[2]->s_time);
}
void hitmgrtest::merge_3()          // prepend
{
  auto buf = m_pArena->allocate(1024);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 9999;                 // Everything's going to append to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  hits.push_back(&hit1);
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 6789;
  hits.push_back(&hit2);
   
  m_pTestObject->mergeHits(hits);  

  EQ(double(1234), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(6789), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[2]->s_time);

}
void hitmgrtest::merge_4()           // Merge with tail.
{
  auto buf = m_pArena->allocate(1024);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;                 // Everything's going to append to this.
  m_pTestObject->m_sortedHits.push_back(&hit0);
  
  DDASReadout::ZeroCopyHit hit00(100, buf->s_pData, buf, m_pArena);
  hit00.s_time = 2222;
  m_pTestObject->m_sortedHits.push_back(&hit00);
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;                    // will b second.
  hits.push_back(&hit1);
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 9999;                    // will b last.
  hits.push_back(&hit2);
  
  m_pTestObject->mergeHits(hits);
  
  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(2222), m_pTestObject->m_sortedHits[2]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[3]->s_time);
}
void hitmgrtest::add_1()               // Soret->assign.
{
  // A couple of hits get sorted in time order:
  
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  
  m_pTestObject->addHits(hits);
  EQ(double(1111), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
    
}
void hitmgrtest::add_2()                  // sort/append to existing.
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit0);
  
  m_pTestObject->addHits(hits);
  
  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1111), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[2]->s_time);
}
void hitmgrtest::add_3()             // prepend
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 9999;
  m_pTestObject->m_sortedHits.push_back(&hit0);
  
  m_pTestObject->addHits(hits);
  
  EQ(double(1111), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[2]->s_time);
}
void hitmgrtest::add_4()            // interleaved.
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 1234;
  
  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1111;         // newer.
  
  std::deque<DDASReadout::ZeroCopyHit*> hits;
  hits.push_back(&hit1);
  hits.push_back(&hit2);
  
  DDASReadout::ZeroCopyHit hit0(100, buf->s_pData, buf, m_pArena);
  hit0.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit0);

  DDASReadout::ZeroCopyHit hit00(100, buf->s_pData, buf, m_pArena);
  hit00.s_time = 9999;
  m_pTestObject->m_sortedHits.push_back(&hit00);
  
  m_pTestObject->addHits(hits);
  
  EQ(double(0), m_pTestObject->m_sortedHits[0]->s_time);
  EQ(double(1111), m_pTestObject->m_sortedHits[1]->s_time);
  EQ(double(1234), m_pTestObject->m_sortedHits[2]->s_time);
  EQ(double(9999), m_pTestObject->m_sortedHits[3]->s_time);
  
}
void hitmgrtest::havehit_1()       // 1 hit - means false (initial tested no hits).
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  
  m_pTestObject->m_sortedHits.push_back(&hit1);
  
  ASSERT(!m_pTestObject->haveHit());
}
void hitmgrtest::havehit_2()           // 2 hits but not outside of window.
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = 1234;
  m_pTestObject->m_sortedHits.push_back(&hit2);
  
  ASSERT(!m_pTestObject->haveHit());
}
void hitmgrtest::havehit_3()            // 2 hits outside of window.
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  DDASReadout::ZeroCopyHit hit2(100, buf->s_pData, buf, m_pArena);
  hit2.s_time = double(10)*1.0e9 + double(1);
  m_pTestObject->m_sortedHits.push_back(&hit2);
  
  ASSERT(m_pTestObject->haveHit());  
}


void hitmgrtest::nexthit_1()         // Can get one hit if there's one there.
{
  auto buf = m_pArena->allocate(1024);
  DDASReadout::ZeroCopyHit hit1(100, buf->s_pData, buf, m_pArena);
  hit1.s_time = 0;
  m_pTestObject->m_sortedHits.push_back(&hit1);

  DDASReadout::ZeroCopyHit* pHit = m_pTestObject->nextHit();
  EQ(&hit1, pHit);
  
  pHit =  m_pTestObject->nextHit();
  ASSERT(!pHit);
}