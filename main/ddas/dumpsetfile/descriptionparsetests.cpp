// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <sstream>
#include <string>
#include <cstdint>
#include "Asserts.h"
#include "DescriptionParser.h"


class VarFileParseTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VarFileParseTests);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(single);
  CPPUNIT_TEST(single2);
  CPPUNIT_TEST(perchandSingle);
  CPPUNIT_TEST(mixed);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void empty();
  void single();
  void single2();
  void perchandSingle();
  void mixed();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarFileParseTests);


//  Empty input file results in empty vector:
void VarFileParseTests::empty() {
  std::stringstream testFile(std::string(""));   // stream on empty file.
  DescriptionParser testParse;
  testParse.parseFile(testFile);
  std::vector<DescriptionParser::DSPParameter> v = testParse.getDescription();
  
  EQ(size_t(0), v.size());
}

// Description file with one setting.
void VarFileParseTests::single()
{
  std::stringstream testFile(std::string("0x0004a000  ModNum"));
  DescriptionParser testParse;
  testParse.parseFile(testFile);
  
  std::vector<DescriptionParser::DSPParameter> v = testParse.getDescription();
  EQ(size_t(1), v.size());
  EQ(std::uint32_t(0), v[0].s_offset);
  EQ(unsigned(1), v[0].s_qty);
  EQ(std::string("ModNum"), v[0].s_name);
}

// Description file with two single elements:

void VarFileParseTests::single2()
{
  std::stringstream testFile(std::string("0x0004a000 ModNum\n0x0004a001  ModCSRA"));
  
  DescriptionParser testParse;
  testParse.parseFile(testFile);
  
  std::vector<DescriptionParser::DSPParameter> v = testParse.getDescription();
  EQ(size_t(2), v.size());
  EQ(std::uint32_t(0), v[0].s_offset);
  EQ(unsigned(1), v[0].s_qty);
  EQ(std::string("ModNum"), v[0].s_name);
  
  EQ(std::uint32_t(4), v[1].s_offset);
  EQ(unsigned(1), v[1].s_qty);
  EQ(std::string("ModCSRA"), v[1].s_name);
}
  
  
// A per-channel variable followed by a single.

void VarFileParseTests::perchandSingle()
{
  std::stringstream testFile(std::string("0x0004a000 ModNum\n0x0004a010  ModCSRA"));
  
  DescriptionParser testParse;
  testParse.parseFile(testFile);
  
  std::vector<DescriptionParser::DSPParameter> v = testParse.getDescription();
  EQ(size_t(2), v.size());
  EQ(std::uint32_t(0), v[0].s_offset);
  EQ(unsigned(16), v[0].s_qty);
  EQ(std::string("ModNum"), v[0].s_name);
  
  EQ(std::uint32_t(16*sizeof(std::uint32_t)), v[1].s_offset);
  EQ(unsigned(1), v[1].s_qty);
  EQ(std::string("ModCSRA"), v[1].s_name);
}

// mixed bag of per board/per channel items.

void VarFileParseTests::mixed()
{
  // ModNum - per module.  ModCSRA per channel, ModFormat per module.
  std::stringstream testFile(std::string(
      "0x0004a000 ModNum\n0x0004a001  ModCSRA\n0x0004a011 ModCSRB\n0x004a012 ModFormat"));
  
  DescriptionParser testParse;
  testParse.parseFile(testFile);
  
  std::vector<DescriptionParser::DSPParameter> v = testParse.getDescription();
  EQ(size_t(4), v.size());
  EQ(std::uint32_t(0), v[0].s_offset);
  EQ(unsigned(1), v[0].s_qty);
  EQ(std::string("ModNum"), v[0].s_name);
  
  EQ(std::uint32_t(sizeof(std::uint32_t)), v[1].s_offset);
  EQ(unsigned(16), v[1].s_qty);
  EQ(std::string("ModCSRA"), v[1].s_name);
  
  EQ(std::uint32_t(sizeof(std::uint32_t)*17), v[2].s_offset);
  EQ(unsigned(1), v[2].s_qty);
  EQ(std::string("ModCSRB"), v[2].s_name);
  
  EQ(std::uint32_t(sizeof(std::uint32_t)*18), v[3].s_offset);
  EQ(unsigned(1), v[3].s_qty);
  EQ(std::string("ModFormat"), v[3].s_name);
}