// Template for a test suite.
 
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "V11/CAbnormalEndItem.h"
#include "V11/DataFormatV11.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <utils.h>

using namespace DAQ::V11;

////////////////////////////////////////////////////////////////////////////////
// Query methods:
//
class InfoTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(InfoTests);
    CPPUNIT_TEST(noswap);
    CPPUNIT_TEST(swapped);
    CPPUNIT_TEST(sizenos);
    CPPUNIT_TEST(sizes);
    CPPUNIT_TEST(typenos);
    CPPUNIT_TEST(types);
    CPPUNIT_TEST_SUITE_END();

protected:
    RingItemHeader straight;
    RingItemHeader switched;    
    
public:
    void setUp() {
        straight.s_size = sizeof(RingItemHeader);
        straight.s_type = PHYSICS_EVENT;
        
        switched.s_size = swal(sizeof(RingItemHeader));
        switched.s_type = swal(PHYSICS_EVENT);
        
    }
    void tearDown() {}
protected:
    void noswap();
    void swapped();
    void sizenos();
    void sizes();
    void typenos();
    void types();
};
CPPUNIT_TEST_SUITE_REGISTRATION(InfoTests);
/**
 * Make sure mustSwap returns 0 if don't need to swap.
 */
void InfoTests::noswap()
{
    
    
    EQ(0, mustSwap((pRingItem)&straight));
}
/**
 * Make sure mustSwap returns 1 if need to  swap
 */
void InfoTests::swapped()
{
    
    
    EQ(1, mustSwap((pRingItem)&switched));
}

// Unswapped size
void InfoTests::sizenos()
{
    EQ(sizeof(RingItemHeader),
      (size_t)(itemSize((pRingItem)&straight)));
}
// Swapped size:

void InfoTests::sizes()
{
    EQ(sizeof(RingItemHeader),
       (size_t)(itemSize((pRingItem)&switched)));
}
// Unswapped item type:
void InfoTests::typenos()
{
    EQ((uint16_t)PHYSICS_EVENT, itemType((pRingItem)&straight));
 
}
// Swapped item type:
void InfoTests::types()
{
    EQ((uint16_t)PHYSICS_EVENT, itemType((pRingItem)&switched));
}
//////////////////////////////////////////////////////////////////////////////////////////
// Event item tests.

class PhysicsItemOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PhysicsItemOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(counting);
  CPPUNIT_TEST(emptyTimestamped);
  CPPUNIT_TEST(countingTimestamped);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void empty();
  void counting();
  void emptyTimestamped();
  void countingTimestamped();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsItemOutput);

// An empty item should just contain a longword  in the
// s_body field and the size should match that.

void PhysicsItemOutput::empty() {
  pPhysicsEventItem pItem = formatEventItem(0, NULL);
  ASSERT(pItem);

  EQMSG("Size of empty event item",
	static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(uint32_t)*2), pItem->s_header.s_size);
  EQMSG("Type of empty event",
	PHYSICS_EVENT, pItem->s_header.s_type);

  
  uint32_t* pPayload = reinterpret_cast<uint32_t*>(pItem->s_body.u_noBodyHeader.s_body);
  EQMSG("Payload contents",
	sizeof(uint32_t)/sizeof(uint16_t), static_cast<size_t>(*pPayload));

  // mbz check:
  
  EQ(static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_mbz);

  free(pItem);
}


// Construct an empty timestamped item:

void
PhysicsItemOutput::emptyTimestamped()
{
    pPhysicsEventItem pItem = formatTimestampedEventItem(
        0x1234567876543210ll, 123, 0, 0, NULL);
    
    // Make sure the header is right
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t)), pItem->s_header.s_size);
    EQ(PHYSICS_EVENT, pItem->s_header.s_type);
    
    // Make sure the Body header is right:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pBH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)),   pBH->s_size);
    EQ(static_cast<uint64_t>(0x1234567876543210ll), pBH->s_timestamp);
    EQ(static_cast<uint32_t>(123), pBH->s_sourceId);
    EQ(static_cast<uint32_t>(0),   pBH->s_barrier);
    
    
    // Check the payload.. should be a uint32_t sizeof(uint32_t)/sizeof(uint16_t)
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    EQ(
       static_cast<uint32_t>(sizeof(uint32_t)/sizeof(uint16_t)),
       *reinterpret_cast<uint32_t*>(pItem->s_body.u_hasBodyHeader.s_body)
    );
    free(pItem);
}


