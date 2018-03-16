// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <V10/DataFormatV10.h>

#define private public
#define protected public
#include <V10/CRingScalerItem.h>
#undef private
#undef protected

#include <time.h>
#include <algorithm>
#include <iterator>
#include <numeric>

using namespace std;
using namespace DAQ::V10;

class scltests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(scltests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(fullcons);
  CPPUNIT_TEST(castcons);
  CPPUNIT_TEST(accessors);
  CPPUNIT_TEST(copycons);
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(scltests);


// Test the simple constructor.

void scltests::simplecons() {
  CRingScalerItem s(32);

  EQ(INCREMENTAL_SCALERS, s.type());
  uint32_t zero(0);
  EQ(zero, s.m_pScalers->s_intervalStartOffset);
  EQ(zero, s.m_pScalers->s_intervalEndOffset);
  EQ((uint32_t)time(NULL), s.m_pScalers->s_timestamp);
  EQ((uint32_t)32, s.m_pScalers->s_scalerCount);

}
// Exercise the fully specifying constructor.

void scltests::fullcons()
{
  
  vector<uint32_t> scalerValues;
  for(uint32_t i = 0; i < 32; i++) {
    scalerValues.push_back(i);
  }
  CRingScalerItem s(10, 12, time(NULL), scalerValues);

  EQ(INCREMENTAL_SCALERS, s.type());
  EQ((uint32_t)10, s.m_pScalers->s_intervalStartOffset);
  EQ((uint32_t)12, s.m_pScalers->s_intervalEndOffset);
  EQ((uint32_t)time(NULL),   s.m_pScalers->s_timestamp);
  EQ((uint32_t)32, s.m_pScalers->s_scalerCount);
  for (int i =0; i < 32; i++) {
    EQ(scalerValues[i], s.m_pScalers->s_scalers[i]);
  }
 
}
// Exercise the 'downcast'  construction from an existing ring buffer.
//  This also exercises the getter accessors.
void scltests::castcons()
{
  CRingItem s(INCREMENTAL_SCALERS,
	      sizeof(ScalerItem) + 31*sizeof(uint32_t) - sizeof(RingItemHeader));
  pScalerItem p = reinterpret_cast<pScalerItem>(s.getItemPointer());

  p->s_intervalStartOffset = 10;
  p->s_intervalEndOffset   = 15;
  p->s_timestamp = static_cast<uint32_t>(time(NULL));
  p->s_scalerCount = 32;
  int i;
  for (i =0; i < 32; i++) {
    p->s_scalers[i] = i;
  }
  s.setBodyCursor(&(p->s_scalers[i]));
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

  // test that we can pass in an array of values
  std::vector<uint32_t> values(32);
  std::iota(begin(values), end(values), 32);

  s.setScalers(values);
  ASSERT(values == s.getScalers());

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
