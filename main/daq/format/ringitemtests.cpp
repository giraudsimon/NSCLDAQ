// Template for a test suite.
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <config.h>

#define private public
#define protected public
#include <CRingItem.h>
#undef private
#undef protected

#include <DataFormat.h>
#include <CAllButPredicate.h>
#include <CDesiredTypesPredicate.h>

#include <CRingBuffer.h>
#include <CRingItemFactory.h>

#include <memory>

std::string uniqueName(std::string);


using namespace std;

class physeventtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(physeventtests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(selectors);
  CPPUNIT_TEST(cursor);
  CPPUNIT_TEST(toring);
  CPPUNIT_TEST(fromring);
  CPPUNIT_TEST(selection);
  CPPUNIT_TEST(copyconstruct);
  CPPUNIT_TEST(tsconstruct);
  CPPUNIT_TEST(addbodyheader);
  CPPUNIT_TEST(equality);
  CPPUNIT_TEST(size);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void construct();    // Non timestamped.
  void copyconstruct();
  void selectors();
  void cursor();
  void toring();
  void fromring();
  void selection();
  void sampling();
  void tsconstruct();
  void addbodyheader();
  void equality();
  void size();
};

CPPUNIT_TEST_SUITE_REGISTRATION(physeventtests);

//
// There are two cases for construction...
// assuming the static ring buffer size is 8k.
// 
// 
void physeventtests::construct() {
  CRingItem small(1);
  CRingItem big(1, CRingItemStaticBufferSize*2);

  // The tests below differentiate between small and large buffers.

  EQ(reinterpret_cast<uint8_t*>(small.m_pItem), 
     (uint8_t*)small.m_staticBuffer);
  ASSERT(reinterpret_cast<uint8_t*>(big.m_pItem) != big.m_staticBuffer);

  // Let's look at the member data for small too:

  EQ((void*)small.m_pItem->s_body.u_noBodyHeader.s_body, (void*)small.m_pCursor);
  EQ((uint32_t)sizeof(uint32_t), small.m_pItem->s_body.u_noBodyHeader.s_empty);
  EQ(CRingItemStaticBufferSize-10, small.m_storageSize);
  EQ(false, small.m_swapNeeded);

  // Let's look at the member data for big:
  
  EQ((uint8_t*)big.m_pItem->s_body.u_noBodyHeader.s_body, big.m_pCursor);
  EQ((uint32_t)sizeof(uint32_t), big.m_pItem->s_body.u_noBodyHeader.s_empty);
  EQ(CRingItemStaticBufferSize*2, big.m_storageSize);
  EQ(false, big.m_swapNeeded);
}

// Test that we've fixed the error in copy construction that did not
// get the cursor right:
//
void physeventtests::copyconstruct()
{
  // Make the source ring and put something in it:
  CRingItem source(100);
  uint8_t  *payload = reinterpret_cast<uint8_t*>(source.getBodyCursor());
  for (int i =0; i < 16; i++) {
    *payload++ = i;
  }
  source.setBodyCursor(payload);
  source.updateSize();

  CRingItem copy(source);

   EQ(source.getBodySize(), copy.getBodySize());
  _RingItem* porig = source.getItemPointer();
  _RingItem* pcopy = copy.getItemPointer();

  // headers must match 

  EQ(porig->s_header.s_size, pcopy->s_header.s_size);
  EQ(porig->s_header.s_type, pcopy->s_header.s_type);

  // contents must match:

  uint8_t* pSource = reinterpret_cast<uint8_t*>(source.getBodyPointer());
  uint8_t* pCopy   = reinterpret_cast<uint8_t*>(copy.getBodyPointer());
  for (int i = 0; i < 16; i++) {
    EQ((int)*pSource, (int)*pCopy);
    pSource++;
    pCopy++;
  }


}

