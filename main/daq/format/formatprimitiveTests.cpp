/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  formatprimitiveTests.cpp
 *  @brief:  tests for the formatting functions in DataFormat.h
 */

// daqdev/NSCLDAQ#1030 - changes applied.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "DataFormat.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

// Note some of these tests may be duplicates.

class fmtprimtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(fmtprimtest);
    CPPUNIT_TEST(event_1);
    CPPUNIT_TEST(trgcount_1);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST(scaler_2);
    CPPUNIT_TEST(text_1);
    CPPUNIT_TEST(state_1);
    
    CPPUNIT_TEST(hasbodyhdr_1);
    CPPUNIT_TEST(hasbodyhdr_2);
    CPPUNIT_TEST(hasbodyhdr_3);
    
    CPPUNIT_TEST(bodyptr_1);
    CPPUNIT_TEST(bodyptr_2);
    CPPUNIT_TEST(bodyptr_3);
    CPPUNIT_TEST(bodyptr_4);
    
    CPPUNIT_TEST(bodyheader_1);
    CPPUNIT_TEST(bodyheader_2);
    CPPUNIT_TEST(bodyheader_3);
    CPPUNIT_TEST_SUITE_END();

protected:
    void event_1();
    
    void trgcount_1();
    
    void scaler_1();
    void scaler_2();
    void text_1();
    
    void state_1();

    void hasbodyhdr_1();
    void hasbodyhdr_2();
    void hasbodyhdr_3();
    
    void bodyptr_1();
    void bodyptr_2();
    void bodyptr_3();
    void bodyptr_4();
    
    void bodyheader_1();
    void bodyheader_2();
    void bodyheader_3();
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(fmtprimtest);

void fmtprimtest::event_1()
{
    // Format an event item check that:
    // - ring item size is correct.
    // - There's no body header.
    // - The data are in place.
    
    uint16_t payload[100];
    for (int i =0; i < 100; i++) {
        payload[i] = i;
    }
    pPhysicsEventItem p = formatEventItem(100, payload);
    
    EQ(
       uint32_t(sizeof(RingItemHeader) + 2*sizeof(uint32_t) + sizeof(payload)),
       p->s_header.s_size
    );
    EQ(PHYSICS_EVENT, p->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), p->s_body.u_noBodyHeader.s_empty);
    
    uint16_t* pB = p->s_body.u_noBodyHeader.s_body;
    uint32_t* pWds = (uint32_t*)(pB);
    EQ(uint32_t(102), (*pWds));
    pB = (uint16_t*)(pWds+1);
    for (uint16_t i =0; i < 100; i++) {
        EQ(i, pB[i]);
    }
    free(p);
}

void fmtprimtest::trgcount_1()
{
    // Format a trigger count item and
    // check
    //    right header.
    //    No body header.
    //    Right time, offset and trigger count.
 
    time_t now = time(nullptr);   
    auto p = formatTriggerCountItem(123, now, 456);
    EQ(
       uint32_t(sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(PhysicsEventCountItemBody)),
       p->s_header.s_size
    );
    EQ(PHYSICS_EVENT_COUNT, p->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), p->s_body.u_noBodyHeader.s_empty);
    
    pPhysicsEventCountItemBody pB = &(p->s_body.u_noBodyHeader.s_body);
    EQ(uint32_t(123), pB->s_timeOffset);
    EQ(uint32_t(1),   pB->s_offsetDivisor);
    EQ(uint32_t(now), pB->s_timestamp);
    EQ(uint64_t(456), pB->s_eventCount);
    
    
    free(p);
}
void fmtprimtest::scaler_1()
{
    // Format a scaler item:
    //   - right header.
    //   - no body header.
    //   - right body contents.
    
    uint32_t scalers[32];
    for (int i =0;i < 32; i++) {
        scalers[i] = i * i;
    }
    time_t now = time(nullptr);
    
    auto p = formatScalerItem(32, now, 0, 10, scalers);
    
    EQ(
        uint32_t(sizeof(RingItemHeader) + sizeof(uint32_t) +
                 sizeof(ScalerItemBody) +32*sizeof(uint32_t)),
        p->s_header.s_size
    );
    EQ(PERIODIC_SCALERS, p->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), p->s_body.u_noBodyHeader.s_empty);

    
    pScalerItemBody b= &(p->s_body.u_noBodyHeader.s_body);
    EQ(uint32_t(0), b->s_intervalStartOffset);
    EQ(uint32_t(10), b->s_intervalEndOffset);
    EQ(uint32_t(now), b->s_timestamp);
    EQ(uint32_t(1), b->s_intervalDivisor);
    EQ(uint32_t(1), b->s_isIncremental);
    EQ(uint32_t(32), b->s_scalerCount);
    EQ(0, memcmp(scalers, b->s_scalers, 32*sizeof(uint32_t)));
    
    free(p);
}
void fmtprimtest::scaler_2()
{
    // Timestamped non-incremental scaler item.. now we also
    // have to check the body header.
    
    time_t now = time(nullptr);
    uint32_t scalers[32];
    for (int i =0; i < 32; i++) {
        scalers[i] = 100-i;
    }
    auto p = formatNonIncrTSScalerItem(
        32, now, 10, 20, 0x123456789,  scalers, 2
    );
    
    EQ(uint32_t(
        sizeof(RingItemHeader) + sizeof(BodyHeader) +
        sizeof(ScalerItemBody) + 32*sizeof(uint32_t)),
       p->s_header.s_size
    );
    EQ(PERIODIC_SCALERS, p->s_header.s_type);
    
    pBodyHeader ph = &(p->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(uint32_t(sizeof(BodyHeader)), ph->s_size);
    EQ(uint64_t(0x123456789), ph->s_timestamp);
    EQ(uint32_t(0), ph->s_sourceId);
    EQ(uint32_t(0), ph->s_barrier);
    
    pScalerItemBody pb = &(p->s_body.u_hasBodyHeader.s_body);
    EQ(uint32_t(10), pb->s_intervalStartOffset);
    EQ(uint32_t(20), pb->s_intervalEndOffset);
    EQ(uint32_t(now), pb->s_timestamp);
    EQ(uint32_t(2), pb->s_intervalDivisor);
    EQ(uint32_t(32), pb->s_scalerCount);
    EQ(uint32_t(0), pb->s_isIncremental);
    EQ(0, memcmp(scalers, pb->s_scalers, 32*sizeof(uint32_t)));
    
    
    free(p);
}
void fmtprimtest::text_1()
{
    // Formatting a text item.
    
    time_t now = time(nullptr);
    const char* items[] = {
        "first", "second", "last", nullptr
    };
    uint32_t stringSizes(0);
    for (int i =0; i < 3; i++) {
        stringSizes += strlen(items[i]) + 1;  // +1 for null.
    }
    auto p = formatTextItem(3, now, 123, items, MONITORED_VARIABLES);
    
    EQ(uint32_t(
        sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody) +
        stringSizes
    ), p->s_header.s_size);
    EQ(MONITORED_VARIABLES, p->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), p->s_body.u_noBodyHeader.s_empty);

    
    pTextItemBody b = &(p->s_body.u_noBodyHeader.s_body);
    EQ(uint32_t(123), b->s_timeOffset);
    EQ(uint32_t(now), b->s_timestamp);
    EQ(uint32_t(3),   b->s_stringCount);
    EQ(uint32_t(1),   b->s_offsetDivisor);
    
    const char** pI = items;
    const char* pS = b->s_strings;
    for (int i =0; i < 3; i++) {
        EQ(0, strcmp(*pI, pS));
        pI++;
        pS += strlen(pS) +1;
    }
    
    
    free(p);
}

