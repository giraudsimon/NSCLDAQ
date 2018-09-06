// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#define private public
#include "CBufferedOutput.h"
#undef private

static const char* filenameTemplate="bftestfileXXXXXX";
static const size_t   bsize(512);

class testBufferedOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testBufferedOutput);
  CPPUNIT_TEST(construct);
  
  CPPUNIT_TEST(put_1);
  CPPUNIT_TEST(put_2);
  
  CPPUNIT_TEST(flush_1);
  CPPUNIT_TEST(flush_2);
  
  CPPUNIT_TEST(insert_1);    // Tests of << operator.
  CPPUNIT_TEST_SUITE_END();


private:
io::CBufferedOutput* m_pBuffer;
int              m_nFd;
std::string      m_filename;
public:
  void setUp() {
    char ftemplate[strlen(filenameTemplate) + 1];
    strcpy(ftemplate, filenameTemplate);
    m_nFd = mkstemp(ftemplate);
    m_filename = ftemplate;
    m_pBuffer = new io::CBufferedOutput(m_nFd, bsize);    
  }
  void tearDown() {
    delete m_pBuffer;
    unlink(m_filename.c_str());
  }
protected:
  void construct();
  
  void put_1();
  void put_2();
  
  void flush_1();
  void flush_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testBufferedOutput);

// Check post construction data:

void testBufferedOutput::construct() {
  EQ(m_nFd, m_pBuffer->m_nFd);
  EQ(m_pBuffer->m_pBuffer, m_pBuffer->m_pInsert);
  EQ(size_t(0), m_pBuffer->m_nBytesInBuffer);
  EQ(bsize, m_pBuffer->m_nBufferSize);
}

// After small put the pointers adjust properly.

void testBufferedOutput::put_1()
{
  ASSERT(size_t(100) < bsize);
  
  uint8_t data[100];
  m_pBuffer->put(data, 100);
  
  EQ(size_t(100), m_pBuffer->m_nBytesInBuffer);
  EQ((void*)(m_pBuffer->m_pBuffer+100), (void*)(m_pBuffer->m_pInsert));
}

// After a larger put, pointers are correct and there is a file
// that has the first bsize data bytes.

void testBufferedOutput:: put_2()
{
  uint8_t data[bsize+100];            // Putting this forces a flush.
  for (int i =0; i < sizeof(data); i++) {
    data[i] = i;                      // Set buffer to known pattern.
  }
  
  m_pBuffer->put(data, bsize+100);
  
  // Should be a partial buffer:
  
  EQ(size_t(100), m_pBuffer->m_nBytesInBuffer);
  EQ((void*)(m_pBuffer->m_pBuffer+100), (void*)(m_pBuffer->m_pInsert));
  
  // Should be a file with bsize bytes of data and that matches the pattern:
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  ASSERT(fd > 0);
  
  uint8_t readData[bsize+100];
  size_t nRead = io::readData(fd, readData, sizeof(readData));
  close(fd);
  
  EQ(bsize, nRead);           // Only one buffer of data in the file.
  for (int i = 0; i < nRead; i++) {
    EQ(data[i], readData[i]);
  }
  
  
}
// Flush empties the buffer and the file should have the data.

void testBufferedOutput::flush_1()
{
  ASSERT(size_t(100) < bsize);
  
  uint8_t data[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  m_pBuffer->put(data, 100);
  
  m_pBuffer->flush();
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  ASSERT(fd > 0);
  
  uint8_t readData[bsize];
  size_t nRead = io::readData(fd, readData, sizeof(readData));
  close(fd);
  
  EQ(sizeof(data), nRead);           // Only one buffer of data in the file.
  for (int i = 0; i < nRead; i++) {
    EQ(data[i], readData[i]);
  }  
}
// Destruction flushes:

void testBufferedOutput::flush_2()
{
  ASSERT(size_t(100) < bsize);
  
  uint8_t data[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  m_pBuffer->put(data, 100);
  
  delete m_pBuffer;                  // Implied flush.
  m_pBuffer = nullptr;               // So teardown doesn't barf on delete.
  
  int fd = open(m_filename.c_str(), O_RDONLY);
  ASSERT(fd > 0);
  
  uint8_t readData[bsize];
  size_t nRead = io::readData(fd, readData, sizeof(readData));
  close(fd);
  
  EQ(sizeof(data), nRead);           // Only one buffer of data in the file.
  for (int i = 0; i < nRead; i++) {
    EQ(data[i], readData[i]);
  }    
}