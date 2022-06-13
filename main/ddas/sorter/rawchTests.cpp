// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <stdint.h>
#include "RawChannel.h"
#include "testcommon.h"

#include <stdexcept>
#include <string.h>


class rawchTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rawchTest);
  CPPUNIT_TEST(construct_1);
  CPPUNIT_TEST(construct_2);
  CPPUNIT_TEST(construct_3);
  
  CPPUNIT_TEST(copyin_1);
  CPPUNIT_TEST(setdata_1);
  
  CPPUNIT_TEST(settime_1);
  CPPUNIT_TEST(settime_2);
  CPPUNIT_TEST(settime_3);
  
  CPPUNIT_TEST(setlength_1);
  CPPUNIT_TEST(setlength_2);
  
  CPPUNIT_TEST(setchan_1);
  
  CPPUNIT_TEST(validate_1);
  CPPUNIT_TEST(validate_2);
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
  
  void settime_1();
  void settime_2();
  void settime_3();
  
  void setlength_1();
  void setlength_2();
  
  void setchan_1();
  
  void validate_1();
  void validate_2();
private:

};

CPPUNIT_TEST_SUITE_REGISTRATION(rawchTest);

///////////////////////////////////////////////////////////////////////////////
// Utilities


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

void rawchTest::settime_1()
{
  // Set the time without a calibration.  That puts the raw time
  // value in the time member (I think).
  
  uint32_t data[4];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetTime();
  
  EQ(double(12345678), ch.s_time);
}

void rawchTest::settime_2()
{
  // Set time with a calibration multiplier.
  uint32_t data[4];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetTime(2.0);
  
  EQ(double(12345678*2), ch.s_time);

}

void rawchTest::settime_3()
{
  // Set time from external stamp and scale.
  

  uint32_t data[6];
  makeHit(data, 1,2,3, 12345678, 100);
  data[4] = 0x54321;
  data[5] = 0x1234;
  
  // Fix up data[0] as well:
  
  data[0] = (6 << 17) | (6 << 12) | (1 << 8) | (2 << 4) | 3;
  
  DDASReadout::RawChannel ch;
  ch.setData(6, data);
  ch.SetTime(2.0, true);
  
  EQ(double(0x123400054321*2), ch.s_time);
  
}

void rawchTest::setlength_1()
{
  // GThe length from makeHit will be four:
  
  uint32_t data[4];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetLength();
  
  EQ(4, ch.s_channelLength);
}
void rawchTest::setlength_2()
{
  // Jigger the eventlength field in the hit
  // channel length should change accordingly.
  
  uint32_t data[8];
  makeHit(data, 1,2,3, 12345678, 100);
  data[0] = (data[0] & 0x80010000) |  (8 << 17);   // Change the event length->8
  
  DDASReadout::RawChannel ch;
  ch.setData(8, data);
  ch.SetLength();
  EQ(8, ch.s_channelLength);
  
}
void rawchTest::setchan_1()
{
  // Channel is extracted properly from the hit:
  
  uint32_t data[4];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetChannel();
  
  EQ(3, ch.s_chanid);
}

void rawchTest::validate_1()
{
  /// Valid length check ok
  
  uint32_t data[4];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetLength();
  EQ(0, ch.Validate(4));
}
void rawchTest::validate_2()
{
  // invalid length.
  
  uint32_t data[8];
  makeHit(data, 1,2,3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(8, data);
  ch.SetLength();
  EQ(1, ch.Validate(8));         // Size field says 4
}