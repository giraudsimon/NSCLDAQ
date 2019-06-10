// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "RawChannel.h"


class RawChTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RawChTest);

  // Zero copy tests since that's what's important:
  
  CPPUNIT_TEST(zCopyConstruct);
  
  CPPUNIT_TEST(length_1);
  CPPUNIT_TEST(length_2);
  CPPUNIT_TEST(length_3);
  
  CPPUNIT_TEST(time_1);
  CPPUNIT_TEST(time_2);
  
  CPPUNIT_TEST(channel);
  
  CPPUNIT_TEST(validate);
  
  CPPUNIT_TEST(lt);
  CPPUNIT_TEST(eq);
  CPPUNIT_TEST(gt);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void zCopyConstruct();
  
  void length_1();         // Event length is in bits 17:30 of the first word.
  void length_2();
  void length_3();
  
  void time_1();           // raw time.
  void time_2();           // calibrated time.
  
  void channel();
  
  void validate();
  
  void lt();
  void eq();
  void gt();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RawChTest);

void RawChTest::zCopyConstruct() {
  uint32_t buffer[] = {1, 2, 3, 4, 5, 6, 7};
  DDASReadout::RawChannel c(7, buffer);
  ASSERT(!c.s_ownData);
  EQ(7, c.s_channelLength);
  for (int i =0; i < 7; i++) {
    EQ(buffer[i], c.s_data[i]);
  }
}

void RawChTest::length_1()           // From zerocopy:
{
  uint32_t buffer[] = {7 << 17, 2, 3, 4, 5, 6, 7, 8};
  DDASReadout::RawChannel c(8, buffer);
  c.SetLength();
  
  EQ(7, c.s_channelLength);
}
void RawChTest::length_2()              // From copyInData:
{
  uint32_t buffer[] = {7 << 17, 2, 3, 4, 5, 6, 7, 8};
  DDASReadout::RawChannel c;
  c.copyInData(8, buffer);
  c.SetLength();
  
  EQ(7, c.s_channelLength);
}
void RawChTest::length_3()           // static length getter.
{
  uint32_t buffer[] = {7 << 17, 2, 3, 4, 5, 6, 7, 8};
  EQ(uint32_t(7), DDASReadout::RawChannel::channelLength(buffer));
}

void RawChTest::time_1()
{
  uint32_t buffer[] = { 4 << 17, 0x12345678, 0xabcd, 0};
  DDASReadout::RawChannel c(4, buffer);
  c.SetTime();
  EQ(double(0xabcd12345678), c.s_time);
}
void RawChTest::time_2()
{
  uint32_t buffer[] = {4 << 17, 0xa5a5a5a5, 0, 0};
  DDASReadout::RawChannel c(4, buffer);
  c.SetTime(10.0);
  
  double expected = 10.0*buffer[1];
  EQ(expected, c.s_time);
}

void RawChTest::channel()
{
  uint32_t buffer[] = {4 << 17 | 3, 0x1234, 0x5678, 0, };
  DDASReadout::RawChannel c(4, buffer);
  c.SetChannel();
  EQ(3, c.s_chanid);
}

void RawChTest::validate()
{
  uint32_t buffer[] = {4 << 17, 0, 0, 0};
  DDASReadout::RawChannel c(4, buffer);
  c.SetLength();
  
  EQ(0, c.Validate(4));
  EQ(1, c.Validate(10));
}
void RawChTest::lt()
{
  uint32_t b1[] = {4 << 17,  0, 0 , 0};        // timestamp 0.
  uint32_t b2[] = { 4<< 17, 100, 0, 0};       // ts = 100.
  
  DDASReadout::RawChannel c1(4, b1);
  DDASReadout::RawChannel c2(4, b2);
  c1.SetTime();
  c2.SetTime();
  
  ASSERT(c1 < c2);
  ASSERT(!(c2 < c1));
}
void RawChTest::eq()
{
  uint32_t b1[] = {4 << 17, 100, 1, 2};
  uint32_t b2[] = {4 << 17, 100, 3, 4};
  
  DDASReadout::RawChannel c1(4, b1);
  DDASReadout::RawChannel c2(4, b2);
  
  ASSERT(c1 == c2);
  ASSERT(c2 == c1);
}
void RawChTest::gt()
{
  uint32_t b1[] = {4 << 17,  0, 0 , 0};        // timestamp 0.
  uint32_t b2[] = { 4<< 17, 100, 0, 0};       // ts = 100.
  
  DDASReadout::RawChannel c1(4, b1);
  DDASReadout::RawChannel c2(4, b2);
  c1.SetTime();
  c2.SetTime();
  
  ASSERT(c2 > c1);
  ASSERT(!(c1 > c2));
}