// Check for an event with a counting pattern of 0-9
// - Size ok.
// - Payload size ok.
// - Contents ok.
// The type is assumed to have been validated by empty().
//

void
PhysicsItemOutput::counting()
{
  // build the event payload:

  uint16_t payload[10];
  for (int i =  0; i < 10; i++) { payload[i] = i;}

  pPhysicsEventItem pItem = formatEventItem(10, payload);

  ASSERT(pItem);
  
  EQMSG("Counting item size",
	static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(uint32_t)*2 + 10*sizeof(uint16_t)),
	pItem->s_header.s_size);


  struct PayloadShape {
    uint32_t s_size;
    uint16_t s_body[10];
  };
  PayloadShape* pBody = reinterpret_cast<PayloadShape*>(pItem->s_body.u_noBodyHeader.s_body);


  EQMSG("Size in payload",
	static_cast<uint32_t>(sizeof(uint32_t)/sizeof(uint16_t) + 10),pBody->s_size);

  for(uint16_t i = 0; i < 10; i++) {
    EQMSG("Contents in payload",
	  i, pBody->s_body[i]);
  }
	
       

  free(pItem);
}
//
// Check the contents of a timestamped event that has a counting pattern 0-9
// in it:

void
PhysicsItemOutput::countingTimestamped()
{
    uint16_t payload[10];
    for (int i =0; i < 10; i++) {
        payload[i] = i;
    }
    
    pPhysicsEventItem pItem = formatTimestampedEventItem(
        0x8765432123456789ll, 1, 2, 10, payload    
    );
    
    // Check the header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader)
        + sizeof(uint32_t) + 10*sizeof(uint16_t)),
        pItem->s_header.s_size
    );
    EQ(PHYSICS_EVENT, pItem->s_header.s_type);
    
    // Check the body header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x8765432123456789ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(1), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(2), pBh->s_barrier);
    
    
    // Check the payload.

    struct PayloadShape {
       uint32_t s_size;
       uint16_t s_body[10];
     };
     // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
     
     struct PayloadShape* pPayload = reinterpret_cast<struct PayloadShape*>(
        pItem->s_body.u_hasBodyHeader.s_body
     );
     
    EQ(
        static_cast<uint32_t>((sizeof(uint32_t) + 10*sizeof(uint16_t))/sizeof(uint16_t)),
        pPayload->s_size
    );
    for (int i =0; i < 10; i++) {
        EQ(payload[i], pPayload->s_body[i]);
    }
    
    // Release dynamic storage:
    
    free(pItem);
}

/////////////////////////////////////////////////////////////////////////////////
// Event trigger count item tests.

class PhysicsCountOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PhysicsCountOutput);
  CPPUNIT_TEST(itemformat);
  CPPUNIT_TEST(timestampedItem);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void itemformat();
  void timestampedItem();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhysicsCountOutput);


void
PhysicsCountOutput::itemformat()
{
  pPhysicsEventCountItem pItem = formatTriggerCountItem(1234, 666, 0xaaaa);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(PhysicsEventCountItemBody);
  EQMSG("Physics count size",
	properSize, pItem->s_header.s_size);
  EQMSG("Physics count type",
	PHYSICS_EVENT_COUNT, pItem->s_header.s_type);
  EQMSG("Time offset",  static_cast<uint32_t>(1234), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG("Time stamp", static_cast<uint32_t>(666), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG("Trigger count", static_cast<uint64_t>(0xaaaa), pItem->s_body.u_noBodyHeader.s_body.s_eventCount);
  EQMSG("Offset Divisor", static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_offsetDivisor);
  EQMSG("Mbz", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_mbz);


  free(pItem);
}

void
PhysicsCountOutput::timestampedItem()
{
    time_t stamp = time(NULL);
    pPhysicsEventCountItem pItem = formatTimestampedTriggerCountItem(
        static_cast<uint64_t>(0x1234123412341234ll), 1, 0,
        1234, 2, stamp, 0x666444333222ll
    );
    
    // Check header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(PHYSICS_EVENT_COUNT, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader)
            + sizeof(PhysicsEventCountItemBody) 
        ), pItem->s_header.s_size
    );
    
    // Check Body header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1234123412341234ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(1), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(0), pBh->s_barrier);
    
    // Check body
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pPhysicsEventCountItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(1234), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(2),  pBody->s_offsetDivisor);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint64_t>(0x666444333222ll), pBody->s_eventCount);
    
    free(pItem);
}

////////////////////////////////////////////////////////////////////////////////////
//  Check scaler items.

class ScalerOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ScalerOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(counting);
  CPPUNIT_TEST(emptyTimestamped);
  CPPUNIT_TEST(countingTimestamped);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() {}
  void tearDown() {}

protected:
  void empty();
  void counting();
  void emptyTimestamped();
  void countingTimestamped();
};
CPPUNIT_TEST_SUITE_REGISTRATION(ScalerOutput);


// Check zero scaler count
//
void
ScalerOutput::empty()
{
  pScalerItem pItem = formatScalerItem(0, 0x1234, 0, 1, NULL);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(ScalerItemBody);
    
  EQMSG("Empty scaler size", properSize, pItem->s_header.s_size);
  EQMSG("Scaler type: ", PERIODIC_SCALERS, pItem->s_header.s_type);
  EQMSG("mbz", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_mbz);
  EQMSG("start time", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_body.s_intervalStartOffset);
  EQMSG("stop time",  static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_intervalEndOffset);
  EQMSG("timestamp",  static_cast<uint32_t>(0x1234), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG("Count",      static_cast<uint32_t>(0),      pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);
  EQMSG("Incremental", static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_isIncremental);
  EQMSG("Time divisor", static_cast<uint32_t>(1), pItem->s_body.u_noBodyHeader.s_body.s_intervalDivisor);

  free(pItem);
}

// Emtpy timestamped scaler:

void
ScalerOutput::emptyTimestamped()
{
    time_t stamp = time(NULL);
    
    pScalerItem pItem = formatTimestampedScalerItem(
        static_cast<uint64_t>(0x1111111122222222ll), 5, 4,
        1, 1, stamp, 10, 12, 0, NULL
    );
    
    // Check header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(ScalerItemBody)
        ), pItem->s_header.s_size
    );
    EQ(PERIODIC_SCALERS, pItem->s_header.s_type);
    
    // Check body header:
    // Contrived data so sizeof(BodyHeader) is fine.
	// u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1111111122222222ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(5), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(4), pBh->s_barrier);
    
    // Check body:
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pScalerItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(10), pBody->s_intervalStartOffset);
    EQ(static_cast<uint32_t>(12), pBody->s_intervalEndOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint32_t>(1), pBody->s_intervalDivisor);
    EQ(static_cast<uint32_t>(0), pBody->s_scalerCount);
    EQ(static_cast<uint32_t>(1), pBody->s_isIncremental);
    
    
    free(pItem);
}

//
// Check with a counting pattern of 10 scalers.
void
ScalerOutput::counting()
{
  // the scalers:

  uint32_t scalers[10];
  for (int i=0; i < 10; i++) { scalers[i] = i;}

  pScalerItem pItem = formatScalerItem(10, 0x4567, 0, 1, scalers);

  // Assume the only things to check are:
  // - Size of the entire item.
  // - Count of scalers.
  // - Payload.
  //
  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(ScalerItemBody)
    + 10*sizeof(uint32_t);
  EQMSG("Counting scaler size", properSize, pItem->s_header.s_size);
  EQMSG("No of scalers", static_cast<uint32_t>(10),
        pItem->s_body.u_noBodyHeader.s_body.s_scalerCount);

  for (uint32_t i = 0; i < 10; i++) {
    EQMSG("Scaler payload",  i, pItem->s_body.u_noBodyHeader.s_body.s_scalers[i]);
  }
  

  free(pItem);
}
// Check with a counting pattern of 10 scalers:

void
ScalerOutput::countingTimestamped()
{
    time_t stamp = time(NULL);
    
    uint32_t scalers[10];
    for (int i=0; i < 10; i++) {scalers[i] = i;}
    pScalerItem pItem = formatTimestampedScalerItem(
        static_cast<uint64_t>(0x1111111122222222ll), 5, 4,
        1, 1, stamp, 10, 12, 10, scalers
    );    
    
    // Check header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(PERIODIC_SCALERS, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(ScalerItemBody)
            + 10*sizeof(uint32_t)                 // (one long in the body)
        ),
        pItem->s_header.s_size    
    );
    
    // Check body header:
    // Contrived data so sizeof(BodyHeader) is fine.
	// u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1111111122222222ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(5), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(4), pBh->s_barrier);
    
    
    // Check body:
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pScalerItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(10), pBody->s_intervalStartOffset);
    EQ(static_cast<uint32_t>(12), pBody->s_intervalEndOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint32_t>(1), pBody->s_intervalDivisor);
    EQ(static_cast<uint32_t>(10), pBody->s_scalerCount);
    EQ(static_cast<uint32_t>(1), pBody->s_isIncremental);
    
    for (int i = 0; i < 10; i++) {
        EQ(scalers[i], pBody->s_scalers[i]);
    }
    
    free(pItem);
}

