// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CRingItemFileTransport.h"
#include <CRingBlockReader.h>
#include <CBufferedOutput.h>
#include <DataFormat.h>
#include <CFileDataSource.h>
#include <CRingItem.h>
#include <URL.h>

#include <unistd.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <stdint.h>
#include <vector>
#include <stdexcept>

std::string nameTemplate="ringXXXXXX";

class rtransportTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rtransportTest);
  CPPUNIT_TEST(write_1);
  CPPUNIT_TEST(write_2);      // Write a few ring items.
  CPPUNIT_TEST(write_3);      // Read from m_pWTestObje should throw.
  
  CPPUNIT_TEST(read_1);      // Now that we know we can write...
  CPPUNIT_TEST(read_2);
  CPPUNIT_TEST(read_3);
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
  void write_1();
  void write_2();
  void write_3();
  
  void read_1();
  void read_2();
  void read_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rtransportTest);

void rtransportTest::write_1() {   // Write a ring itme to file, can it be read?
  // The itemwe write is just a  header:
  
  RingItemHeader hdr;
  hdr.s_size = sizeof(hdr);
  hdr.s_type = PHYSICS_EVENT;               // good as any.
  
  iovec v;
  v.iov_base = &hdr;
  v.iov_len  = hdr.s_size;
  m_pWTestObj->send(&v, 1);

  m_pWriter->sync();
  
  //Assume the file is created here:
  
  std::string fnameUri = "file://./";
  fnameUri += m_fileName;
  URL srcUri(fnameUri);
  std::vector<uint16_t> empty;
  CFileDataSource src(srcUri, empty);
  
  CRingItem* pItem = src.getItem();
  EQ(PHYSICS_EVENT, pItem->type());
  EQ(
     uint32_t(sizeof(RingItemHeader)),
     itemSize(pItem->getItemPointer())
  );
  
  delete pItem;
  pItem = src.getItem();
  EQ((CRingItem*)(nullptr), pItem);
}
void rtransportTest::write_2()
{
  uint8_t ringItems[1024];
  ASSERT(sizeof(ringItems) > 10*(sizeof(RingItemHeader) + 2* sizeof(uint32_t)));
  
  iovec v[10];                // For writing ring items.
  pRingItemHeader pItem = reinterpret_cast<pRingItemHeader>(ringItems);
  
  for (int i = 0; i < 10; i++) {
    pItem->s_type = PHYSICS_EVENT;
    pItem->s_size = sizeof(RingItemHeader) + 2*sizeof(uint32_t);
    uint32_t* pBody = reinterpret_cast<uint32_t*>(pItem + 1);
    *pBody++ = 0;                            // no body header.
    *pBody++ = i;                            // body
    
    v[i].iov_base = pItem;
    v[i].iov_len  = pItem->s_size;
    
    pItem = reinterpret_cast<pRingItemHeader>(pBody);
  }
  // Write-em.
  m_pWTestObj->send(v, 10);
  m_pWriter->sync();                   // Make sure it's to file.
  
  //Assume the file is created here:
  
  std::string fnameUri = "file://./";
  fnameUri += m_fileName;
  URL srcUri(fnameUri);
  std::vector<uint16_t> empty;
  CFileDataSource src(srcUri, empty);

  // Read and check the 10 items.. that should empty the source out.
  // We'll assume sizes are ok and just check type and body.
  
  for (int i = 0; i < 10; i++) {
    CRingItem* pI = src.getItem();
    EQ(PHYSICS_EVENT, pI->type());
    uint32_t* p = static_cast<uint32_t*>(pI->getBodyPointer());
    EQ(uint32_t(i), p[0]);                 // -- because getBodyPointer knows mbz.
    
    delete pI;
  }
  CRingItem* pI = src.getItem();
  EQ((CRingItem*)(nullptr), pI);
  
}
void rtransportTest::write_3()
{
  void* pData;
  size_t n;
  CPPUNIT_ASSERT_THROW(
    m_pWTestObj->recv(&pData, n),
    std::runtime_error
  );
}

void rtransportTest::read_1()
{
  
  // Write an empty ring item:
  
  RingItemHeader hdr;
  hdr.s_size = sizeof(hdr);
  hdr.s_type = PHYSICS_EVENT;
  iovec v;
  v.iov_base = &hdr;
  v.iov_len  = hdr.s_size;
  m_pWTestObj->send(&v, 1);
  m_pWriter->sync();
  usleep(1000);                     // Evidently takes time to get to disk!
  
  void* pData(0);
  size_t n;
  m_pRTestObj->recv(&pData, n);
  ASSERT(pData);
  EQ(sizeof(hdr), n);
  pRingItem p = reinterpret_cast<pRingItem>(pData);
  EQ(hdr.s_size, itemSize(p));
  EQ(uint16_t(hdr.s_type), itemType(p));
  
  free(pData);
}
void rtransportTest::read_2()
{
  uint8_t ringItems[1024];
  ASSERT(sizeof(ringItems) > 10*(sizeof(RingItemHeader) + 2* sizeof(uint32_t)));
  
  iovec v[10];                // For writing ring items.
  pRingItemHeader pItem = reinterpret_cast<pRingItemHeader>(ringItems);
  
  for (int i = 0; i < 10; i++) {
    pItem->s_type = PHYSICS_EVENT;
    pItem->s_size = sizeof(RingItemHeader) + 2*sizeof(uint32_t);
    uint32_t* pBody = reinterpret_cast<uint32_t*>(pItem + 1);
    *pBody++ = 0;                            // no body header.
    *pBody++ = i;                            // body
    
    v[i].iov_base = pItem;
    v[i].iov_len  = pItem->s_size;
    
    pItem = reinterpret_cast<pRingItemHeader>(pBody);
  }
  // Write-em.
  m_pWTestObj->send(v, 10);
  m_pWriter->sync();                   // Force out.
  
  usleep(1000);
  
  for (int i =0; i < 10; i++) {
    void* pData;
    size_t n;
    m_pRTestObj->recv(&pData, n);
    pRingItem item = reinterpret_cast<pRingItem>(pData);
    EQ(uint32_t(sizeof(RingItemHeader) + 2*sizeof(uint32_t)), itemSize(item));
    EQ(uint16_t(PHYSICS_EVENT), itemType(item));
    
    ASSERT(!bodyHeader(item));
    uint32_t* p = static_cast<uint32_t*>(bodyPointer(item));
    EQ(uint32_t(i), *p);            // body.
    
    free(pData);
  }
  void* pData;
  size_t n;
  m_pRTestObj->recv(&pData, n);
  EQ(size_t(0), n);
}
void rtransportTest::read_3()
{
  iovec v;
  v.iov_base=nullptr;
  v.iov_len = 0;
  CPPUNIT_ASSERT_THROW(
    m_pRTestObj->send(&v, 1),
    std::runtime_error
  );
}