void fmtprimtest::state_1()
{
    const char* title="This is my title";
    time_t now = time(nullptr);
    
    auto p = formatStateChange(now, 0, 12, title, BEGIN_RUN);
    
    EQ(uint32_t(
        sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(StateChangeItemBody)
    ), p->s_header.s_size);
    EQ(BEGIN_RUN, p->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), p->s_body.u_noBodyHeader.s_empty);

    
    auto b = &(p->s_body.u_noBodyHeader.s_body);
    EQ(uint32_t(12), b->s_runNumber);
    EQ(uint32_t(0), b->s_timeOffset);
    EQ(uint32_t(now), b->s_Timestamp);
    EQ(uint32_t(1),  b->s_offsetDivisor);
    EQ(0, strcmp(title, b->s_title));
    
    free(p);
}

void fmtprimtest::hasbodyhdr_1()
{
    // false if mbz == 0
    
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);
    ASSERT(!hasBodyHeader(&item));
    
}
void fmtprimtest::hasbodyhdr_2()
{
    // false if mbz == sizeof(uint32_t)
    
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);
    ASSERT(!hasBodyHeader(&item));
}
void fmtprimtest::hasbodyhdr_3()
{
    // true if mbz > sizeof(uint32_t).
    
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t)*2;
    ASSERT(hasBodyHeader(&item));
}



void fmtprimtest::bodyptr_1()
{
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);      // No body header.
    void* pBody = bodyPointer(&item);
    void* pExpected = &(item.s_body.u_noBodyHeader.s_body);
    EQ(pExpected, pBody);
    
    // mbz = 0 -> pointer to just past that.
}
void fmtprimtest::bodyptr_2()
{
    // mbz == sizeof(uint32_t) -> pointer to just past that too.
    
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);

    void* pBody = bodyPointer(&item);
    void* pExpected = &(item.s_body.u_noBodyHeader.s_body);
    EQ(pExpected, pBody);
}
void fmtprimtest::bodyptr_3()
{
    RingItem item;
    item.s_body.u_hasBodyHeader.s_bodyHeader.s_size = sizeof(BodyHeader);
    void* pBody = bodyPointer(&item);
    void* pExpected = &(item.s_body.u_hasBodyHeader.s_body);
    EQ(pExpected, pBody);
}
void fmtprimtest::bodyptr_4()
{
    uint8_t  ringItem[100];
    pRingItem pItem = reinterpret_cast<pRingItem>(ringItem);
    
    pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_size=2*sizeof(BodyHeader);
    void *pBody = bodyPointer(pItem);
    void* pExpected = pItem->s_body.u_hasBodyHeader.s_body + sizeof(BodyHeader);
    EQ(pExpected, pBody);
}


void fmtprimtest::bodyheader_1()
{
    // mbz = 0 -> nullptr.
    
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);

    ASSERT(!bodyHeader(&item));
}
void fmtprimtest::bodyheader_2()
{
    // mbz = sizeof(uint32_t) -> nullptr.
    RingItem item;
    item.s_body.u_noBodyHeader.s_empty = sizeof(uint32_t);
    ASSERT(!bodyHeader(&item));
    
}
void fmtprimtest::bodyheader_3()
{
    // mbz = sizeof(uint32_t)* -> not null - pointing to body header.
    
    RingItem item;
    item.s_body.u_hasBodyHeader.s_bodyHeader.s_size = 2*sizeof(uint32_t);
    
    void* expecting = &(item.s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(expecting, bodyHeader(&item));
}


