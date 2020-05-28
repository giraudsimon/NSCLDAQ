// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CEvents2Fragments.h"
#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>
#include "CFragmentMaker.h"

#include <DataFormat.h>
#include <fragment.h>

#include <list>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
// We need some mock classes for the reader and the writer.

// The reader will allow the test system to queue up a set of data descriptors
// which read delivers until they're exhausted.

class CMockRingFileBlockReader : public CRingFileBlockReader
{
private:
  std::list<CRingBlockReader::DataDescriptor> m_data;
public:
  CMockRingFileBlockReader() :
    CRingFileBlockReader(dup(STDIN_FILENO)) {} // dup because destruction closes.
  virtual ~CMockRingFileBlockReader();
  
  virtual CRingBlockReader::DataDescriptor read(size_t nBytes);
  void addData(CRingBlockReader::DataDescriptor desc) {
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
CRingBlockReader::DataDescriptor
CMockRingFileBlockReader::read(size_t nBytes) // we ignore the buffer size
{
  CRingBlockReader::DataDescriptor result;
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

class apptests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(apptests);
  // The mocks are complex enough they need tests of their own:
  
  // block reader mock tests
  
  CPPUNIT_TEST(breaderEmpty);
  CPPUNIT_TEST(breader_1Item);
  CPPUNIT_TEST(breader_Order);
  
  // buffered writer mock tests.
  
  CPPUNIT_TEST(writerEmpty);
  CPPUNIT_TEST(writer_1);            // An item written comes out correctly.
  CPPUNIT_TEST(writer_ordered);      // Data comes out in order it's written.
  
  // Application class tests.
  
  CPPUNIT_TEST(fragmaker_eof);        // Immediate eof.
  CPPUNIT_TEST(fragmaker_begonly_1);    // Only a begin run.
  CPPUNIT_TEST(fragmaker_begonly_2);
  
  CPPUNIT_TEST(fragmaker_complete_1);
  CPPUNIT_TEST(fragmaker_complete_2);  
  CPPUNIT_TEST_SUITE_END();


private:
  CMockRingFileBlockReader* m_pReader;
  MockBufferedOutput*      m_pWriter;
  CFragmentMaker*           m_pFragMaker;
  CEvents2Fragments*        m_pTestObj;
public:
  void setUp() {
    m_pReader = new CMockRingFileBlockReader;
    m_pWriter = new MockBufferedOutput;
    m_pFragMaker = new CFragmentMaker(123);
    m_pTestObj = new CEvents2Fragments(8192, *m_pReader, *m_pFragMaker, *m_pWriter);
  }
  void tearDown() {
    delete m_pTestObj;
    delete m_pReader;
    delete m_pWriter;
    delete m_pFragMaker;
  }
protected:
  void breaderEmpty();
  void breader_1Item();
  void breader_Order();
  
  void writerEmpty();
  void writer_1();
  void writer_ordered();
  
  void fragmaker_eof();
  
  void fragmaker_begonly_1();
  void fragmaker_begonly_2();
  
  void fragmaker_complete_1();   // Begin and end -- both have body headers.
  void fragmaker_complete_2();   // Begin with body header, end without.
private:
  CRingBlockReader::DataDescriptor makeCountingPattern(
    int nBytes, int nItems
  );
  pRingItem makeStateChange(uint64_t ts, uint32_t sid, uint32_t type);
  pRingItem makeStateChange(uint32_t type);
  CRingBlockReader::DataDescriptor packItems(std::vector<pRingItem> items);
};

CPPUNIT_TEST_SUITE_REGISTRATION(apptests);

///////////////////////////////////////////////////////////////////////////
// Utilities:
//

// pack ring items into a single block.
// prior items are freed, the return block was malloced.
//
CRingBlockReader::DataDescriptor
apptests::packItems(std::vector<pRingItem> items)
{
  CRingBlockReader::DataDescriptor result = {0, 0, nullptr};
  
  result.s_nItems =  items.size();
  result.s_nBytes =  0;
  result.s_pData  =  nullptr;
  
  // Now figure out the total size:
  
  for (int i = 0; i < items.size(); i++) {
    result.s_nBytes += itemSize(items[i]);
  }
  
  result.s_pData = malloc(result.s_nBytes);
  
  // Copy the items into the new data block.
  
  uint8_t* p = static_cast<uint8_t*>(result.s_pData);
  for (int i =0; i < items.size();  i++) {
    size_t n = itemSize(items[i]);
    memcpy(p, items[i], n);
    p += n;
    free(items[i]);
  }
  
  return result;
}

// Make a state change item with a body header.
//  ts - timestamp, sid - sourceid, type  item type 
//   pRingItem is returned that points to the dynamically allocated ring item.

pRingItem
apptests::makeStateChange(uint64_t ts, uint32_t sid, uint32_t type)
{
  // cmopute the size of the item (fixed body helps).
  // Ok not to check for body header extension since we've contrived these data.
  
  size_t itemSize =
    sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(StateChangeItemBody);
  pStateChangeItem p = static_cast<pStateChangeItem>(malloc(itemSize));
  
  p->s_header.s_size = itemSize;
  p->s_header.s_type = type;
  
  // Cant' use bodyHeader method -- it'll return nullptr.
  // since there's no body header .. yet.
  
  pBodyHeader pB = &(p->s_body.u_hasBodyHeader.s_bodyHeader);
  pB->s_size = sizeof(BodyHeader);
  pB->s_timestamp= ts;
  pB->s_sourceId = sid;
  
  if (type == BEGIN_RUN) {
    pB->s_barrier = 1;
  } else if (type == END_RUN) {
    pB->s_barrier = 2;
  } else {
    pB->s_barrier = 0;
  }
  
  // for these tests we don't actually need to fil in the body.
  
  return reinterpret_cast<pRingItem>(p);
}
// Make a state change item that has no body header.
//  type - item type  returns a pointer to the item.

pRingItem
apptests::makeStateChange(uint32_t type)
{
  size_t itemSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(StateChangeItemBody);
    
  pStateChangeItem p = static_cast<pStateChangeItem>(malloc(itemSize));
  p->s_header.s_size = itemSize;
  p->s_header.s_type = type;
  
  p->s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);
  
  return reinterpret_cast<pRingItem>(p);
}

CRingBlockReader::DataDescriptor
apptests::makeCountingPattern(int nBytes, int nItems)
{
  CRingBlockReader::DataDescriptor result;
  result.s_nItems = nItems;
  result.s_nBytes = nBytes;
  result.s_pData  = malloc(nBytes);
  uint8_t* p = static_cast<uint8_t*>(result.s_pData);
  ASSERT(p);
  for (int i =0; i < nBytes; i++) {
    *p++ = static_cast<uint8_t>(i & 0xff);
  }
  
  return result;
}

///////////////////////////////////////////////////////////////////////////
// Tests

// If there's nothing in the block reader it immediately reads an empty.
void apptests::breaderEmpty() {
  CRingBlockReader::DataDescriptor d = m_pReader->read(100);
  EQ(uint32_t(0), d.s_nBytes);
  EQ(uint32_t(0), d.s_nItems);
  EQ((void*)(nullptr), d.s_pData);
  
}

// if an item is put in it should come back out and then eof:

void apptests::breader_1Item()
{
  CRingBlockReader::DataDescriptor data = {100, 2, malloc(100)};
  m_pReader->addData(data);
  
  CRingBlockReader::DataDescriptor rdata = m_pReader->read(100);
  EQ(data.s_nBytes, rdata.s_nBytes);
  EQ(data.s_nItems, rdata.s_nItems);
  EQ(data.s_pData, rdata.s_pData);
  
  free(rdata.s_pData);               // no leaks here.
}

// Items put in come out in order.

void apptests::breader_Order()
{
  // Data goes in and comes out in the same order.
  
  m_pReader->addData(makeCountingPattern(100, 10));
  m_pReader->addData(makeCountingPattern(200, 20));
  m_pReader->addData(makeCountingPattern(300, 30));
  
  // should get the three in order:
  
  CRingBlockReader::DataDescriptor d;
  
  d  = m_pReader->read(2000);
  EQ(uint32_t(100), d.s_nBytes);
  EQ(uint32_t(10),  d.s_nItems);
  free(d.s_pData);
  
  d = m_pReader->read(1000);
  EQ(uint32_t(200), d.s_nBytes);
  EQ(uint32_t(20), d.s_nItems);
  free(d.s_pData);
  
  d = m_pReader->read(1234);
  EQ(uint32_t(300), d.s_nBytes);
  EQ(uint32_t(30), d.s_nItems);
  free(d.s_pData);
  
  // That was the last item:
  
  d = m_pReader->read(4567);
  EQ(uint32_t(0), d.s_nBytes);
  EQ(uint32_t(0), d.s_nItems);
  EQ((void*)(nullptr), d.s_pData);
}

//  If the writer hasn't been given anything it's empty:

void apptests::writerEmpty()
{
  std::list<std::pair<size_t, void*>> l = m_pWriter->getWrittenData();
  ASSERT(l.empty());
  
  std::pair<size_t, void*> d = m_pWriter->get();
  EQ(size_t(0), d.first);
  EQ((void*)(nullptr), d.second);
  
}
//  If I write an item to the writer it will come back correctly.

void apptests::writer_1()
{
  uint32_t buffer[1024];
  for (int i = 0; i < 1024; i++) {
    buffer[i] = i;
  }
  // Writer has one item and it's correct:
  m_pWriter->put(buffer, sizeof(buffer));
  
  std::list<std::pair<size_t, void*>> l = m_pWriter->getWrittenData();
  EQ(size_t(1), l.size());
  std::pair<size_t, void*> d1 = l.front();
  EQ(sizeof(buffer), d1.first);
  uint32_t* p = static_cast<uint32_t*>(d1.second);
  for (int i =0; i < 1024; i++) {
    EQ(buffer[i], p[i]);
  }
  
  // we can get one item out and its correct:
  
  d1 = m_pWriter->get();
  EQ(sizeof(buffer), d1.first);
  p = static_cast<uint32_t*>(d1.second);
  for (int i =0; i < 1024; i++) {
    EQ(buffer[i], p[i]);
  }
  free(p);                       // This is dynamic.
  
  // we can't get a second one out,
  
  d1 = m_pWriter->get();
  EQ(size_t(0), d1.first);
  EQ((void*)(nullptr), d1.second);
  
  
}
// Ensure ordering.

void apptests::writer_ordered()
{
  uint32_t buffer[1024];
  for (int i =0; i < 1024; i++) {
    buffer[i] = i;
  }
  m_pWriter->put(buffer, 100);
  m_pWriter->put(buffer, 200);
  m_pWriter->put(buffer, sizeof(buffer));
  
  std::pair<size_t, void*> d = m_pWriter->get();
  EQ(size_t(100), d.first);
  free(d.second);
  
  d = m_pWriter->get();
  EQ(size_t(200), d.first);
  free(d.second);
  
  d = m_pWriter->get();
  EQ(sizeof(buffer), d.first);
  free(d.second);
  
  d = m_pWriter->get();
  EQ(size_t(0), d.first);
  EQ((void*)(nullptr), d.second);
}
// If no data are on the input 'file' no data will be written to the
// output file.

void apptests::fragmaker_eof()
{
  (*m_pTestObj)();              // Even returning  is a victory ;-)
  
  std::list<std::pair<size_t, void*>> l = m_pWriter->getWrittenData();
  EQ(size_t(0), l.size());
}
// A begin run (only) with a body header.

void apptests::fragmaker_begonly_1()
{
  pRingItem pBegin = makeStateChange(0x12345, 2, BEGIN_RUN);
  uint32_t  sBegin = itemSize(pBegin);
  CRingBlockReader::DataDescriptor d =
      {sBegin, 1, pBegin};
    
  
  m_pReader->addData(d);
  
  (*m_pTestObj)();
  
  // Should be two gettable items.
  
  EQ(size_t(2), m_pWriter->getWrittenData().size());
  
  // first read gives us the fragment header:
  
  std::pair<size_t, void*> h = m_pWriter->get();
  EQ(sizeof(EVB::FragmentHeader), h.first);
  EVB::pFragmentHeader pH = static_cast<EVB::pFragmentHeader>(h.second);
  
  EQ(uint64_t(0x12345), pH->s_timestamp);
  EQ(uint32_t(2), pH->s_sourceId);
  EQ(uint32_t(1), pH->s_barrier);
  
  // The second item is the ring item itslef.
  
  std::pair<size_t, void*> i = m_pWriter->get();
  pRingItem pItem = static_cast<pRingItem>(i.second);
  
  EQ(pH->s_size, itemSize(pItem));
  EQ(size_t(itemSize(pItem)), i.first);
  EQ(uint16_t(BEGIN_RUN), itemType(pItem));
  
  free(h.second);
  free(i.second);
  
  // There should be one end run needed:
  
  EQ(1, m_pFragMaker->getEndRunsRemaining());
}
// Begin run only -- with no body header.

void apptests::fragmaker_begonly_2()
{
  pRingItem pBegin = makeStateChange(BEGIN_RUN);
  uint32_t  sBegin = itemSize(pBegin);
  CRingBlockReader::DataDescriptor d =
    {sBegin, 1, pBegin};
  
  m_pReader->addData(d);
  
  (*m_pTestObj)();
  
  // Should be two gettable items.
  
  EQ(size_t(2), m_pWriter->getWrittenData().size());
  
  // Fragment header should have the most recent timestamp and
  // default source id...and proper barrier type:
  
  std::pair<size_t, void*> h = m_pWriter->get();
  EQ(sizeof(EVB::FragmentHeader), h.first);
  EVB::pFragmentHeader pH = reinterpret_cast<EVB::pFragmentHeader>(h.second);
  EQ(m_pFragMaker->getLastTimestamp(), pH->s_timestamp);
  EQ(uint32_t(123), pH->s_sourceId);
  EQ(uint32_t(1), pH->s_barrier);
  
  // Second is the ring item.. our payload should match that.
  
  std::pair<size_t, void*> i = m_pWriter->get();
  pStateChangeItem pItem = static_cast<pStateChangeItem>(i.second);
  EQ(size_t(itemSize(reinterpret_cast<pRingItem>(pItem))), i.first);
  EQ(pH->s_size, itemSize(reinterpret_cast<pRingItem>(pItem)));
  EQ(BEGIN_RUN, uint32_t(itemType(reinterpret_cast<pRingItem>(pItem))));
  
  free(h.second);
  free(i.second);
  
  EQ(1, m_pFragMaker->getEndRunsRemaining());
}
// Begin and end run, both with body headers.

void apptests::fragmaker_complete_1()
{
  pRingItem pBegin =
    reinterpret_cast<pRingItem>(makeStateChange(0x100, 1, BEGIN_RUN));
  pRingItem pEnd   =
    reinterpret_cast<pRingItem>(makeStateChange(0x50000, 1, END_RUN));
    
  CRingBlockReader::DataDescriptor d = packItems({pBegin, pEnd});
  m_pReader->addData(d);
  
  (*m_pTestObj)();
  
  EQ(0, m_pFragMaker->getEndRunsRemaining());
  EQ(size_t(4), m_pWriter->getWrittenData().size());  // two ring items & headers.
  EQ(uint64_t(0x50000), m_pFragMaker->getLastTimestamp());
  
  // We'll be happy if the fragment headers are ok.  We trust our tests of
  // the fragment maker.
  
  std::pair<size_t, void*> beginHeader = m_pWriter->get();
  std::pair<size_t, void*> beginItem   = m_pWriter->get();
  
  std::pair<size_t, void*> endHeader   = m_pWriter->get();
  std::pair<size_t, void*> endItem     = m_pWriter->get();
  
  EVB::FragmentHeader* pBegHdr =
    static_cast<EVB::FragmentHeader*>(beginHeader.second);
  EQ(uint64_t(0x100), pBegHdr->s_timestamp);
  EQ(uint32_t(1), pBegHdr->s_sourceId);
  EQ(uint32_t(1), pBegHdr->s_barrier);
  
  EVB::FragmentHeader* pEndHdr =
    static_cast<EVB::FragmentHeader*>(endHeader.second);
  EQ(uint64_t(0x50000), pEndHdr->s_timestamp);
  EQ(uint32_t(1), pEndHdr->s_sourceId);
  EQ(uint32_t(2), pEndHdr->s_barrier);
}
// begin run has body header, end run does not.

void apptests::fragmaker_complete_2()
{
  pRingItem pBegin =
    reinterpret_cast<pRingItem>(makeStateChange(0x100, 1, BEGIN_RUN));
  pRingItem pEnd   =
    reinterpret_cast<pRingItem>(makeStateChange(END_RUN));
    
  CRingBlockReader::DataDescriptor d = packItems({pBegin, pEnd});
  m_pReader->addData(d);
  
  (*m_pTestObj)();
  
  EQ(0, m_pFragMaker->getEndRunsRemaining());
  EQ(size_t(4), m_pWriter->getWrittenData().size());  // two ring items & headers.
  EQ(uint64_t(0x100), m_pFragMaker->getLastTimestamp());
  
  // We'll just require that the end run timestamp is 0x1000 -- should
  
  std::pair<size_t, void*> beginHeader = m_pWriter->get();
  std::pair<size_t, void*> beginItem   = m_pWriter->get();
  
  std::pair<size_t, void*> endHeader   = m_pWriter->get();
  std::pair<size_t, void*> endItem     = m_pWriter->get();
  EVB::FragmentHeader* pEndHdr =
    static_cast<EVB::FragmentHeader*>(endHeader.second);
  EQ(uint64_t(0x100), pEndHdr->s_timestamp);
}
