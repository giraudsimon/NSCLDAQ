/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   trigcounttests.cpp
# @brief  Test formatting trigger count items.
# @author <fox@nscl.msu.edu>
*/
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

static uint32_t swal(uint32_t l)
{
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (l & 0xff);
        l = l >> 8;
    }
    return result;
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
   ASSERT((pItem->s_body.u_noBodyHeader.s_mbz == 0) ||
	  (pItem->s_body.u_noBodyHeader.s_mbz == sizeof(uint32_t)));



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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(PHYSICS_EVENT_COUNT, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader)
            + sizeof(PhysicsEventCountItemBody) 
        ), pItem->s_header.s_size
    );
    
    // Check Body header
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1234123412341234ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(1), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(0), pBh->s_barrier);
    
    // Check body
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

    pPhysicsEventCountItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(1234), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(2),  pBody->s_offsetDivisor);
    EQ(static_cast<uint32_t>(stamp), pBody->s_timestamp);
    EQ(static_cast<uint64_t>(0x666444333222ll), pBody->s_eventCount);
    
    free(pItem);
}
