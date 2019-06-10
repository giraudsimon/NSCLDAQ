
// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CTimeout.h"
#include <thread>


class timeoutTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(timeoutTests);
  CPPUNIT_TEST(expired_0);
  CPPUNIT_TEST(expired_1);
  CPPUNIT_TEST(remainingSeconds_0);
  CPPUNIT_TEST(remainingSeconds_1);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
  }
  void tearDown() {
  }

  void expired_0() {
      CTimeout timeout(10000);

      EQMSG("Timeout should not expire if specified time has not passed",
            false, timeout.expired());
  }

  void expired_1() {
      CTimeout timeout(0);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      EQMSG("Timeout should expire if specified time has passed",
            true, timeout.expired());
  }

  void remainingSeconds_0 () {
      CTimeout timeout(10);

      CPPUNIT_ASSERT_MESSAGE(
           "Remaining time should be nonzero if not expired",
                  timeout.getRemainingSeconds() > 0);

  }

  void remainingSeconds_1 () {
      CTimeout timeout(0);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      EQMSG("Remaining time should be zero if expired",
            double(0), timeout.getRemainingSeconds());

  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(timeoutTests);

