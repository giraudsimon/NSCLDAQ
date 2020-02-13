// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <stdint.h>
#include "RawChannel.h"

#include <stdexcept>
#include <string.h>


class rawchTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rawchTest);
  CPPUNIT_TEST(construct_1);
  CPPUNIT_TEST(construct_2);
  CPPUNIT_TEST(construct_3);
  
  CPPUNIT_TEST(copyin_1);
  CPPUNIT_TEST(setdata_1);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {

  }
  void tearDown() {

  }
protected:
  void construct_1();
  void construct_2();
  void construct_3();
  
  void copyin_1();
  void setdata_1();
private:
  void makeHit(
    uint32_t* hit,
    int crate, int slot, int chan, uint64_t rawTime, uint16_t energy,
    uint16_t cfdTime=0
  );
};

CPPUNIT_TEST_SUITE_REGISTRATION(rawchTest);

///////////////////////////////////////////////////////////////////////////////
// Utilities

/**
 * makeHit
 *    Given parameters for a hit, creates the data for a 4 longword hit.
 *
 * @param[out] hit - Pointer to a uint32_t[4]  which will receive the hit.
 * @param    crate   - Hit crate number.
 * @param    slot    - Hit slot number.
 * @param    chan    - Hit chanel number.
 * @param    rawTime - The hit time from the clock.
 * @param    energy  - Energy value.
 * @param    cfdTime - Defaults to 0, the CFD fractional time.
 */
void
rawchTest::makeHit(
  uint32_t* hit,
  int crate, int slot, int chan, uint64_t rawTime, uint16_t energy,
  uint16_t cfdTime
)
{
  int eventSize = 4;
  int hdrSize   = 4;
  memset(hit, 0, sizeof(uint32_t)*4);
  hit[0] =
    (eventSize << 17) | (hdrSize << 12) | (crate << 8) | (slot << 4) | chan;
  hit[1] = rawTime | 0xffffffff;
  hit[2] = (rawTime >> 32) | (cfdTime << 16);
  hit[3] = energy;
  
}
////////////////////////////////////////////////////////////////////////////////
// Tests

void rawchTest::construct_1()
{
  // Default constructor sets pretty much everything to zero:
  
  DDASReadout::RawChannel ch;
  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(false, ch.s_ownData);
  EQ(0, ch.s_ownDataSize);
  EQ(0, ch.s_channelLength);
  EQ((uint32_t*)(nullptr), ch.s_data);
}
void rawchTest::construct_2()
{
  // construct with Locally owned storage:
  
  DDASReadout::RawChannel ch(100);              // Local owned storage.
  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(true, ch.s_ownData);
  EQ(100, ch.s_ownDataSize);
  EQ(0, ch.s_channelLength);
  ASSERT(ch.s_data);
  
}
void rawchTest::construct_3()
{
  // Construct with user owned storage (support zero copy).
 
  uint32_t data[100]; 
  DDASReadout::RawChannel ch(100, data);  // Zcopy storage.
  
  
  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(false, ch.s_ownData);
  EQ(100, ch.s_ownDataSize);
  EQ(100, ch.s_channelLength);           // b/c it's already assumed a hit.
  EQ((uint32_t*)(data), ch.s_data);
}

void rawchTest::copyin_1()
{
  // Copy in a hit:
  
  uint32_t data[4];
  makeHit(data, 1, 2,3, 0x12345, 100);
  DDASReadout::RawChannel ch;
  ch.copyInData(4, data);
  
  EQ(true, ch.s_ownData);
  EQ(4, ch.s_ownDataSize);
  EQ(4, ch.s_channelLength);
  EQ(0, memcmp(data, ch.s_data, sizeof(data)));
}

void rawchTest::setdata_1()
{
  // Same as copyin but zerocopy:
  
  uint32_t data[4];
  makeHit(data, 1, 2,3, 0x12345, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  EQ(0, ch.s_ownDataSize);         // I don't own any data!
  EQ(4, ch.s_channelLength);
  EQ((uint32_t*)(data), ch.s_data);
}