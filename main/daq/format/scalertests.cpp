// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#define private public
#define protected public
#include <CRingScalerItem.h>
#undef private
#undef protected

#include <DataFormat.h>
#include <time.h>

using namespace std;

class scltests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(scltests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST(copycons);
  CPPUNIT_TEST(tstampCons);
  CPPUNIT_TEST(tstampCopyCons);
  CPPUNIT_TEST(fractionalRunTime);
  CPPUNIT_TEST(incremental);
  CPPUNIT_TEST(origsid);           // V12.0-pre1
  CPPUNIT_TEST(tsorigsid);         // V12.0-pre1
  CPPUNIT_TEST(origsid_1);
  CPPUNIT_TEST(origsid_2);
  CPPUNIT_TEST(origsid_3);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void simplecons();
  void fullcons();
  void castcons();
  void accessors();
  void copycons();
  void tstampCons();
  void tstampCopyCons();
  void fractionalRunTime();
  void incremental();
  void origsid();
  void tsorigsid();
  void origsid_1();
  void origsid_2();
  void origsid_3();

};

CPPUNIT_TEST_SUITE_REGISTRATION(scltests);


// Test the simple constructor.

void scltests::simplecons() {
  CRingScalerItem s(32);

  pScalerItem pItem = reinterpret_cast<pScalerItem>(s.getItemPointer());
  
  EQ(PERIODIC_SCALERS, s.type());
  uint32_t zero(0);
  EQ(zero, pItem->s_body.u_noBodyHeader.s_body.s_intervalStartOffset);
  EQ(zero, pItem->s_body.u_noBodyHeader.s_body.s_intervalEndOffset);
  EQ((uint32_t)time(NULL), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQ((uint32_t)32, pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);
  EQ((uint32_t)1, pItem->s_body.u_noBodyHeader.s_body.s_isIncremental);
  EQ((uint32_t)1, pItem->s_body.u_noBodyHeader.s_body.s_intervalDivisor);

}
// Exercise the fully specifying constructor.

void scltests::fullcons()
{
  
  vector<uint32_t> scalerValues;
  for(uint32_t i = 0; i < 32; i++) {
    scalerValues.push_back(i);
  }
  CRingScalerItem s(10, 12, time(NULL), scalerValues);
  pScalerItem pItem = reinterpret_cast<pScalerItem>(s.getItemPointer());

  EQ(PERIODIC_SCALERS, s.type());
  EQ((uint32_t)10, pItem->s_body.u_noBodyHeader.s_body.s_intervalStartOffset);
  EQ((uint32_t)12, pItem->s_body.u_noBodyHeader.s_body.s_intervalEndOffset);
  EQ((uint32_t)time(NULL),   pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQ((uint32_t)32, pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);
  for (int i =0; i < 32; i++) {
    EQ(scalerValues[i], pItem->s_body.u_noBodyHeader.s_body.s_scalers[i]);
  }
 
}
// Exercise the 'downcast'  construction from an existing ring buffer.
//  This also exercises the getter accessors.
void scltests::castcons()
{
  CRingItem s(PERIODIC_SCALERS,
	      sizeof(ScalerItem) + 31*sizeof(uint32_t) - sizeof(RingItemHeader));
  pScalerItem p = reinterpret_cast<pScalerItem>(s.getItemPointer());

  p->s_body.u_noBodyHeader.s_body.s_intervalStartOffset = 10;
  p->s_body.u_noBodyHeader.s_body.s_intervalEndOffset   = 15;
  p->s_body.u_noBodyHeader.s_body.s_timestamp = static_cast<uint32_t>(time(NULL));
  p->s_body.u_noBodyHeader.s_body.s_scalerCount = 32;
  int i;
  for (i =0; i < 32; i++) {
    p->s_body.u_noBodyHeader.s_body.s_scalers[i] = i;
  }
  s.setBodyCursor(&(p->s_body.u_noBodyHeader.s_body.s_scalers[i]));
  s.updateSize();


  // Now the item is prepared, construct the scaler from it:

  bool thrown = false;
  try {
    CRingScalerItem sc(s);
    EQ((uint32_t)10, sc.getStartTime());
    EQ((uint32_t)15, sc.getEndTime());
    for (int i = 0; i < 32; i++) {
      EQ((uint32_t)i, sc.getScaler(i));
    }
  }
  catch (bad_cast) {
    thrown = true;
  }
  catch (CRangeError& re) {
    thrown = true;
  }
  ASSERT(!thrown);

  // Now let's force a throw:

  CRingItem bad(BEGIN_RUN);
  thrown = false;
  try {
    CRingScalerItem sc(bad);
  }
  catch (bad_cast) {
    thrown = true;
  }
  ASSERT(thrown);
  
}


// Test the setting accessors.

void scltests::accessors()
{
  CRingScalerItem s(32);
  s.setStartTime(100);
  s.setEndTime(105);
  for (int i = 0; i < 32; i++) {
    s.setScaler(i, i);
  }

  EQ((uint32_t)100, s.getStartTime());
  EQ((uint32_t)105, s.getEndTime());
  for (uint32_t i = 0; i < 32; i++) {
    EQ(i, s.getScaler(i));
  }
}
// Test copy construction.

void scltests::copycons()
{
  vector<uint32_t> simulatedScaler;
  for (uint32_t i=0; i < 64; i++) {
    simulatedScaler.push_back(i);
  }

  CRingScalerItem original(0, 10, 12345, simulatedScaler);
  CRingScalerItem copy(original);

  EQ(original.getBodySize(), copy.getBodySize());
  _RingItem* porig = original.getItemPointer();
  _RingItem* pcopy = copy.getItemPointer();

  // headers must match 

  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

  // Contents must match:


  EQ(original.getStartTime(),   copy.getStartTime());
  EQ(original.getEndTime(),    copy.getEndTime());
  EQ(original.getTimestamp(),  copy .getTimestamp());
  EQ(original.getScalerCount(), copy.getScalerCount());
  for (uint32_t i =0; i < 64; i++) {
    EQ(i, copy.getScaler(i));
  }
  
}
/*
  Test construction of timestamped scaler items.
*/
void
scltests::tstampCons()
{
    std::vector<uint32_t> scalers;
    for(int i=0; i < 16; i++) {
        scalers.push_back(i);
    }
    CRingScalerItem item(
        0x1234567887654321ll, 1, 0,
        10, 20, (time_t)1111, scalers
    );
    
    // Ensure we can get the body header stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1234567887654321ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(0), item.getBarrierType());
    
    // And that we can still get the stuff from the event body:
    
    EQ(static_cast<uint32_t>(10), item.getStartTime());
    EQ(static_cast<uint32_t>(20), item.getEndTime());
    EQ(static_cast<time_t>(1111), item.getTimestamp());
    EQ(static_cast<uint32_t>(16), item.getScalerCount());
    
    for(int i = 0; i < 16; i++) {
        EQ(static_cast<uint32_t>(i), item.getScaler(i));
    }
    
    // Make sure that scaler value alterations work too:
    
    for(int i = 0; i < 16; i++) {
        item.setScaler(i, 100-i);
    }
    for (int i = 0; i < 16; i++) {
        EQ(static_cast<uint32_t>(100-i), item.getScaler(i));
    }
}
/*
  test copy construction with body headers:
*/
void
scltests::tstampCopyCons()
{
    std::vector<uint32_t> scalers;
    for(int i=0; i < 16; i++) {
        scalers.push_back(i);
    }
    CRingScalerItem orig(
        0x1234567887654321ll, 1, 0,
        10, 20, (time_t)1111, scalers
    );
    CRingScalerItem item(orig);
    
    // Ensure we can get the body header stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1234567887654321ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(0), item.getBarrierType());
    
    // And that we can still get the stuff from the event body:
    
    EQ(static_cast<uint32_t>(10), item.getStartTime());
    EQ(static_cast<uint32_t>(20), item.getEndTime());
    EQ(static_cast<time_t>(1111), item.getTimestamp());
    EQ(static_cast<uint32_t>(16), item.getScalerCount());
    
    for(int i = 0; i < 16; i++) {
        EQ(static_cast<uint32_t>(i), item.getScaler(i));
    }
    
    // Make sure that scaler value alterations work too:
    
    for(int i = 0; i < 16; i++) {
        item.setScaler(i, 100-i);
    }
    for (int i = 0; i < 16; i++) {
        EQ(static_cast<uint32_t>(100-i), item.getScaler(i));
    }
}
/*
* tests for the compute*Time methods
*/
void
scltests::fractionalRunTime()
{
    std::vector<uint32_t> scalers;
    for(int i=0; i < 16; i++) {
        scalers.push_back(i);
    }
    CRingScalerItem orig(
        0x1234567887654321ll, 1, 0,
        10, 20, (time_t)1111, scalers, 2
    );
    EQ(static_cast<float>(10.0/2.0), orig.computeStartTime());
    EQ(static_cast<float>(20.0/2.0), orig.computeEndTime());
}
/**
 * test isIncremental.
 */
void
scltests::incremental()
{
    std::vector<uint32_t> scalers;
    for(int i=0; i < 16; i++) {
        scalers.push_back(i);
    }
    CRingScalerItem inc(
        0x1234567887654321ll, 1, 0,
        10, 20, (time_t)1111, scalers, 2
    );
    CRingScalerItem notinc(
        0x1234567887654321ll, 1, 0,
        10, 20, (time_t)1111, scalers, 2, false
    );
    ASSERT(inc.isIncremental());
    ASSERT(!notinc.isIncremental());
    
}
  // These tests were added for 12.0-pre1 to check that the s_originalSid
  // field of the body is correctly set.
  
void
scltests::origsid()
{
  uint32_t counters[32];
  time_t now = time(nullptr);
  pScalerItem pItem = formatScalerItem(
    32, now, 0, 10, counters
  );
  pScalerItemBody pBody = reinterpret_cast<pScalerItemBody>(bodyPointer(
    reinterpret_cast<pRingItem>(pItem)
  ));
  EQ(uint32_t(0), pBody->s_originalSid);
  
  free(pItem);
}
void
scltests::tsorigsid()
{
    uint32_t counters[32];
    time_t now = time(nullptr);
    
    pScalerItem pItem = formatTimestampedScalerItem(
        0x123456789, 15, 0, 1, 1, static_cast<uint32_t>(now), 0, 10, 32,
        counters
    );
    
    pScalerItemBody pBody = reinterpret_cast<pScalerItemBody>(bodyPointer(
    reinterpret_cast<pRingItem>(pItem)
  ));
  EQ(uint32_t(15), pBody->s_originalSid);
  
  free(pItem);
    
}

void scltests::origsid_1()
{
  CRingScalerItem item(5);
  EQ(uint32_t(0), item.getOriginalSourceId());
}
void scltests::origsid_2()
{
  std::vector<uint32_t> scalers={1,2,3,4,5};
  CRingScalerItem item(0, 10, time(nullptr), scalers);
  EQ(uint32_t(0), item.getOriginalSourceId());
}
void scltests::origsid_3()
{
  std::vector<uint32_t> scalers={1,2,3,4,5,6};
  CRingScalerItem item(
    0x123456789a, 5, 0, 10, 20, time(nullptr), scalers
  );
  EQ(uint32_t(5), item.getOriginalSourceId());

}
