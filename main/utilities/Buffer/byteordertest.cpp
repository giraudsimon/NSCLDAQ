

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#define private public
#define protected public
#include <BufferPtr.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace std;

using namespace DAQ::BO;

class byteordertest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(byteordertest);
  CPPUNIT_TEST(copyAs_0);
  CPPUNIT_TEST(copyAs_1);
  CPPUNIT_TEST(interpretAs_0);
  CPPUNIT_TEST(interpretAs_1);
  CPPUNIT_TEST(interpretAs_2);
  CPPUNIT_TEST(interpretAs_3);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }

  void copyAs_0() {
    union {
      uint16_t asInt;
      char asBytes[sizeof(asInt)];
    } composite;

    composite.asInt = 0x1234;
    CByteSwapper swapper(false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non swapping should return the original",
                                 uint16_t(0x1234), swapper.copyAs<uint16_t>(composite.asBytes));

  }

  void copyAs_1() {
    union {
      uint16_t asInt;
      char asBytes[sizeof(asInt)];
    } composite;

    composite.asInt = 0x1234;

    CByteSwapper swapper(true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Swapping should not return the original",
                                 uint16_t(0x3412),
                                 swapper.copyAs<uint16_t>(composite.asBytes));

  }

  void interpretAs_0() {
    std::uint32_t arr[5] = {0, 1, 2, 3, 4};
    const char* p = reinterpret_cast<const char*>(arr);

    std::uint32_t expected[5] = {0x00000000, 0x01000000, 0x02000000, 0x03000000, 0x04000000};
    std::uint32_t newArr [5];
    CByteSwapper swapper(true);

    swapper.interpretAs(p, newArr);

    for (int i=0; i<5; ++i) {
      string msg("array should swap intelligently : ");
      msg += to_string(i);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.c_str(),
                                   expected[i], newArr[i]);
    }

  }

  void interpretAs_1() {
    char aString[39] = "this is the order the string should be";
    char test[39];
    CByteSwapper swapper(true);
    swapper.interpretAs(aString, test);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("char[N] should not swap",
                                 0, std::strncmp(aString, test, 39));

  }

  void interpretAs_2() {
    std::uint32_t arr[5] = {0, 1, 2, 3, 4};
    const char* p = reinterpret_cast<const char*>(arr);

    std::uint32_t newArr [5];
    CByteSwapper swapper(false);

    swapper.interpretAs(p, newArr);

    for (int i=0; i<5; ++i) {
      string msg("array should swap intelligently : ");
      msg += to_string(i);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.c_str(),
                                   arr[i], newArr[i]);
    }

  }

  void interpretAs_3() {
    char aString[39] = "this is the order the string should be";
    char test[39];
    CByteSwapper swapper(false);
    swapper.interpretAs(aString, test);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("char[N] should not swap",
                                 0, std::strncmp(aString, test, 39));

  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(byteordertest);


