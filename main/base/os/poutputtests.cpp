// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CPagedOutput.h"
#include "io.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <stdio.h>
#include <stdint.h>

class PagedOTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PagedOTest);
  CPPUNIT_TEST(construct_1);
  CPPUNIT_TEST(construct_2);
  
  CPPUNIT_TEST(write_1);
  CPPUNIT_TEST(write_2);
  CPPUNIT_TEST(write_3);
  CPPUNIT_TEST(write_4);
  
  CPPUNIT_TEST(flush_1);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string m_name;
public:
  void setUp() {
    m_name = tmpnam(nullptr);      // Yeah I know but mkstemp etc. don't do what I need.
  }
  void tearDown() {
    unlink(m_name.c_str());
    m_name.clear();
  }
protected:
  void construct_1();
  void construct_2();
  
  void write_1();
  void write_2();
  void write_3();
  void write_4();
  
  void flush_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PagedOTest);

// Creating the object does not throw and actually makes the file;
// Buffer is a page:

void PagedOTest::construct_1()
{
  io::CPagedOutput* pTestObj;
  size_t pageSize = sysconf(_SC_PAGESIZE);
  
  CPPUNIT_ASSERT_NO_THROW(
    pTestObj = new io::CPagedOutput(m_name.c_str(), pageSize)
  );
  delete pTestObj;
  
  // Should be left with a zero length file.
  
  struct stat info;
  int status = stat(m_name.c_str(), &info);
  EQ(0, status);
  EQ(off_t(0), info.st_size);
}

// should be able to specify a buffersize that's not a page multiple:

void PagedOTest::construct_2()
{
  io::CPagedOutput* pTestObj;
  
  CPPUNIT_ASSERT_NO_THROW(
    pTestObj = new io::CPagedOutput(m_name.c_str(), 1)
  );
  delete pTestObj;
  
  // Should be left with a zero length file.
  
  struct stat info;
  int status = stat(m_name.c_str(), &info);
  EQ(0, status);
  EQ(off_t(0), info.st_size); 
}

// Write a single byte to the file:

void PagedOTest::write_1()
{
  io::CPagedOutput* pTestObj = new io::CPagedOutput(m_name.c_str(), 1);
  
  uint8_t byte = 0xaa;
  pTestObj->put(&byte, sizeof(byte));
  
  delete pTestObj;
  
  // Should have a byte sized file with the byte value 0xaa inside.
  
  struct stat info;
  int status = stat(m_name.c_str(), &info);
  EQ(0, status);
  EQ(off_t(1), info.st_size);
  
  int fd = open(m_name.c_str(), O_RDONLY);
  ASSERT(fd >= 0);
  
  uint8_t buf[256];
  size_t nRead = io::readData(fd, buf, sizeof(buf));
  close(fd);
  
  EQ(nRead, sizeof(byte));
  EQ(uint8_t(0xaa), buf[0]);
}
// Write  exactly a page of data:

void PagedOTest::write_2()
{
  
  size_t pageSize = sysconf(_SC_PAGESIZE);
  io::CPagedOutput* pTestObj = new io::CPagedOutput(m_name.c_str(), pageSize);
  
  uint8_t outblock[pageSize];
  for (int i = 0; i < pageSize; i++) {
    outblock[i] = i;
  }
  pTestObj->put(outblock, pageSize);
  delete pTestObj;
  
  int fd = open(m_name.c_str(), O_RDONLY);
  ASSERT(fd >= 0);
  
  uint8_t inblock[pageSize*2];
  size_t nRead = io::readData(fd, inblock, pageSize*2);
  close(fd);
  EQ(size_t(pageSize), nRead);
  
  for (int i =0; i < nRead; i++) {
    EQ(outblock[i], inblock[i]);
  }
}
// Write that spills over into the next block.

void PagedOTest::write_3()
{
  size_t pageSize = sysconf(_SC_PAGESIZE);
  io::CPagedOutput* pTestObj = new io::CPagedOutput(m_name.c_str(), pageSize);
  
  uint8_t outblock[pageSize+1];
  for (int i = 0; i < pageSize+1; i++) {
    outblock[i] = i;
  }
  
  pTestObj->put(outblock, pageSize+1);
  delete pTestObj;
  
  int fd = open(m_name.c_str(), O_RDONLY);
  ASSERT(fd >= 0);
  
  uint8_t inblock[pageSize*2];
  size_t nRead = io::readData(fd, inblock, pageSize*2);
  close(fd);
  EQ(size_t(pageSize+1), nRead);
  
  for (int i =0; i < nRead; i++) {
    EQ(outblock[i], inblock[i]);
  }
}
// Multiblock + remainder write.

void PagedOTest::write_4()
{
  size_t pageSize = sysconf(_SC_PAGESIZE);
  io::CPagedOutput* pTestObj = new io::CPagedOutput(m_name.c_str(), pageSize);
  
  size_t writeSize = pageSize*4 + pageSize/2;
  uint8_t obuf[writeSize];
  for (int i = 0; i < writeSize; i++) {
    obuf[i] = i;
  }
  pTestObj->put(obuf, writeSize);
  delete pTestObj;
  
  
  int fd = open(m_name.c_str(), O_RDONLY);
  ASSERT(fd >= 0);
  
  uint8_t inblock[writeSize*2];
  size_t nRead = io::readData(fd, inblock, writeSize*2);
  close(fd);
  EQ(size_t(writeSize), nRead);
  
  for (int i =0; i < nRead; i++) {
    EQ(obuf[i], inblock[i]);
  }
}
// If I write a byte, then flush I can read the file and it'll be a page
// long padded with nulls.

void PagedOTest::flush_1()
{
  size_t pageSize = sysconf(_SC_PAGESIZE);
  io::CPagedOutput* pTestObj = new io::CPagedOutput(m_name.c_str(), pageSize);

  uint8_t buf = 0x55;;
  pTestObj->put(&buf, sizeof(buf));
  pTestObj->flush();
  
  // Note that we can't delete the test obj yet as that'll truncate thefile
  // to a byte so:
  
  int fd = open(m_name.c_str(), O_RDONLY);
  ASSERT(fd >= 0);
  
  uint8_t ibuf[pageSize*2];
  size_t nRead = io::readData(fd, ibuf, pageSize*2);
  EQ(size_t(pageSize), nRead);
  EQ(buf, ibuf[0]);
  for(int i =1; i < nRead; i++) {
    EQ(uint8_t(0), ibuf[i]);
  }
  
}