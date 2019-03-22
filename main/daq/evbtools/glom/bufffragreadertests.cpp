// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <fragment.h>
#define private public
#include "CBufferedFragmentReader.h"
#undef private
#include <unistd.h>
#include <fcntl.h>


class bfragreadertest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(bfragreadertest);
  CPPUNIT_TEST(constest);
  
  CPPUNIT_TEST(mustread_1);
  CPPUNIT_TEST(mustread_2);
  CPPUNIT_TEST(mustread_3);
  CPPUNIT_TEST(mustread_4);
  CPPUNIT_TEST(mustread_5);
  
  CPPUNIT_TEST(readpointer_1);
  CPPUNIT_TEST(readpointer_2);
  CPPUNIT_TEST(readpointer_3);
  
  CPPUNIT_TEST(cursor_1);
  CPPUNIT_TEST(cursor_2);
  
  CPPUNIT_TEST(updoffset_1);
  
  CPPUNIT_TEST(fsize_1);
  
  CPPUNIT_TEST(readdata_1);
  CPPUNIT_TEST(readdata_2);
  
  CPPUNIT_TEST(fillbuffer_1);
  CPPUNIT_TEST(fillbuffer_2);
  
  CPPUNIT_TEST(getfrag_1);
  CPPUNIT_TEST(getfrag_2);
  CPPUNIT_TEST_SUITE_END();


private:
  int m_writeFd;
  int m_readFd;
  CBufferedFragmentReader* m_pTestObj;
public:
  void setUp() {
    int pipes[2];
    pipe(pipes);
    m_readFd = pipes[0];
    m_writeFd = pipes[1];
    m_pTestObj = new CBufferedFragmentReader(m_readFd);
  }
  void tearDown() {
    
    delete m_pTestObj;
    m_pTestObj = nullptr;
    close(m_writeFd);
    close(m_readFd);
    
  }
protected:
  void constest();
  
  void mustread_1();
  void mustread_2();
  void mustread_3();
  void mustread_4();
  void mustread_5();
  
  void readpointer_1();
  void readpointer_2();
  void readpointer_3();
  
  void cursor_1();
  void cursor_2();
  
  void updoffset_1();
  
  void fsize_1();
  
  void readdata_1();
  void readdata_2();
  
  void fillbuffer_1();
  void fillbuffer_2();
  
  void getfrag_1();
  void getfrag_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(bfragreadertest);

void bfragreadertest::constest() {
  EQ(m_readFd, m_pTestObj->m_nFd);
  EQ(fcntl(m_readFd, F_GETPIPE_SZ), int(m_pTestObj->m_nBufferSize));
  EQ(size_t(0), m_pTestObj->m_nBytesInBuffer);
  EQ(size_t(0), m_pTestObj->m_nOffset);
  EQ(m_pTestObj->m_nBufferSize, m_pTestObj->m_nReadSize);
}

void bfragreadertest::mustread_1()   // must start out needing to read:
{
  ASSERT(m_pTestObj->mustRead());
}
void bfragreadertest::mustread_2()   // Just after read e.g.
{
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  ASSERT(!m_pTestObj->mustRead());
}
void bfragreadertest::mustread_3()   // Must read if less than a frag header:
{
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset        = m_pTestObj->m_nBytesInBuffer - sizeof(EVB::FragmentHeader)/2;
  ASSERT(m_pTestObj->mustRead());
}
void bfragreadertest::mustread_4()
{
  // fragment header but not enough left for the body:
  
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset        = m_pTestObj->m_nBytesInBuffer - sizeof(EVB::FragmentHeader);
  
  // We've not yet tested cursor so:
  
  uint8_t* pBytes = static_cast<uint8_t*>(m_pTestObj->m_pBuffer);
  pBytes += m_pTestObj->m_nOffset;
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(pBytes);
  pHdr->s_size = 128;
  
  ASSERT(m_pTestObj->mustRead());
}

