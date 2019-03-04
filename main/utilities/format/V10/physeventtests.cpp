// Template for a test suite.
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <config.h>
#include <V10/DataFormatV10.h>
#include <ByteBuffer.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <V10/CPhysicsEventItem.h>
#undef private
#undef protected

using namespace DAQ::V10;
using namespace std;

class physeventtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physeventtests);
  CPPUNIT_TEST(getBodyData_0);
  CPPUNIT_TEST_SUITE_END();

private:

  CPhysicsEventItem       m_item;
  DAQ::Buffer::ByteBuffer m_bodyData;
public:

  physeventtests() : m_item(100), m_bodyData() {}

  void setUp() {
    m_item = CPhysicsEventItem(100);
    m_bodyData = DAQ::Buffer::ByteBuffer();
    m_bodyData << std::vector<std::uint16_t>({6, 0, 1, 2, 3, 4});

    m_item.fillBody(m_bodyData);
  }
  void tearDown() {
  }

  void getBodyData_0() {
    DAQ::Buffer::ByteBuffer data = m_item.getBodyData();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getBodyData() should retrieve the correct data",
                                 m_bodyData, data);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physeventtests);