///////////////////////////////////////////////////////////////////////////////
// Test text output item.

class TextOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TextOutput);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(someStrings);
  CPPUNIT_TEST(emptyTimestamp);
  CPPUNIT_TEST(someStringsTimestamp);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}
  void tearDown() {}

protected:
  void empty();
  void someStrings();
  void emptyTimestamp();
  void someStringsTimestamp();

};
CPPUNIT_TEST_SUITE_REGISTRATION(TextOutput);

//
// Empty test.. 
// - Size should be sizeof(Textitem - sizeof(char)
// - Type shoujild be whatever we tell it to be.
// - Time offset, timestamp should be what we tell it to be
// - string count should be zero.
//
void
TextOutput::empty()
{
  pTextItem pItem = formatTextItem(0, 0xaaaa, 0xbbbb, NULL, MONITORED_VARIABLES);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody);
  EQMSG("Empty text item size", properSize, pItem->s_header.s_size);
  EQMSG("Type ", MONITORED_VARIABLES, pItem->s_header.s_type);
  EQMSG(" time offset", static_cast<uint32_t>(0xbbbb), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG(" timestamp", static_cast<uint32_t>(0xaaaa), pItem->s_body.u_noBodyHeader.s_body.s_timestamp);
  EQMSG(" string count", static_cast<uint32_t>(0),   pItem->s_body.u_noBodyHeader.s_body.s_stringCount);

  free(pItem);
}

void
TextOutput::emptyTimestamp()
{
    time_t stamp = time(NULL);
    
    pTextItem pItem = formatTimestampedTextItem(
        0x1122334455667788ll, 1, 1, 0, stamp, 1234, NULL, MONITORED_VARIABLES, 1
    );
    // Check header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(MONITORED_VARIABLES, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(TextItemBody)
        ), pItem->s_header.s_size
    );
    
    // Check body header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pB = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pB->s_size);
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pB->s_timestamp);
    EQ(static_cast<uint32_t>(1), pB->s_sourceId);
    EQ(static_cast<uint32_t>(1), pB->s_barrier);
    
    
    // Check body proper.
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pTextItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(1234), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint32_t>(0), pBody->s_stringCount);
    EQ(static_cast<uint32_t>(1), pBody->s_offsetDivisor);
    
    free(pItem);
}
//
// put a few strings in, check the string count and check the string contents.
//
void
TextOutput::someStrings()
{
  const char* strings[] = {	// 4 strings.
    "First string",
    "Second String",
    "Third string",
    "Last String"
  };

  uint32_t stringSize = 0;
  for (int i=0; i < 4; i++) {
    stringSize += strlen(strings[i]) + 1;
  }

  pTextItem pItem = formatTextItem(4, 0xaaaa, 0xbbbb, strings, MONITORED_VARIABLES);

  ASSERT(pItem);
  uint32_t properSize =
    sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody)
    + stringSize;
  EQMSG("Item size",    properSize, pItem->s_header.s_size);
  EQMSG("String count", static_cast<uint32_t>(4), pItem->s_body.u_noBodyHeader.s_body.s_stringCount);
 
  char* p = pItem->s_body.u_noBodyHeader.s_body.s_strings;

  for (int i = 0; i < 4; i++) {
    EQMSG("Contents", std::string(strings[i]), std::string(p));
    p += strlen(p) + 1;
  }
  

  free(pItem);
  
}
// Check a timestamped item that has stings:

void
TextOutput::someStringsTimestamp()
{
    time_t stamp = time(NULL);
    const char* strings[] = {	// 4 strings.
        "First string",
        "Second String",
        "Third string",
        "Last String"
    };
    uint32_t stringSizes = 0;
    for (int i =0; i < 4; i++) {
        stringSizes += strlen(strings[i]) + 1;
    }
    pTextItem pItem = formatTimestampedTextItem(
        0x1122334455667788ll, 1, 1, 4, stamp, 1234, strings, MONITORED_VARIABLES, 1
    );
    // Check header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(MONITORED_VARIABLES, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(TextItemBody)
            + stringSizes
        ), pItem->s_header.s_size
    );
    
    // Check body header:
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pH->s_size);
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pH->s_timestamp);
    EQ(static_cast<uint32_t>(1), pH->s_sourceId);
    EQ(static_cast<uint32_t>(1), pH->s_barrier);
    
    
    // Check body:
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pTextItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(1234), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint32_t>(4), pBody->s_stringCount);
    EQ(static_cast<uint32_t>(1), pBody->s_offsetDivisor);
    
    const char* p = pBody->s_strings;
    for (int i = 0; i < 4; i++) {
        EQ(0, strcmp(strings[i], p));
        p += strlen(p) + 1;
    }
    
    free(pItem);
    
}

