// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemFileTransport.h"
#include "CRingItemTransport.h"
#include "CRingBuffer.h"

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>


static const std::string filenameTemplate("facttestXXXXXX");
static const std::string ringName("facttest");

static std::string makeFileURI(const char* pName)
{
  std::string result = "file://./";
  result += pName;
  
  return result;
}
static std::string makeRingURI(const char* ringName)
{
  std::string result = "tcp://localhost/";
  result += ringName;
  
  return result;
}



///

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string m_filename;
  int         m_fd;
  
public:
  void setUp() {
    char tempName[filenameTemplate.size() + 1];
    strcpy(tempName, filenameTemplate.c_str());
    m_fd = mkstemp(tempName);
    m_filename = tempName;             // mkstemp modified.
    
    CRingBuffer::create(ringName);
  }
  void tearDown() {
    close(m_fd);
    unlink(m_filename.c_str());
    
    CRingBuffer::remove(ringName);
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
