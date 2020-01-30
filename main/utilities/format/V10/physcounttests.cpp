// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"


#include <V10/CRingPhysicsEventCountItem.h>
#include <V10/CRingStateChangeItem.h>

#include <string.h>

using namespace DAQ::V10;

class physcounttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physcounttests);
  CPPUNIT_TEST(simplecons);
  CPPUNIT_TEST(partialcons);
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
  void partialcons();
  void fullcons();
  void castcons();
  void accessors();
  void copycons();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physcounttests);

// Test construction with default constructor.

void physcounttests::simplecons() {
  CRingPhysicsEventCountItem instance;

  EQ(PHYSICS_EVENT_COUNT, instance.type());
  EQ((uint32_t)0, instance.getTimeOffset());
  EQ((uint64_t)0, instance.getEventCount());
  
}
// Test construction given count and offset.

void physcounttests::partialcons()
{
  CRingPhysicsEventCountItem i(12345678, 100);
  EQ((uint64_t)12345678, i.getEventCount());
  EQ((uint32_t)100, i.getTimeOffset());

}

// Test full construction.
//
void physcounttests::fullcons()
{
  time_t t;
  time(&t);

  CRingPhysicsEventCountItem i(12345678, 100, t);

  EQ((uint64_t)12345678, i.getEventCount());
  EQ((uint32_t)100, i.getTimeOffset());
  EQ(t, i.getTimestamp());
}


// Construct from ring item.. good and bad.
//
void physcounttests::castcons()
{
  CRingItem            good(PHYSICS_EVENT_COUNT);
  CRingStateChangeItem bad;

  time_t now;
  time(&now);

  PhysicsEventCountItem body = {sizeof(PhysicsEventCountItem),
				PHYSICS_EVENT_COUNT,
				1234, static_cast<uint32_t>(now), 1234568};
  memcpy(good.getItemPointer(), &body, sizeof(body));
  uint8_t* p = reinterpret_cast<uint8_t*>(good.getBodyCursor());
  p          += sizeof(PhysicsEventCountItem) - sizeof(RingItemHeader);
  good.setBodyCursor(p);

  bool thrown(false);
  try {
    CRingPhysicsEventCountItem i(good);
  }
  catch(...) {
    thrown = true;
  }
  ASSERT(!thrown);

  // Bad construction should throw a bad cast:

  bool rightexception(false);
  thrown = false;
  try {
    CRingPhysicsEventCountItem i(bad);
  }
  catch (std::bad_cast c) {
    thrown = true;
    rightexception = true;
  }
  catch(...) {
    thrown = true;
    rightexception = false;
  }
  ASSERT(thrown);
  ASSERT(rightexception);
}

// Test write accessors (reads have already been tested.
//

void physcounttests::accessors()
{
  CRingPhysicsEventCountItem i(0,0,0);

  i.setTimeOffset(1234);
  time_t now = time(NULL);
  i.setTimestamp(now);
  i.setEventCount(12345678);

  EQ((uint32_t)1234, i.getTimeOffset());
  EQ(now, i.getTimestamp());
  EQ((uint64_t)12345678, i.getEventCount());
}
// Test copy construction

void physcounttests::copycons()
{
  CRingPhysicsEventCountItem original(1234, 10, 5678);
  CRingPhysicsEventCountItem copy(original);

  EQ(original.getBodySize(), copy.getBodySize());
  _RingItem* porig = original.getItemPointer();
  _RingItem* pcopy = copy.getItemPointer();

  // headers must match 

  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

  // Contents must match:

  EQ(original.getTimeOffset(),    copy.getTimeOffset());
  EQ(original.getTimestamp(),     copy.getTimestamp());
  EQ(original.getEventCount(),    copy.getEventCount());
}

