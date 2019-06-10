// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V11/DataFormatV11.h"
#include "V11/CDataFormatItem.h"

using namespace DAQ::V11;

/**
 * tests for the CDataFormat item class.
 */
class DataFormatTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DataFormatTest);
  CPPUNIT_TEST(test);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DataFormatTest);

/**
 * test
 *   exercise the data format class:
 */
void
DataFormatTest::test()
{
    CDataFormatItem item;
    
    // Check the major/minor and typename:
    
    EQ(FORMAT_MAJOR, item.getMajor());
    EQ(FORMAT_MINOR, item.getMinor());
    EQ(std::string("Ring Item format version"), item.typeName());
    
    // Check a few base class bits of info:
    
    EQ(false, item.hasBodyHeader());
    EQ(RING_FORMAT, item.type());
    pDataFormat pItem = reinterpret_cast<pDataFormat>(item.getItemPointer());
    EQ(static_cast<uint32_t>(sizeof(DataFormat)), pItem->s_header.s_size);
}
