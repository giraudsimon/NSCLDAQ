// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>
#include "CThreadedProcessingElement.h"
#include "CProcessingElement.h"


class TestProcessingElement : public CProcessingElement
{
public:
  bool hasRun;
  
  TestProcessingElement() : hasRun(false) {}
  void operator()() {hasRun = true;}
  void process(void* pData, size_t nBytes) {}
};


class tproctest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(tproctest);
  CPPUNIT_TEST(thread_1);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void thread_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(tproctest);

void tproctest::thread_1() {
  TestProcessingElement test;
  CThreadedProcessingElement testThread(&test);
  testThread.run();
  testThread.join();
  ASSERT(test.hasRun);
}