//
// Tests the various selector  member functions.
//
void
physeventtests::selectors()
{
  CRingItem item(5, 1024);
  EQ((size_t)1024, item.getStorageSize());
  EQ((size_t)0, item.getBodySize());
  EQ((void*)item.m_pCursor, item.getBodyPointer());
  EQ((void*)item.m_pCursor, item.getBodyCursor());
  EQ(item.m_pItem, item.getItemPointer());
  EQ((uint32_t)5, item.type());
  ASSERT(!item.mustSwap());
}
//
// Test the ability to manipulate the item by filling it in and
// updating the body cursor.
//
void
physeventtests::cursor()
{
  CRingItem item(0x1234);
  uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
  for (uint16_t  i =0; i < 16; i++) {
    *pData++  = i;
  }
  item.setBodyCursor(pData);

  EQ(16*sizeof(uint16_t), item.getBodySize());
}

//
// Test the ability to write an item into a ring buffer.
//
void
physeventtests::toring()
{
  CRingBuffer::create(uniqueName("items"));

  try {
    // Create fill an item

    CRingItem item(0x1234);
    uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
    for (uint16_t  i =0; i < 16; i++) {
      *pData++  = i;
    }
    item.setBodyCursor(pData);

    CRingBuffer prod(uniqueName("items"), CRingBuffer::producer);
    CRingBuffer cons(uniqueName("items"), CRingBuffer::consumer);

    item.commitToRing(prod);

    const size_t itemSize = item.getBodySize() + sizeof(RingItemHeader)
                            + sizeof(uint32_t);
    EQ(itemSize, cons.availableData());
    uint8_t itemData[itemSize];
    
    size_t got = cons.get(itemData, itemSize, itemSize, 0);
    EQ(itemSize, got);

    // Check contents:

    RingItem* pItem = reinterpret_cast<RingItem*>(itemData);
    EQ((uint32_t)0x1234, pItem->s_header.s_type);
    EQ(itemSize, (size_t)pItem->s_header.s_size);
    uint16_t* body = reinterpret_cast<uint16_t*>(pItem->s_body.u_noBodyHeader.s_body);
    for (uint16_t i =0; i < 16; i++) {
      EQ(i, body[i]);
    }
  }
  catch(...) {
    CRingBuffer::remove(uniqueName("items"));
    throw;
  }
  CRingBuffer::remove(uniqueName("items"));


}

//
// Insert an item int oa ring and then see if it can be gotten out
// we'll use a the all predicate with no exceptions in this test.
//
void
physeventtests::fromring()
{
  CRingBuffer::create(uniqueName("items"));
  
  try {
    CRingItem item(0x1234);
    uint16_t* pData = reinterpret_cast<uint16_t*>(item.getBodyCursor());
    for (uint16_t  i =0; i < 16; i++) {
      *pData++  = i;
    }
    item.setBodyCursor(pData);

    CRingBuffer prod(uniqueName("items"), CRingBuffer::producer);
    CRingBuffer cons(uniqueName("items"), CRingBuffer::consumer);

    item.commitToRing(prod);

    CAllButPredicate all;
    CRingItem* pItem = CRingItem::getFromRing(cons, all);
    ASSERT(pItem);

    EQ((uint32_t)0x1234, pItem->type());
    EQ((size_t)16*sizeof(uint16_t), pItem->getBodySize());
    uint16_t* pBody = reinterpret_cast<uint16_t*>(pItem->getBodyPointer());
    for (uint16_t i=0; i < 16; i++) {
      EQ(i, pBody[i]);
    }


    delete pItem;
    
  }
  catch (...) {
    CRingBuffer::remove(uniqueName("items"));
    throw;
  
  }
  CRingBuffer::remove(uniqueName("items"));


}
//
// Lets' get data with some selectivity.
//   We'll insert an item with type 1 and another with type 2.
// we'll request type 2 only with no sampling.. getting should get us that
// item.
void
physeventtests::selection()
{
  CRingBuffer::create(uniqueName("items"));

  try {
    CRingItem i1(1);
    CRingItem i2(2);


    CRingBuffer prod(uniqueName("items"), CRingBuffer::producer);
    CRingBuffer cons(uniqueName("items"), CRingBuffer::consumer);

    i1.commitToRing(prod);
    i2.commitToRing(prod);

    CDesiredTypesPredicate two;
    two.addDesiredType(2);

    CRingItem* pItem = CRingItem::getFromRing(cons, two);
    EQ((uint32_t)2, pItem->type());

  }
  catch(...) {
    CRingBuffer::remove(uniqueName("items"));
    throw;
  }

  CRingBuffer::remove(uniqueName("items"));
}

