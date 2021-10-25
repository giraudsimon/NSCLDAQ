// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "DataFormat.h"
#include "CRingFragmentItem.h"
#include "CUnknownFragment.h"
// Test for event fragment in a ring item:

class FragmentTests : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(FragmentTests);
    CPPUNIT_TEST(emptyCons);
    CPPUNIT_TEST(copyCons);
    
    // Tests for unknown payload types:
    
    CPPUNIT_TEST(unknownEmptyCons);
    CPPUNIT_TEST(unknownCopyCons);
    
    // Being satisfied that unknown is appropriately derived from fragment,
    // tests for CRingFragmentItem with payloads:
    
    CPPUNIT_TEST(payloadCons);
    CPPUNIT_TEST(payloadCopy);
    CPPUNIT_TEST(payloadAccess);
    
    CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
    void emptyCons();
    void copyCons();
    void unknownEmptyCons();
    void unknownCopyCons();
    
    void payloadCons();
    void payloadCopy();
    void payloadAccess();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FragmentTests);

/**
 * emptyCons
 * Test that we can construct a fragment with an empty payload
 */
void
FragmentTests::emptyCons()
{
    CRingFragmentItem item(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, 0, NULL
    );
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(size_t(0), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_FRAGMENT, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment)), pItem->s_header.s_size);
    
    
}
/**
 *copyCons
 * Test copy construction
 */
void
FragmentTests::copyCons()
{
    CRingFragmentItem original(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, 0, NULL
    );
    CRingFragmentItem item(original);
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(size_t(0), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_FRAGMENT, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment)), pItem->s_header.s_size);
    
    // item and original should be equal:
    
    ASSERT(original == item);
    
}

/**
 * unknownwEmptyCons
 * Test that we can construct a fragment with an empty payload
 */
void
FragmentTests::unknownEmptyCons()
{
    CUnknownFragment item(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, 0, 0, NULL
    );
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(size_t(0), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_UNKNOWN_PAYLOAD, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment)), pItem->s_header.s_size);
    
    
}

/**
 * unknownCopyCons
 *
 * Copy construction of an empty unknonw payload type:
 */
void
FragmentTests::unknownCopyCons()
{
    CUnknownFragment originalItem(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, 0, 0, NULL
    );
    CUnknownFragment item(originalItem);
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(size_t(0), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_UNKNOWN_PAYLOAD, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment)), pItem->s_header.s_size);
    
    ASSERT(item == originalItem);
}
/**
 * payloadCons
 *   Test construction with a payload.  We're really intersted in ensuring
 *   the correct size of the item is produced See payloadAccess for recoving
 *   the data itself.
 */
void
FragmentTests::payloadCons()
{
    uint8_t payload[100];
    for (int i =0; i < 100; i++) {
        payload[i] = i;
    }
    CRingFragmentItem item(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, sizeof(payload), payload
    );
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(sizeof(payload), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_FRAGMENT, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment) + sizeof(payload)), pItem->s_header.s_size);
    
}
/**
 * payloadCopy
 *
 * Test copy with payload:
 */
void
FragmentTests::payloadCopy()
{
    uint8_t payload[100];
    for (int i =0; i < 100; i++) {
        payload[i] = i;
    }
    CRingFragmentItem originalItem(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, sizeof(payload), payload
    );
    CRingFragmentItem item(originalItem);
    
    // Exercise the getter member functions:
    
    EQ(static_cast<uint64_t>(0xffeeddccaa998877ll), item.timestamp());
    EQ(static_cast<uint32_t>(1), item.source());
    EQ(sizeof(payload), item.payloadSize());
    EQ(static_cast<uint32_t>(0), item.barrierType());
    pEventBuilderFragment pItem =
        reinterpret_cast<pEventBuilderFragment>(item.getItemPointer());
    EQ(
        reinterpret_cast<void*>(pItem->s_body),
        reinterpret_cast<void*>(item.payloadPointer())
    );
    
    // Base class stuff:
    
    EQ(true, item.hasBodyHeader());
    EQ(EVB_FRAGMENT, item.type());
    EQ(static_cast<uint32_t>(sizeof(EventBuilderFragment) + sizeof(payload)), pItem->s_header.s_size);
        
    ASSERT(item == originalItem);
}
/**
 * payloadAccess
 * 
 * Ensure we can access the payload:
 */
void
FragmentTests::payloadAccess()
{
    uint8_t payload[100];
    for (int i =0; i < 100; i++) {
        payload[i] = i;
    }
    CRingFragmentItem item(
        static_cast<uint64_t>(0xffeeddccaa998877ll), 1, sizeof(payload), payload
    );
    
    pEventBuilderFragment p =
      reinterpret_cast<pEventBuilderFragment>(item.payloadPointer());
    
    for (int i =0; i < sizeof(payload); i++) {
        EQ((int)(payload[i]), (int)(p->s_body[i]));
    }
}
