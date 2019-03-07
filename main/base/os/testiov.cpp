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
  CPPUNIT_TEST(full_2);
  
  CPPUNIT_TEST(partial_1);
  CPPUNIT_TEST(partial_2);
  CPPUNIT_TEST(partial_3);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void full_1();
  void full_2();
  
  void partial_1();
  void partial_2();
  void partial_3();
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

void iovtest::full_2() {
  struct iovec iov[2] {
    {(void*)(nullptr), 50},
    {(void*)(nullptr), 50}
  };
  int n = 2;
  io::updateIov(iov, n, 100);
  EQ(0, n);
}

void iovtest::partial_1() { // one vector element partially done.
  struct iovec iov {
    0, 100
  };
  int n = 1;
  io::updateIov(&iov, n, 50);
  EQ(1, n);
  EQ(size_t(50), iov.iov_len);
  EQ((void*)(50), iov.iov_base);
  
}
void iovtest::partial_2() {  // Two vector elements, the first is fully done:
  struct iovec iov[2]{
    {0, 100},
    {0, 200}
  };
  int n = 2;
  struct iovec* next = io::updateIov(iov, n, 100);
  EQ(1, n);
  EQ(&(iov[1]), next);
  EQ(size_t(200), next->iov_len);
  
}
void iovtest::partial_3() { // Two vector elements, first and part of second done.
  struct iovec iov[2]{
    {0, 100},
    {0, 200}
  };
  int n = 2;
  struct iovec* next = io::updateIov(iov, n, 200);  
  
  EQ(1, n);
  EQ(&(iov[1]), next);
  EQ(size_t(100), next->iov_len);
}