void bfragreadertest::mustread_5() // full fragment is available (false).
{
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset =
    m_pTestObj->m_nBytesInBuffer - sizeof(EVB::FragmentHeader) - 128;
  uint8_t* pBytes = static_cast<uint8_t*>(m_pTestObj->m_pBuffer);
  pBytes += m_pTestObj->m_nOffset;
  EVB::pFragmentHeader pHdr = reinterpret_cast<EVB::pFragmentHeader>(pBytes);
  pHdr->s_size = 128;
  
  ASSERT(!m_pTestObj->mustRead());
}

void bfragreadertest::readpointer_1() // Initial condition: -> m_pBuffer
{
  void* p = m_pTestObj->readPointer();
  EQ(m_pTestObj->m_pBuffer, p);
  EQ(m_pTestObj->m_nBufferSize, m_pTestObj->m_nReadSize);
  EQ(size_t(0), m_pTestObj->m_nOffset);
  EQ(size_t(0), m_pTestObj->m_nBytesInBuffer);
}
void bfragreadertest::readpointer_2() // completely used up - -> m_pBuffer.
{
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset       = m_pTestObj->m_nBufferSize;
  
  void* p = m_pTestObj->readPointer();

  EQ(m_pTestObj->m_pBuffer, p);
  EQ(m_pTestObj->m_nBufferSize, m_pTestObj->m_nReadSize);
  EQ(size_t(0), m_pTestObj->m_nOffset);
  EQ(size_t(0), m_pTestObj->m_nBytesInBuffer);

}

void bfragreadertest::readpointer_3()   // handle remainder.
{
  size_t remainder = 12;
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset        = m_pTestObj->m_nBufferSize - remainder;
  
  void* p = m_pTestObj->readPointer();
  uint8_t* pExpected = static_cast<uint8_t*>(m_pTestObj->m_pBuffer);
  pExpected += remainder;
  
  EQ((void*)(pExpected), p);
  EQ((m_pTestObj->m_nBufferSize - remainder), m_pTestObj->m_nReadSize);
  EQ(size_t(0), m_pTestObj->m_nOffset);
  EQ(remainder, m_pTestObj->m_nBytesInBuffer);
  
}

void bfragreadertest::cursor_1()   // initially m_pBuffer.
{
  EQ((const EVB::pFlatFragment)(m_pTestObj->m_pBuffer), m_pTestObj->cursor());
}
void bfragreadertest::cursor_2()  // With an offset:
{
  size_t offset = 128;
  m_pTestObj->m_nOffset = offset;
  EVB::pFlatFragment p = m_pTestObj->cursor();
  
  uint8_t* pExpected = static_cast<uint8_t*>(m_pTestObj->m_pBuffer);
  pExpected += offset;
  
  EQ((EVB::pFlatFragment)(pExpected), p);
}
void bfragreadertest::updoffset_1()
{
  size_t s = 100;
  EVB::pFragmentHeader pF = static_cast<EVB::pFragmentHeader>(m_pTestObj->m_pBuffer);
  pF->s_size = s;
  
  EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pF);
  m_pTestObj->updateOffset(pFrag);
  
  EQ(s + sizeof(EVB::FragmentHeader), m_pTestObj->m_nOffset);
}
void bfragreadertest::fsize_1()
{
  size_t s = 100;
  EVB::FlatFragment f;
  f.s_header.s_size = s;
  size_t n = m_pTestObj->fragSize(&f);
  
  EQ(s + sizeof(EVB::FragmentHeader), n);
}

