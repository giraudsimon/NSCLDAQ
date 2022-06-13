// Template for a test suite.
 
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "DataFormat.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "CAbnormalEndItem.h"
#include <iostream>
#include <utils.h>
  
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