// Let's get data with sampling.
// insert two items of type 1, get with sampling on type 1. Should get
// the second item.
//
void 
physeventtests::sampling()
{
   CRingBuffer::create(uniqueName("items"));

  try {
    CRingItem i1(1);
    CRingItem i2(1);

    // Distinguish i1 from i2 by putting data into it:

    uint16_t* pData =  reinterpret_cast<uint16_t*>(i2.getBodyCursor());
    *pData++ = 2;
    i2.setBodyCursor(pData);

    CRingBuffer prod(uniqueName("items"), CRingBuffer::producer);
    CRingBuffer cons(uniqueName("items"), CRingBuffer::consumer);

    i1.commitToRing(prod);
    i2.commitToRing(prod);

    CDesiredTypesPredicate s;
    s.addDesiredType(1, true);

    CRingItem* pItem = CRingItem::getFromRing(cons, s);
    EQ(sizeof(uint16_t), pItem->getBodySize());


  }
  catch(...) {
    CRingBuffer::remove(uniqueName("items"));
    throw;
  }

  CRingBuffer::remove(uniqueName("items"));
} 

/**
 * tsconstruct
 *   Construction of timestamped item.
 */
void
physeventtests::tsconstruct()
{
    CRingItem ts(
        1, static_cast<uint64_t>(0x1122334455667788ll), 12, 34, 10
    );
    /* Check the body header is ok */
    // Use of u_hasBodyHeader.s_body is ok here because we are generating
    // synthetic events that don't have a body header extension.

    pRingItem    pItem = ts.getItemPointer();
    pBodyHeader  pB    = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    void*        pBody = reinterpret_cast<void*>(
        pItem->s_body.u_hasBodyHeader.s_body
    );
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
  
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pB->s_size);
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pB->s_timestamp);
    EQ(static_cast<uint32_t>(12), pB->s_sourceId);
    EQ(static_cast<uint32_t>(34), pB->s_barrier);
    
    /* Check that the body pointer is returned correctly */
    
    EQ(pBody, ts.getBodyPointer());
}
// test subsequent add of body header:

void
physeventtests::addbodyheader()
{
    CRingItem item(1, 8192);
    uint16_t* p = reinterpret_cast<uint16_t*>(item.getBodyCursor());
    for (int i = 0; i < 10; i++) {
        *p++ = i;
    }
    item.setBodyCursor(p);
    item.updateSize();

    
    // Add in a body header:
    
    item.setBodyHeader(123, 1, 2);
    
    // Check size:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pRingItem pItem = item.getItemPointer();
    EQ(static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint16_t)*10), 
       pItem->s_header.s_size);
    EQ((uint64_t)123, item.getEventTimestamp());
    EQ((uint32_t)1, item.getSourceId());
    EQ((uint32_t)2, item.getBarrierType());
    p = reinterpret_cast<uint16_t*>(item.getBodyPointer());
    
    for (uint16_t i = 0; i < 10; i++) {
        EQ(i, *p++);
    }
    
}

void physeventtests::equality()
{
    CRingItem item0(30,8192);
    CRingItem item1(item0);

    ASSERT( item0 == item1 );

}


void physeventtests::size() 
{
  CRingItem item( PHYSICS_EVENT );
  uint16_t* pBody = reinterpret_cast<uint16_t*>(item.getBodyPointer());
  *pBody = 0;
  pBody++;

  item.setBodyCursor(pBody);
  item.updateSize();

  pRingItem pItem = item.getItemPointer();

  // because we made the item locally on this computer, we don't
  // need to worry about byte order
  EQ( pItem->s_header.s_size, item.size() );
}