///////////////////////////////////////////////////////////////////////////////////////////
// Check the state change items:
//

class StateChangeOutput : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StateChangeOutput);
  CPPUNIT_TEST(begin);
  CPPUNIT_TEST(beginTimestamped);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}
protected:
  void begin();
  void beginTimestamped();
};
CPPUNIT_TEST_SUITE_REGISTRATION(StateChangeOutput);

void
StateChangeOutput::begin()
{
  pStateChangeItem pItem = formatStateChange(0x66eb, 0, 1234, "This is a test title",
					     BEGIN_RUN);

  ASSERT(pItem);
  EQMSG("State change item sizse", static_cast<uint32_t>(sizeof(StateChangeItem)), pItem->s_header.s_size);
  EQMSG("Item type", BEGIN_RUN, pItem->s_header.s_type);
  EQMSG("Run Number", static_cast<uint32_t>(1234), pItem->s_body.u_noBodyHeader.s_body.s_runNumber);
  EQMSG("Time offset", static_cast<uint32_t>(0), pItem->s_body.u_noBodyHeader.s_body.s_timeOffset);
  EQMSG("Timestamp", static_cast<uint32_t>(0x66eb), pItem->s_body.u_noBodyHeader.s_body.s_Timestamp);
  EQMSG("Title", std::string("This is a test title"), std::string(pItem->s_body.u_noBodyHeader.s_body.s_title));

  free(pItem);
}
void
StateChangeOutput::beginTimestamped()
{
    time_t stamp = time(NULL);
    const char* titleString = "This is the run title";
    
    pStateChangeItem pItem = formatTimestampedStateChange(
        static_cast<uint64_t>(8877665544332211ll), 12, 34,
        stamp, 0, 12, 1, titleString, BEGIN_RUN
    );
    // Check header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader)
            + sizeof(StateChangeItemBody)), pItem->s_header.s_size
    );
    EQ(BEGIN_RUN, pItem->s_header.s_type);
    
    // Check body header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader pH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pH->s_size);
    EQ(static_cast<uint64_t>(8877665544332211ll), pH->s_timestamp);
    EQ(static_cast<uint32_t>(12), pH->s_sourceId);
    EQ(static_cast<uint32_t>(34), pH->s_barrier);
    

    // Check body contents.
    // u_hasBodyHeader.s_body is ok here because we're generating synthetic events.
    
    pStateChangeItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(12), pBody->s_runNumber);
    EQ(static_cast<uint32_t>(0), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_Timestamp);
    EQ(static_cast<uint32_t>(1), pBody->s_offsetDivisor);
    EQ(0, strcmp(titleString, pBody->s_title));
    
    free(pItem);
}


/*----------------------------------------------------------------------------
 * Check miscellaneous formats not worthy of a separate class:
 *--------------------------------------------------------------------------*/
class MiscFormat :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MiscFormat);
    CPPUNIT_TEST(dataFormat);
    CPPUNIT_TEST(evbFragment);
    CPPUNIT_TEST(unknownPayload);
    CPPUNIT_TEST(glomParameters);
    CPPUNIT_TEST(abnormalend);
    CPPUNIT_TEST(abnormalendclass);
    CPPUNIT_TEST(abnormalendtypename);
    CPPUNIT_TEST(abnormalendstring);
    CPPUNIT_TEST_SUITE_END();
protected:
    
    void dataFormat();
    void evbFragment();
    void unknownPayload();
    void glomParameters();
    void abnormalend();
    void abnormalendclass();
    void abnormalendtypename();
    void abnormalendstring();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MiscFormat);


/**
 * dataFormat - contains the data format level of the
 * ring items in the ringbuffer:
 */
void
MiscFormat::dataFormat()
{
    pDataFormat pItem = formatDataFormat();
    
    // Check header:
    
    EQ(static_cast<uint32_t>(sizeof(DataFormat)), pItem->s_header.s_size);
    EQ(RING_FORMAT, pItem->s_header.s_type);
    
    
    //Check body:
    
    EQ(static_cast<uint32_t>(0), pItem->s_mbz);
    EQ(FORMAT_MAJOR, pItem->s_majorVersion);
    EQ(FORMAT_MINOR, pItem->s_minorVersion);
    
    free(pItem);   
}
/**
 * evbFragment - A ring item containing an event fragment.
 */
