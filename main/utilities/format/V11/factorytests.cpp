// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"

#include "V11/DataFormatV11.h"
#include "V11/CRingItemFactory.h"
#include "V11/CRingItem.h"
#include "V11/CRingStateChangeItem.h"
#include "V11/CRingTextItem.h"
#include "V11/CRingScalerItem.h"
#include "V11/CPhysicsEventItem.h"
#include "V11/CRingPhysicsEventCountItem.h"
#include "V11/CRingFragmentItem.h"
#include "V11/CUnknownFragment.h"
#include "V11/CGlomParameters.h"


#include <time.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>

// Test the ring item factory.  Our test strategy will be to build
// Ring items (with and without headers if appropriate) using the
// data format methods, run those through both createRingItem methods
// to check the result.
//
using namespace DAQ::V11;

class RingFactoryTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(RingFactoryTests);
    CPPUNIT_TEST(stateChangeNoTs);
    CPPUNIT_TEST(stateChangeTs);
    CPPUNIT_TEST(textNoTs);
    CPPUNIT_TEST(textTs);
    CPPUNIT_TEST(scalerNoTs);
    CPPUNIT_TEST(scalerTs);
    CPPUNIT_TEST(physicsNoTs);
    CPPUNIT_TEST(physicsTs);
    CPPUNIT_TEST(triggersNoTs);
    CPPUNIT_TEST(triggersTs);
    CPPUNIT_TEST(evbFragment);    // Always have timestamps.
    CPPUNIT_TEST(evbUnkPayload);  // Always have timestamps.
    CPPUNIT_TEST(glom);           // Never has a timetamp.
    CPPUNIT_TEST_SUITE_END();
    
    
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void stateChangeNoTs();
    void stateChangeTs();

    void textNoTs();
    void textTs();
    
    void scalerNoTs();
    void scalerTs();
    
    void physicsNoTs();
    void physicsTs();
    
    void triggersNoTs();
    void triggersTs();
    
    void evbFragment();
    void evbUnkPayload();
    
    void glom();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingFactoryTests);

/**
 * stateChangeNoTs
 *
 * Create a ring state change item without a timestamp and run it past the
 * factory methods:
 */
void
RingFactoryTests::stateChangeNoTs()
{
    time_t stamp = time(NULL);
    pStateChangeItem pRingItem = formatStateChange(
        stamp, 1234, 66, "This is a title", END_RUN
    );
    
    CRingItem* pItem = CRingItemFactory::createRingItem(pRingItem);
    
    // Should be a state cvhange item:
    
    CRingStateChangeItem item(*pItem);  // should not throw!!
    
    EQ(static_cast<uint32_t>(66), item.getRunNumber());
    EQ(static_cast<uint32_t>(1234), item.getElapsedTime());
    EQ(std::string("This is a title"), item.getTitle());
    EQ(stamp, item.getTimestamp());
    EQ(END_RUN, item.type());
    
    // Free dynamic storage:
    
    delete(pItem);
    free(pRingItem);
}
void
RingFactoryTests::stateChangeTs()
{
    time_t stamp = time(NULL);
    pStateChangeItem pRingItem = formatTimestampedStateChange(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        stamp, 1234, 66, 1, "This is a title", END_RUN
    );
    CRingItem* pItem = CRingItemFactory::createRingItem(pRingItem);
    CRingStateChangeItem item(*pItem);
    
    EQ(static_cast<uint32_t>(66), item.getRunNumber());
    EQ(static_cast<uint32_t>(1234), item.getElapsedTime());
    EQ(std::string("This is a title"), item.getTitle());
    EQ(stamp, item.getTimestamp());
    EQ(END_RUN, item.type());
    
    
    // Should see we have a timestamp in the base and be able to access
    // the body header that way:
    
    ASSERT(pItem->hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pItem->getEventTimestamp());
    EQ(static_cast<uint32_t>(1), pItem->getSourceId());
    EQ(static_cast<uint32_t>(2), pItem->getBarrierType());
    
    // free storage:
    
    delete pItem;
    free(pRingItem);
    
}

/**
 * textNoTs
 *  Test factory for text item without a timestamp.
 */
void
RingFactoryTests::textNoTs()
{
    const char* strings[] = {              // not quote Dr. Suess.
        "one string",
        "two string",
        "three strings",
        "four",
        "five strings",
        "six strings",
        "seven strings",
        "more"
    };
    size_t nStrings = sizeof(strings)/sizeof(char*);
    time_t stamp =time(NULL);
    
    // Make the ring item struct.
    
    pTextItem  pStructItem = formatTextItem(
        nStrings, stamp, 1122, strings, MONITORED_VARIABLES
    );
    
    CRingItem* pBaseItem = CRingItemFactory::createRingItem(pStructItem);
    CRingTextItem item(*pBaseItem);
    
    // Get the stuff that the ring text item gives:
    
    EQ(static_cast<uint32_t>(1122), item.getTimeOffset());
    EQ(stamp, item.getTimestamp());
    std::vector<std::string> s = item.getStrings();
    EQ(nStrings, s.size());
    
    for(int i =0; i < nStrings; i++) {
        EQ(std::string(strings[i]), s[i]);
    }
    // Free storage:
    
    delete pBaseItem;
    free(pStructItem);
}
/**
 * textTs
 *
 * Test text item factory with a timestamp:
 */
