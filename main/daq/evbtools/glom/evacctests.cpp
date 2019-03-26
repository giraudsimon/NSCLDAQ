// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#define private public
#include "CEventAccumulator.h"            // White box testing.
#undef private

#include <fragment.h>
#include <DataFormat.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

std::string FilenameTemplate="evactestXXXXXX";

class evaccTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(evaccTest);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string        m_filename;
  int                m_fd;
  CEventAccumulator* m_pTestObj;
public:
  void setUp() {
    char fnameTemplate[FilenameTemplate.size() +1];
    strcpy(fnameTemplate, FilenameTemplate.c_str());
    m_fd = mkstemp(fnameTemplate);
    m_filename = fnameTemplate;
    
    m_pTestObj =
      new CEventAccumulator(m_fd, 1, 1024, 10, CEventAccumulator::last);
    
  }
  void tearDown() {
    close(m_fd);
    delete m_pTestObj;
    unlink(m_filename.c_str());
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(evaccTest);

void evaccTest::aTest() {
}
