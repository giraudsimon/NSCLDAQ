

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <ByteBuffer.h>

#define private public
#define protected public
#include <Deserializer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

using namespace DAQ::Buffer;

class deserializertest : public CppUnit::TestFixture {
  public:
  CPPUNIT_TEST_SUITE(deserializertest);
  CPPUNIT_TEST(extract_0);
  CPPUNIT_TEST(extract_1);
  CPPUNIT_TEST(extract_2);
  CPPUNIT_TEST(extract_3);
  CPPUNIT_TEST(extract_4);
  CPPUNIT_TEST(extract_5);
  CPPUNIT_TEST(eof_0);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
  }
  void tearDown() {
  }


  void extract_0 () {
    ByteBuffer buffer;
    buffer.push_back(0x35);
    buffer.push_back(0x12); // little endian 0x1235

    Deserializer<ByteBuffer> stream(buffer);
    std::uint16_t value;
    stream.extract(value);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting 16 bit number makes sense",
                                 std::uint16_t(0x1235), value);
  }

  void extract_1 () {
    ByteBuffer buffer;
    buffer.push_back(0xfe);
    buffer.push_back(0xff);
    buffer.push_back(0xff);
    buffer.push_back(0xff); // little endian twos-complement of -1

    Deserializer<ByteBuffer> stream(buffer);
    std::int32_t value;
    stream.extract(value);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting 32 bit signed number makes sense",
                                 std::int32_t(-2), value);
  }

  void extract_2 () {
    ByteBuffer buffer;
    buffer.push_back(0xff);
    buffer.push_back(0xff);
    buffer.push_back(0xff);
    buffer.push_back(0xff);

    std::uint32_t value;
    Deserializer<ByteBuffer> stream(buffer);
    stream >> value;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting 32 bit signed number makes sense",
                                 std::uint32_t(0xffffffff), value);
  }

  void extract_3 () {
    ByteBuffer buffer;
    buffer.push_back(0xff);
    buffer.push_back(0xff);
    buffer.push_back(0xff);
    buffer.push_back(0xff);

    Deserializer<ByteBuffer> stream(buffer);

    uint32_t value = stream.extract<std::uint32_t>();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting 32 bit signed number makes sense",
                                 std::uint32_t(0xffffffff), value );
  }

  void extract_4 () {
    ByteBuffer buffer;
    buffer.push_back('a');
    buffer.push_back('b');
    buffer.push_back('c');
    buffer.push_back('d');

    Deserializer<ByteBuffer> stream(buffer);

    char arr[4];
    stream.extract(arr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('a'), arr[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('b'), arr[1]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('c'), arr[2]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('d'), arr[3]);
  }

  void extract_5 () {
    ByteBuffer buffer;
    buffer.push_back('a');
    buffer.push_back('b');
    buffer.push_back('c');
    buffer.push_back('d');

    Deserializer<ByteBuffer> stream(buffer);

    char arr[4];
    stream.extract(arr, arr+4);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('a'), arr[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('b'), arr[1]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('c'), arr[2]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting string array makes sense",
                                 char('d'), arr[3]);
  }

  void eof_0 () {
    ByteBuffer buffer;
    buffer.push_back(1);
    std::uint16_t value;
    Deserializer<ByteBuffer> stream(buffer);
    stream >> value;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Extracting past buffer.end() sets eof()",
                                 true, stream.eof());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(deserializertest);



