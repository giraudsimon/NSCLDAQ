// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "daqshm.h"



class removeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(removeTests);
  CPPUNIT_TEST(removeNoSuch);
  CPPUNIT_TEST(removeOk);
  CPPUNIT_TEST_SUITE_END();


private:
  static const char* shmName;
public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void removeNoSuch();
  void removeOk();
};
const char* removeTests::shmName = "/testshm";

CPPUNIT_TEST_SUITE_REGISTRATION(removeTests);

void removeTests::removeNoSuch() {

  ASSERT(CDAQShm::remove(shmName));
  EQ(CDAQShm::NonExistent, CDAQShm::lastError());
}


void removeTests::removeOk()
{
  CDAQShm::create(shmName, 0x1000, 0);
  ASSERT(!CDAQShm::remove(shmName));

  // Upon removing we should be able to create the shm...
  /// else it didn't really go away:

  ASSERT(!CDAQShm::create(shmName, 0x1000, 0));
  CDAQShm::remove(shmName);
}