void
RingFactoryTests::textTs()
{
    const char* strings[] = {              // not quote Dr. Suess.
        "one string",
        "two string",
        "three strings",
        "four",
        "five strings",
        "six strings",
        "seven strings",
        "more"
    };
    size_t nStrings = sizeof(strings)/sizeof(char*);
    time_t stamp =time(NULL);
    
    pTextItem pStructItem = formatTimestampedTextItem(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        nStrings, stamp, 1122, strings, MONITORED_VARIABLES, 1
    );
    CRingItem* pBaseItem = CRingItemFactory::createRingItem(pStructItem);
    CRingTextItem item(*pBaseItem);
    
    // Get the stuff that the ring text item gives:
    
    EQ(static_cast<uint32_t>(1122), item.getTimeOffset());
    EQ(stamp, item.getTimestamp());
    std::vector<std::string> s = item.getStrings();
    EQ(nStrings, s.size());
    
    for(int i =0; i < nStrings; i++) {
        EQ(std::string(strings[i]), s[i]);
    }
    // Check that timestamps etc. are in place.
    
    ASSERT(pBaseItem->hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pBaseItem->getEventTimestamp());
    EQ(static_cast<uint32_t>(1), pBaseItem->getSourceId());
    EQ(static_cast<uint32_t>(2), pBaseItem->getBarrierType());
        
    // Free storage:
    
    delete pBaseItem;
    free(pStructItem);    
}

void RingFactoryTests::scalerNoTs()
{
    uint32_t scalers[10];
    for (int i =0; i < 10; i++) {
        scalers[i] = i;
    }
    time_t stamp = time(NULL);
    
    pScalerItem pStruct = formatScalerItem(10, stamp, 10, 20, scalers);
    CRingItem* pBaseItem = CRingItemFactory::createRingItem(pStruct);
    CRingScalerItem item(*pBaseItem);
    
    // Check the item contents:
    
    EQ(static_cast<uint32_t>(10), item.getStartTime());
    EQ(static_cast<uint32_t>(20), item.getEndTime());
    EQ(static_cast<uint32_t>(10), item.getScalerCount());
    
    for (int i =0; i < 10; i++) {
        EQ(scalers[i], item.getScaler(i));
    }
    
    ASSERT(!item.hasBodyHeader());

    // Free the storage:
    delete pBaseItem;
    free(pStruct);
}
void
RingFactoryTests::scalerTs()
{
    uint32_t scalers[10];
    for (int i =0; i < 10; i++) {
        scalers[i] = i;
    }
    time_t stamp = time(NULL);
    
    pScalerItem pStruct = formatTimestampedScalerItem(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        1, 1, stamp, 10, 20, 10, scalers
    );
    
    CRingItem* pBaseItem = CRingItemFactory::createRingItem(pStruct);
    CRingScalerItem item(*pBaseItem);
    
    // Check the item contents:
    
    EQ(static_cast<uint32_t>(10), item.getStartTime());
    EQ(static_cast<uint32_t>(20), item.getEndTime());
    EQ(static_cast<uint32_t>(10), item.getScalerCount());
    
    for (int i =0; i < 10; i++) {
        EQ(scalers[i], item.getScaler(i));
    }
    
    ASSERT(item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pBaseItem->getEventTimestamp());
    EQ(static_cast<uint32_t>(1), pBaseItem->getSourceId());
    EQ(static_cast<uint32_t>(2), pBaseItem->getBarrierType());

    // Free the storage:
    
    delete pBaseItem;
    free(pStruct);
}
/**
 * physicsNoTs
 *
 * Physics event without a timestamp.
 */
void
RingFactoryTests::physicsNoTs()
{
    // This struct looks like the final payload:
    
    struct eventPayload {
        uint32_t size;
        uint16_t data[10];
    } event;
    
    event.size = sizeof(event)/sizeof(uint16_t);
    for (int i =0; i < 10; i++) {
        event.data[i] = i;
    }
    
    pPhysicsEventItem pEvent = formatEventItem(10, event.data);
    CRingItem*        pBase  = CRingItemFactory::createRingItem(pEvent);
    CPhysicsEventItem item   = CPhysicsEventItem(
        *reinterpret_cast<CPhysicsEventItem*>(pBase)
    );
    
    
    EQ(0, memcmp(&event, item.getBodyPointer(), sizeof(eventPayload)));
    ASSERT(!(item.hasBodyHeader()));
    
    delete pBase;
    free(pEvent);
    
}
void
RingFactoryTests::physicsTs()
{
    // This struct looks like the final payload:
    
    struct eventPayload {
        uint32_t size;
        uint16_t data[10];
    } event;
    
    event.size = sizeof(event)/sizeof(uint16_t);
    for (int i =0; i < 10; i++) {
        event.data[i] = i;
    }
    
    pPhysicsEventItem pEvent = formatTimestampedEventItem(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        10, event.data
    );

     CRingItem*        pBase  = CRingItemFactory::createRingItem(pEvent);
    CPhysicsEventItem item   = CPhysicsEventItem(
        *reinterpret_cast<CPhysicsEventItem*>(pBase)
    );
    
    
    EQ(0, memcmp(&event, item.getBodyPointer(), sizeof(eventPayload)));
    ASSERT(item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(2), item.getBarrierType());
    
    free(pEvent);
    delete pBase;
}
/**
 * triggerNoTs
 *
 * Trigger count item with no timestamp.
 */
