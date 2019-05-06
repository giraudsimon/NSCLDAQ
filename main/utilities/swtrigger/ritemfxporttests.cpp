// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingItemFileTransport.h"
#include <CRingBlockReader.h>
#include <CBufferedOutput.h>
#include <DataFormat.h>

#include <unistd.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <string.h>

std::string nameTemplate="ringXXXXXX";

class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string m_fileName;            // Acxtual temp filenames.
  int         m_nFd;                 // Fd open for outputter.
  
  CRingFileBlockReader*  m_pReader;
  io::CBufferedOutput*   m_pWriter;
  
  CRingItemFileTransport* m_pRTestObj;   // Reading test obj.
  CRingItemFileTransport* m_pWTestObj;   // Writing test obj.
  
  
public:
  void setUp() {
    char Template[nameTemplate.size() + 1];
    strcpy(Template, nameTemplate.c_str());
    int m_nFd = mkstemp(Template);
    m_fileName = Template;               // Was modified.
    
    // Make a reader and an associated transport:
    
    m_pReader = new CRingFileBlockReader(Template);
    m_pRTestObj = new CRingItemFileTransport(*m_pReader);
    
    // .. and a writer with an associated transport:
    
    m_pWriter = new io::CBufferedOutput(m_nFd, 8192);
    m_pWTestObj = new CRingItemFileTransport(*m_pWriter);
    
    
  }
  void tearDown() {
    delete m_pRTestObj;
    delete m_pWTestObj;
    unlink(m_fileName.c_str());
    
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
