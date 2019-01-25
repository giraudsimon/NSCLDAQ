// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <CRingBlockReader.h>
#include <CPhysicsEventItem.h>
#include <DataFormat.h>

#include "swFilterRingBlockDataSource.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


// A fake block data source:

class CFakeRingBlockReader : public CRingBlockReader
{
private:
  void*  m_pData;
  
  size_t m_nTotalBytes;
  size_t m_nBytesRead;
  

  void*  m_pRead;
public:
  CFakeRingBlockReader();
  virtual ~CFakeRingBlockReader() {free(m_pData);}
  
  void addData(void* pBytes, size_t nBytes);

protected:
  virtual ssize_t readBlock(void* pBuffer, size_t nBytes);
  
};

/// The fake blovck reader data source implementation.

CFakeRingBlockReader::CFakeRingBlockReader() :
  m_pData(nullptr), m_nTotalBytes(0), m_nBytesRead(0),
  m_pRead(nullptr)
{}

// Put data into the reader so that readBlock can get it out.

void
CFakeRingBlockReader::addData(void* pBytes, size_t nBytes)
{
  m_pData = realloc(m_pData, m_nTotalBytes + nBytes);
  
  // COpy the new data in:
  
  uint8_t* pWriteCursor = static_cast<uint8_t*>(m_pData);
  pWriteCursor += m_nTotalBytes;
  memcpy(pWriteCursor, pBytes, nBytes);
  
  m_nTotalBytes += nBytes;
  
  // Now set the new read pointer as the base address may have been
  // modified:
  
  uint8_t* pRead = static_cast<uint8_t*>(m_pData);
  pRead += m_nBytesRead;
  m_pRead = pRead;
  
  
}
// 'Read' data from the test patterns

ssize_t
CFakeRingBlockReader::readBlock(void* pBuffer, size_t nBytes)
{
  ssize_t result  = 0;
  size_t bytesLeft = m_nTotalBytes - m_nBytesRead;
  if (bytesLeft) {
    nBytes =  nBytes <= bytesLeft ? nBytes : bytesLeft; // Don't over read.
    memcpy(pBuffer, m_pRead, nBytes);
    result = nBytes;
    
    // Do the book keeping
  
    m_nBytesRead += nBytes;
    uint8_t*   p  = static_cast<uint8_t*>(m_pRead);
    m_pRead       = p + nBytes;
  }

  return result;
}


class blockdsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(blockdsTest);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(oneitem);
  CPPUNIT_TEST_SUITE_END();


private:
  CFakeRingBlockReader*        m_pReader;
  swFilterRingBlockDataSource* m_pTestObj;
public:
  void setUp() {
    m_pReader = new CFakeRingBlockReader;
    m_pTestObj = new swFilterRingBlockDataSource(*m_pReader, 128);
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pReader;
    
    m_pTestObj = nullptr;
    m_pReader  = nullptr;
  }
protected:
  void empty();
  void oneitem();
};

CPPUNIT_TEST_SUITE_REGISTRATION(blockdsTest);

// If there's no data to read we get {0, nullptr]}
void blockdsTest::empty() {
  
  std::pair<std::size_t, void*> result = m_pTestObj->read();
  
  EQ(std::size_t(0), result.first);
  EQ((void*)(nullptr), result.second);
}
// Put a small item ( < 128 bytes) in the block buffer.
void blockdsTest::oneitem()
{
  CPhysicsEventItem item(0x1234, 1, 0);
  uint32_t* p = static_cast<uint32_t*>(item.getBodyCursor());
  *p++ = 0;
  item.setBodyCursor(p);
  pRingItem pItem = item.getItemPointer();
  item.updateSize();
  m_pReader->addData(pItem, pItem->s_header.s_size);
  
  std::pair<std::size_t, void*> result = m_pTestObj->read();
  EQ(sizeof(CRingBlockReader::DataDescriptor), result.first);
  CRingBlockReader::pDataDescriptor pI =
    static_cast<CRingBlockReader::pDataDescriptor>(result.second);
  ASSERT(pI);
  
  EQ(uint32_t(1), pI->s_nItems);
  EQ(pItem->s_header.s_size, pI->s_nBytes);
  pRingItem pReadItem = static_cast<pRingItem>(pI->s_pData);
  
  EQ(pItem->s_header.s_size, pReadItem->s_header.s_size);
  EQ(pItem->s_header.s_type, pReadItem->s_header.s_type);
  EQ(
    uint32_t(sizeof(BodyHeader)),
    pReadItem->s_body.u_hasBodyHeader.s_bodyHeader.s_size
  );
  EQ(
     pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp,
     pReadItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp
  );
  EQ(
    pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp,
    pReadItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp 
  );
  EQ(
    pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId,
    pReadItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId
  );
  EQ(
    pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier,
    pReadItem->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier
  );
  uint32_t* pD = reinterpret_cast<uint32_t*>(
    (pItem->s_body.u_hasBodyHeader.s_body)
  );
  uint32_t* pR =  reinterpret_cast<uint32_t*>(
    (pItem->s_body.u_hasBodyHeader.s_body)
  );
  EQ(*pD, *pR );
}