void
RingFactoryTests::triggersNoTs()
{
    time_t stamp = time(NULL);
    
    pPhysicsEventCountItem pI =  formatTriggerCountItem(
        1234, stamp, static_cast<uint64_t>(0x1111111122222222ll)
    );
    CRingItem* pBase = CRingItemFactory::createRingItem(pI);
    CRingPhysicsEventCountItem item(*pBase);
    
    EQ(static_cast<uint32_t>(1234), item.getTimeOffset());
    EQ(stamp, item.getTimestamp());
    EQ(static_cast<uint64_t>(0x1111111122222222ll), item.getEventCount());
    
    ASSERT(!item.hasBodyHeader());
    
    delete pBase;
    free(pI);
}
/**
 * triggersTs
 *
 * Trigger count item with a timestamp.
 */
void
RingFactoryTests::triggersTs()
{
    time_t stamp = time(NULL);
    
    pPhysicsEventCountItem pI = formatTimestampedTriggerCountItem(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        1234, 1, stamp, static_cast<uint64_t>(0x1111111122222222ll)
    );
    CRingItem* pBase = CRingItemFactory::createRingItem(pI);
    CRingPhysicsEventCountItem item(*pBase);
    
    EQ(static_cast<uint32_t>(1234), item.getTimeOffset());
    EQ(stamp, item.getTimestamp());
    EQ(static_cast<uint64_t>(0x1111111122222222ll), item.getEventCount());
    
    ASSERT(item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(2), item.getBarrierType());
    
    free(pI);
    delete pBase;    
    
}

/**
 * evbFragment
 *
 * Event builder fragment (always has timestamp by definition).
 */
void
RingFactoryTests::evbFragment()
{
    uint8_t payload[10];
    for (int i =0; i < 10; i++) {
     payload[i] = i;
    }
    
    pEventBuilderFragment pI = formatEVBFragment(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        10, payload
    );
    CRingItem*        pBase    = CRingItemFactory::createRingItem(pI);
    CRingFragmentItem item(*pBase);
 
    EQ(static_cast<size_t>(10), item.payloadSize());
    EQ(0, memcmp(payload, item.payloadPointer(), item.payloadSize()));
 
    ASSERT(item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(2), item.getBarrierType());
    
    delete pBase;
    free(pI);
        
}
/**
 * evbUnkPayload
 *
 * EVB fragment with unknown payload type. Always has a timestamp.
 */
void
RingFactoryTests::evbUnkPayload()
{
    uint8_t payload[10];
    for (int i =0; i < 10; i++) {
     payload[i] = i;
    }
    
    pEventBuilderFragment pI = formatEVBFragmentUnknown(
        static_cast<uint64_t>(0x1122334455667788ll), 1, 2,
        10, payload
    );
    CRingItem*        pBase    = CRingItemFactory::createRingItem(pI);
    CUnknownFragment  item(*pBase);
 
    EQ(static_cast<size_t>(10), item.payloadSize());
    EQ(0, memcmp(payload, item.payloadPointer(), item.payloadSize()));
 
    ASSERT(item.hasBodyHeader());
    EQ(static_cast<uint64_t>(0x1122334455667788ll), item.getEventTimestamp());
    EQ(static_cast<uint32_t>(1), item.getSourceId());
    EQ(static_cast<uint32_t>(2), item.getBarrierType());
    
    delete pBase;
    free(pI);    
}
/**
 * glom
 *
 * Glom parameters item.. Never has a timestamp.
 */
void
RingFactoryTests::glom()
{
    pGlomParameters pI = formatGlomParameters(
        static_cast<uint64_t>(100), 1, GLOM_TIMESTAMP_FIRST
    );
    CRingItem* pItem = CRingItemFactory::createRingItem(pI);
    CGlomParameters item(*pItem);
    
    ASSERT(!item.hasBodyHeader());
    EQ(static_cast<uint64_t>(100), item.coincidenceTicks());
    EQ(true, item.isBuilding());
    EQ(CGlomParameters::first, item.timestampPolicy());

}
