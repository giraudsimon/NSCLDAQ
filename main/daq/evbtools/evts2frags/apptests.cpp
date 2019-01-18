// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "CEvents2Fragments.h"
#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>

#include <list>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
// We need some mock classes for the reader and the writer.

// The reader will allow the test system to queue up a set of data descriptors
// which read delivers until they're exhausted.

class CMockRingFileBlockReader : public CRingFileBlockReader
{
private:
  std::list<CRingFileBlockReader::DataDescriptor> m_data;
  CMockRingFileBlockReader() :
    CRingFileBlockReader(dup(STDIN_FILENO)) {} // dup because destruction closes.
  virtual ~CMockRingFileBlockReader();
  
  virtual CRingFileBlockReader::DataDescriptor read(size_t nBytes);
  void addData(CRingFileBlockReader::DataDescriptor desc) {
    m_data.push_back(desc);
  }
  
};

// Destructor must get rid of any remaining, unread blocks - just in case.

CMockRingFileBlockReader::~CMockRingFileBlockReader() {
  for (auto p : m_data) {
    free(p.s_pData);
  }
  // the list will clear itself.
}
CRingFileBlockReader::DataDescriptor
CMockRingFileBlockReader::read(size_t nBytes) // we ignore the buffer size
{
  CRingFileBlockReader::DataDescriptor result;
  if (m_data.empty()) {
    // Return an eof indicator:
    
    result.s_nBytes = 0;
    result.s_nItems = 0;
    result.s_pData  = nullptr;
    
  } else {
    // Return the front item of the queue:
    
    result = m_data.front();
    m_data.pop_front();
    
  }
  
  return result;
}

// Mock for io::CBUfferedOutput - just hold the data written as a list of
// writes.

class MockBufferedOutput : public io::CBufferedOutput {
private:
  std::list<std::pair<size_t, void*>> m_Writes;
public:
  MockBufferedOutput() :
    io::CBufferedOutput(STDIN_FILENO, 8192) {} // base class ~ does not close.
  ~MockBufferedOutput();
  
  virtual void put(const void* pData, size_t nBytes) {
    void* pCopy = malloc(nBytes);
    memcpy(pCopy, pData, nBytes);
    m_Writes.push_back({nBytes, pCopy});
    
  }
  
  std::pair<size_t ,void*> get() {
    std::pair<size_t, void*> result;
    if (m_Writes.empty()) {
      result = {0, nullptr};
    } else {
      result = m_Writes.front();
      m_Writes.pop_front();
    }
    
    return result;
  }
  std::list<std::pair<size_t, void*>>& getWrittenData() { return m_Writes; }
  
  virtual void flush() {}
};
// The destructor free's the data:

MockBufferedOutput::~MockBufferedOutput() {
  for (auto p : m_Writes) {
    free(p.second);
  }
  // The list frees itself.
}

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
