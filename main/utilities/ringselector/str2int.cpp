// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <StringsToIntegers.h>

#include <CInvalidArgumentException.h>
// #include <DataFormat.h>

using namespace std;




class str2int : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(str2int);
  CPPUNIT_TEST(one);
  CPPUNIT_TEST(two);
  CPPUNIT_TEST(many);
  CPPUNIT_TEST(multidigit);
  CPPUNIT_TEST(bad);
  CPPUNIT_TEST(text);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void one();
  void two();
  void many();
  void multidigit();
  void bad();
  void text();
};

CPPUNIT_TEST_SUITE_REGISTRATION(str2int);

// Parse a single element list.

void str2int::one() {
  string      str("1234");

  vector<int> items = stringListToIntegers(str);

  EQ((size_t)1, items.size());
  EQ(1234,items[0]);
}

// Parse a list with two elements.

void str2int::two()
{
  string s("0,1");
  vector<int> items = stringListToIntegers(s);
  EQ((size_t)2, items.size());
  for (int i=0; i < items.size(); i++) {
    EQ(i, items[i]);
  }
}


// parse a list with many elemnts.

void
str2int::many()
{
  string s("0,1,2,3,4,5,6,7,8,9");

  vector<int> items = stringListToIntegers(s);
  EQ((size_t)10, items.size());
  for (int i=0; i < items.size(); i++) {
    EQ(i, items[i]);
  }
}

// Parse multidigit strings:

void
str2int::multidigit()
{
  string s("100,101,102,103,104");
  vector<int> items = stringListToIntegers(s);

  EQ((size_t)5, items.size());
  for (int i = 0; i< items.size(); i++) {
    EQ(i+100, items[i]);
  }
}
// ensure bad values throw the right exception:

void
str2int::bad()
{
  string s("george");
  bool thrown(false);
  try {
    vector<int> items = stringListToIntegers(s);
  }
  catch(CInvalidArgumentException e) {
    thrown = true;
  }
  ASSERT(thrown);
}

// Should be able to use symbolic names mixed with numbers.


void
str2int::text()
{
  string s("BEGIN_RUN,2,3,PERIODIC_SCALERS,31");

  vector<int> items = stringListToIntegers(s);
  ASSERT(items.size() == 5);
  EQ((int)BEGIN_RUN, items[0]);
  EQ(2, items[1]);
  EQ(3, items[2]);
  EQ((int)PERIODIC_SCALERS, items[3]);
  EQ(31, items[4]);
}
