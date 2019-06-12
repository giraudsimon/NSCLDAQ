// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemFileTransport.h"
#include "CRingBufferTransport.h"
#include "CRingBuffer.h"

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <typeinfo>
#include <stdexcept>
#include <sys/uio.h>

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

class ringxpFactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ringxpFactoryTests);
  CPPUNIT_TEST(fileWriter);
  CPPUNIT_TEST(fileReader);
  
  CPPUNIT_TEST(ringWriter);
  CPPUNIT_TEST(ringReader);
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
    
    try {CRingBuffer::create(ringName);} catch(...) {}
  }
  void tearDown() {
    close(m_fd);
    unlink(m_filename.c_str());
    try {
      CRingBuffer::remove(ringName);
    } catch(...) {}

  }
protected:
  void fileWriter();
  void fileReader();
  
  void ringWriter();
  void ringReader();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ringxpFactoryTests);

void ringxpFactoryTests::fileWriter() {   // Create a file writer.
  std::string uri = makeFileURI(m_filename.c_str());
  CRingItemTransport* pXport =
    CRingItemTransportFactory::createTransport(uri.c_str(), CRingBuffer::producer);
    
  EQ( typeid(CRingItemFileTransport).hash_code(), typeid(*pXport).hash_code());
  
  void* pData; size_t n;
  CPPUNIT_ASSERT_THROW(
    pXport->recv(&pData,n),
    std::runtime_error
  );
  
  delete pXport;
  
}

void ringxpFactoryTests::fileReader()
{

  std::string uri = makeFileURI(m_filename.c_str());
  CRingItemTransport* pXport =
    CRingItemTransportFactory::createTransport(uri.c_str(), CRingBuffer::consumer);
    
  EQ( typeid(CRingItemFileTransport).hash_code(), typeid(*pXport).hash_code());
  
  iovec v;
  v.iov_base = nullptr;
  v.iov_len  = 0;
  CPPUNIT_ASSERT_THROW(
    pXport->send(&v, 1),
    std::runtime_error
  );
  
  delete pXport;
}

void ringxpFactoryTests::ringWriter()
{
  std::string uri = makeRingURI(ringName.c_str());
  CRingItemTransport* pXport =
    CRingItemTransportFactory::createTransport(uri.c_str(), CRingBuffer::producer);
  
  EQ(typeid(CRingBufferTransport).hash_code(), typeid(*pXport).hash_code());
  
  void* pData;
  size_t n;
  CPPUNIT_ASSERT_THROW(
    pXport->recv(&pData, n),
    std::logic_error
  );
  
  delete pXport;
}

void ringxpFactoryTests::ringReader()
{
  std::string uri = makeRingURI(ringName.c_str());
  CRingItemTransport* pXport =
    CRingItemTransportFactory::createTransport(uri.c_str(), CRingBuffer::consumer);
  
  EQ(typeid(CRingBufferTransport).hash_code(), typeid(*pXport).hash_code());
  
  iovec v;
  v.iov_base = nullptr;
  v.iov_len = 0;
  CPPUNIT_ASSERT_THROW(
    pXport->send(&v, 0),
    std::logic_error
  );
  
  delete pXport;
}