void
MiscFormat::evbFragment()
{
    // Create payload:
    
    uint8_t data[10];
    for (int i =0; i < 10; i++) {
        data[i] = i;
    }
    
    pEventBuilderFragment pItem = formatEVBFragment(
        static_cast<uint64_t>(0x8888888877777777ll), 1, 2,
        10, data
    );
    // Check the header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + 10),
        pItem->s_header.s_size
    );
    EQ(EVB_FRAGMENT, pItem->s_header.s_type);
    
    // Check the body header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader p = &(pItem->s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), p->s_size);
    EQ(static_cast<uint64_t>(0x8888888877777777ll), p->s_timestamp);
    EQ(static_cast<uint32_t>(1), p->s_sourceId);
    EQ(static_cast<uint32_t>(2), p->s_barrier);
    
    
    
    // Check the body itself.. which is just the bytes of data.
    
    for (int i = 0; i < 10; i++) {
        EQ(data[i], pItem->s_body[i]);
    }
    
    free(pItem);
}
/**
 * unknownPayload - EVB fragment with unknown payload type
 **/
void
MiscFormat::unknownPayload()
{
    uint8_t data[10];
    for (int i =0; i < 10; i++) {
        data[i] = i;
    }
    pEventBuilderFragment pItem = formatEVBFragmentUnknown(
        static_cast<uint64_t>(0x8888888877777777ll), 1, 2,
        10, data        
    );
    // Check the header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + 10),
        pItem->s_header.s_size
    );
    EQ(EVB_UNKNOWN_PAYLOAD, pItem->s_header.s_type);
    
    // Check the body header
    // Contrived data so sizeof(BodyHeader) is fine.
		
    pBodyHeader p = &(pItem->s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), p->s_size);
    EQ(static_cast<uint64_t>(0x8888888877777777ll), p->s_timestamp);
    EQ(static_cast<uint32_t>(1), p->s_sourceId);
    EQ(static_cast<uint32_t>(2), p->s_barrier);
    
    
    
    // Check the body itself.. which is just the bytes of data.
    
    for (int i = 0; i < 10; i++) {
        EQ(data[i], pItem->s_body[i]);
    }
    
    free(pItem);
}
/**
 * glomParameters - parameters of the glom program:
 */
void
MiscFormat::glomParameters()
{
    pGlomParameters pItem = formatGlomParameters(
        static_cast<uint64_t>(100), 1, GLOM_TIMESTAMP_AVERAGE
    );
    
    // Check header:
    
    EQ(static_cast<uint32_t>(sizeof(GlomParameters)), pItem->s_header.s_size);
    EQ(EVB_GLOM_INFO, pItem->s_header.s_type);
    
    // Check body:
    
    EQ(static_cast<uint32_t>(0), pItem->s_mbz);
    EQ(static_cast<uint64_t>(100), pItem->s_coincidenceTicks);
    EQ(static_cast<uint16_t>(1), pItem->s_isBuilding);
    EQ(static_cast<uint16_t>(GLOM_TIMESTAMP_AVERAGE), pItem->s_timestampPolicy);
    
    free(pItem);
}

void
MiscFormat::abnormalend()
{
    pAbnormalEndItem pItem = formatAbnormalEndItem();
    EQ(static_cast<uint32_t>(sizeof(AbnormalEndItem)), pItem ->s_header.s_size);
    EQ(ABNORMAL_ENDRUN, pItem->s_header.s_type);
    EQ(uint32_t(0), pItem->s_mbz);
    free(pItem);
}

void
MiscFormat::abnormalendclass()
{
    CAbnormalEndItem item;
    pRingItem pItem = item.getItemPointer();
    EQ(static_cast<uint32_t>(sizeof(AbnormalEndItem)), pItem->s_header.s_size);
    EQ(ABNORMAL_ENDRUN, pItem->s_header.s_type);
    EQ(uint32_t(0), pItem->s_body.u_noBodyHeader.s_mbz);
    
}
void
MiscFormat::abnormalendtypename()
{
    CAbnormalEndItem item;
    EQ(std::string("Abnormal End"), item.typeName());
}
void
MiscFormat::abnormalendstring()
{
    CAbnormalEndItem item;
    EQ(std::string("Abnormal End\n"), item.toString());
}
