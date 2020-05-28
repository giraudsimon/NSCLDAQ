// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "Asserts.h"

#include "CFragmentMaker.h"

#include <DataFormat.h>
#include <fragment.h>


class FragmakerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FragmakerTests);
  CPPUNIT_TEST(contruction);
  CPPUNIT_TEST(hasHeader_1);
  CPPUNIT_TEST(hasHeader_2);
  CPPUNIT_TEST(hasHeader_3);
  CPPUNIT_TEST(hasHeader_4);
  CPPUNIT_TEST(hasHeader_5);
  
  CPPUNIT_TEST(noHeader_1);
  CPPUNIT_TEST(noHeader_2);
  CPPUNIT_TEST(noHeader_3);
  CPPUNIT_TEST(noHeader_4);
  CPPUNIT_TEST_SUITE_END();


private:
  CFragmentMaker* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new CFragmentMaker(123);
  }
  void tearDown() {
    delete m_pTestObj;
    m_pTestObj = nullptr;
  }
protected:
  void contruction();
  void hasHeader_1();
  void hasHeader_2();
  void hasHeader_3();
  void hasHeader_4();
  void hasHeader_5();
  
  void noHeader_1();
  void noHeader_2();
  void noHeader_3();
  void noHeader_4();
private:
  RingItem* makeBodyHeaderStateChange(StateChangeItem& item, int type);
  RingItem* makeNoHeaderStateChange(StateChangeItem& item, int type);
};

CPPUNIT_TEST_SUITE_REGISTRATION(FragmakerTests);

//////////////////////////////////////////////////////////////////////////
// Utilities

// Fill in a state change item with 'standards body header stuff.

RingItem*
FragmakerTests::makeBodyHeaderStateChange(StateChangeItem& item, int type)
{
  // Ok to us sizeof(BodyHeader) here because we're building contrived
  // data which has no body header.
  
  item.s_header.s_size =
    sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(StateChangeItemBody);
  item.s_header.s_type = type;
  
  // the body is unimportant but the body header contents matter:
  // Again we're contriving the event so we know there's no body header extension.
  
  item.s_body.u_hasBodyHeader.s_bodyHeader.s_size      = sizeof(BodyHeader);
  item.s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp = 0x12345678;
  item.s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId  = 12;
  if (type == BEGIN_RUN)
    item.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier   = 1;
  else if (type == END_RUN) {
    item.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier = 2;
  } else {
    item.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier = 0; // not a barrier.
  }
  
  return reinterpret_cast<RingItem*>(&item);
}

RingItem*
FragmakerTests::makeNoHeaderStateChange(StateChangeItem& item, int type)
{
  item.s_header.s_size =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(StateChangeItemBody);
  item.s_header.s_type = type;
  
  item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);
 
  return reinterpret_cast<RingItem*>(&item);
}

//////////////////////////////////////////////////////////////////////////
// Tests

void FragmakerTests::contruction() {
  EQ(uint64_t(0), m_pTestObj->getLastTimestamp());
  EQ(0, m_pTestObj->getEndRunsRemaining());
  
}

/**
 * Ring items with body headers will use that to populate the
 * fragment header:
 */
void FragmakerTests::hasHeader_1()
{
   // Statechange items are simplest:
   
  StateChangeItem item;

  
  EVB::FragmentHeader hdr = m_pTestObj->makeHeader(
    makeBodyHeaderStateChange(item, BEGIN_RUN)
  );
  
  EQ(uint64_t(0x12345678), hdr.s_timestamp);
  EQ(uint32_t(12), hdr.s_sourceId);
  EQ(uint32_t(1), hdr.s_barrier);
  EQ(item.s_header.s_size, hdr.s_size);
    
}
// a state change item updates the internal last timestamp value.

void FragmakerTests::hasHeader_2()
{
  StateChangeItem item;
  m_pTestObj->makeHeader(
    makeBodyHeaderStateChange(item, PAUSE_RUN)
  );
  
  EQ(uint64_t(0x12345678),  m_pTestObj->getLastTimestamp());
}
// A null timestamp results in a header with the most recent timestamp.

void FragmakerTests::hasHeader_3()
{
  StateChangeItem item;
  m_pTestObj->makeHeader(
    makeBodyHeaderStateChange(item,PAUSE_RUN)
  );                    // Last timestamp is now 0x1234678
  
  // give Item a null timestamp.
  
  
  pBodyHeader pB =
    reinterpret_cast<pBodyHeader>(bodyHeader(reinterpret_cast<pRingItem>(&item)));
  pB->s_timestamp = NULL_TIMESTAMP;
  
  EVB::FragmentHeader hdr = m_pTestObj->makeHeader(
    reinterpret_cast<RingItem*>(&item)
  );
  
  EQ(uint64_t(0x12345678), hdr.s_timestamp);
  EQ(uint64_t(0x12345678), m_pTestObj->getLastTimestamp());  // Not modified.
  
}
// BEGIN_RUNs increment the remaining end run count.

void FragmakerTests::hasHeader_4()
{
  StateChangeItem item;
  m_pTestObj->makeHeader(makeBodyHeaderStateChange(item, BEGIN_RUN));
  EQ(1, m_pTestObj->getEndRunsRemaining());
}
// END_RUNs decrement the remaining end run count.

void FragmakerTests::hasHeader_5()
{
  StateChangeItem item;
  RingItem* pItem = makeBodyHeaderStateChange(item, BEGIN_RUN);
  m_pTestObj->makeHeader(pItem);
  m_pTestObj->makeHeader(pItem);    // two ends remaining.
  
  item.s_header.s_type = END_RUN;
  m_pTestObj->makeHeader(pItem);        // Now only 1.
  
  EQ(1, m_pTestObj->getEndRunsRemaining());
}

/// If there's no header, the timestamp is most recent.

void FragmakerTests::noHeader_1()
{
  StateChangeItem item;
  RingItem* pItem = makeBodyHeaderStateChange(item, BEGIN_RUN);
  m_pTestObj->makeHeader(pItem);          // last stamp is 0x12345678
  
  // Modify the item so that it does not have a body header:
  
  makeNoHeaderStateChange(item, BEGIN_RUN);
    
  // This test relies on the fact that the result is initialized.

  EVB::FragmentHeader h = m_pTestObj->makeHeader(pItem);
  EQ(uint64_t(0x12345678), h.s_timestamp);       // Timestamp is prior
  EQ(uint32_t(123), h.s_sourceId);               // Sourceid is default.
  EQ(2, m_pTestObj->getEndRunsRemaining());      // run count still incremented.
}
// The barrier type depends on the item type - BEGIN_RUN -> 1.

void FragmakerTests::noHeader_2()
{
  StateChangeItem item;
  EVB::FragmentHeader h = m_pTestObj->makeHeader(
    makeNoHeaderStateChange(item, BEGIN_RUN)
  );
  EQ(uint32_t(1), h.s_barrier);
}
// END_RUN is barrier type 2.

void FragmakerTests::noHeader_3()
{
  StateChangeItem item;
  EVB::FragmentHeader h = m_pTestObj->makeHeader(
    makeNoHeaderStateChange(item, END_RUN)
  );
  EQ(uint32_t(2), h.s_barrier);  
}
// non begin/ends have barrier types of zero:

void FragmakerTests::noHeader_4()
{
  StateChangeItem item;
  EVB::FragmentHeader h = m_pTestObj->makeHeader(
    makeNoHeaderStateChange(item, PAUSE_RUN)
  );
  EQ(uint32_t(0), h.s_barrier);    
}
