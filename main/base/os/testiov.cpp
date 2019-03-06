// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "io.h"
#include <sys/uio.h>

#include "Asserts.h"

// The function we're testing is not exported via the header as it's
// intended to be private...though this trick allows us to see it.

namespace io {
extern struct iovec*
updateIov(struct iovec* iov, int& nItems, ssize_t nBytes);
}

class iovtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(iovtest);
  CPPUNIT_TEST(full_1);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void full_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(iovtest);

void iovtest::full_1() {
  struct iovec iov {
    (void*)(nullptr), 100
  };
  int n = 1;
  io::updateIov(&iov, n, 100);
  
  EQ(0, n);
}
