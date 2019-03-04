// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "daqshm.h"


class createTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(createTests);
  CPPUNIT_TEST(mainLine);
  CPPUNIT_TEST(noDups);
  CPPUNIT_TEST_SUITE_END();


private:
  static const char* shmName;
public:
  void setUp() {
  }
  void tearDown() {
    CDAQShm::remove(shmName);
  }
protected:
  void mainLine();
  void noDups();
};


const char* createTests::shmName = "/testshm";

CPPUNIT_TEST_SUITE_REGISTRATION(createTests);


/**
 * Main line test tests the creation of the 
 * shared memory region by doing a create
 * and checking to see if it can be opened.
 * the flags passed are 0 which should 
 * map to O_RDWR. There are no os independent ways to test things like
 * the mode etc.
 */

void createTests::mainLine() {
  ASSERT(!CDAQShm::create(shmName, 0x1000, 0));
  
}
/**
 * Catch duplicate creation errors:
 */
void createTests::noDups() {
  ASSERT(!CDAQShm::create(shmName, 0x1000, 0));
  ASSERT(CDAQShm::create(shmName, 0x1000, 0));
  EQ(CDAQShm::Exists, CDAQShm::lastError());
}