void bfragreadertest::readdata_1()
{
  // Write some junk to the pipe:
  
 uint8_t junk[100];
 write(m_writeFd, junk, 100);
 m_pTestObj->readData();
 
 // Should have 100 bytes.
 
 EQ(size_t(100), m_pTestObj->m_nBytesInBuffer);
 EQ(size_t(0),   m_pTestObj->m_nOffset);
  
}
void bfragreadertest::readdata_2()   // read with partial data:
{
  uint8_t junk[100];
  write(m_writeFd, junk, 100);
  
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset        = m_pTestObj->m_nBytesInBuffer- sizeof(EVB::FragmentHeader);
  
  m_pTestObj->readData();
  EQ(size_t(100) + sizeof(EVB::FragmentHeader), m_pTestObj->m_nBytesInBuffer);
  EQ(size_t(0), m_pTestObj->m_nOffset);
}
void bfragreadertest::fillbuffer_1()
{
  uint8_t junk[100];
  write(m_writeFd, junk, 100);
  m_pTestObj->fillBuffer();
  
  EQ(size_t(100),m_pTestObj->m_nBytesInBuffer);
  
}
void bfragreadertest::fillbuffer_2()
{
  uint8_t junk[100];
  write(m_writeFd, junk, 100);
  
  m_pTestObj->m_nBytesInBuffer = m_pTestObj->m_nBufferSize;
  m_pTestObj->m_nOffset        = m_pTestObj->m_nBytesInBuffer- sizeof(EVB::FragmentHeader);
  
  m_pTestObj->fillBuffer();
  EQ(size_t(100) + sizeof(EVB::FragmentHeader), m_pTestObj->m_nBytesInBuffer);
  EQ(size_t(0), m_pTestObj->m_nOffset);  
}
void bfragreadertest::getfrag_1()
{
  // push a couple of fragments into the pipe and see that we get them back out.
  
  size_t bodySize = 16;
  uint8_t buffer[500];
  EVB::pFragmentHeader p = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  p->s_size = bodySize;
  p->s_timestamp = 1;
  
  write(m_writeFd, p, sizeof(EVB::FragmentHeader) + bodySize);
  p->s_timestamp = 2;
  write(m_writeFd, p, sizeof(EVB::FragmentHeader) + bodySize);
  
  const EVB::pFlatFragment f1 = m_pTestObj->getFragment();
  const EVB::pFlatFragment f2 = m_pTestObj->getFragment();
  
  EQ(uint64_t(1), f1->s_header.s_timestamp);
  EQ(uint64_t(2), f2->s_header.s_timestamp);
  EQ(uint32_t(bodySize), f1->s_header.s_size);
  EQ(uint32_t(bodySize), f2->s_header.s_size);

  
}
void bfragreadertest::getfrag_2()  // partial + read to fulfil.
{
  uint8_t* p = static_cast<uint8_t*>(m_pTestObj->m_pBuffer);
  p += 2;
  m_pTestObj->m_nOffset = 2;
  m_pTestObj->m_nBytesInBuffer = 2 + sizeof(EVB::FragmentHeader);
  
  // Put the fragment header for the body that follows at p:
  
  EVB::pFragmentHeader pHdr1 = reinterpret_cast<EVB::pFragmentHeader>(p);
  pHdr1->s_timestamp = 0x5555aaaaffff7777;
  pHdr1->s_size      = 100;
  
  uint8_t buffer[100];
  write(m_writeFd, buffer, 100);
  
  EVB::pFragmentHeader pHdr2 = reinterpret_cast<EVB::pFragmentHeader>(buffer);
  pHdr2->s_timestamp = 0xaaaa55557777ffff;
  pHdr2->s_size      = 10;
  write(m_writeFd, pHdr2, sizeof(EVB::FragmentHeader) + 10);
  
  const EVB::pFlatFragment f1 = m_pTestObj->getFragment();
  const EVB::pFlatFragment f2 = m_pTestObj->getFragment();
  
  EQ(uint64_t(0x5555aaaaffff7777), f1->s_header.s_timestamp);
  EQ(uint32_t(100), f1->s_header.s_size);
  
  EQ(uint64_t(0xaaaa55557777ffff), f2->s_header.s_timestamp);
  EQ(uint32_t(10), f2->s_header.s_size);
}