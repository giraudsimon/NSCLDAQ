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
# @file   statechangetests.cpp
# @brief  Tests for the format of state change items.
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
  
  // EQMSG("State change item sizse", static_cast<uint32_t>(sizeof(StateChangeItem)), pItem->s_header.s_size);
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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader)
            + sizeof(StateChangeItemBody)), pItem->s_header.s_size
    );
    EQ(BEGIN_RUN, pItem->s_header.s_type);
    
    // Check body header
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pH->s_size);
    EQ(static_cast<uint64_t>(8877665544332211ll), pH->s_timestamp);
    EQ(static_cast<uint32_t>(12), pH->s_sourceId);
    EQ(static_cast<uint32_t>(34), pH->s_barrier);
    

    // Check body contents.
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

    pStateChangeItemBody pBody = &(pItem->s_body.u_hasBodyHeader.s_body);
    EQ(static_cast<uint32_t>(12), pBody->s_runNumber);
    EQ(static_cast<uint32_t>(0), pBody->s_timeOffset);
    EQ(static_cast<uint32_t>(stamp), pBody->s_Timestamp);
    EQ(static_cast<uint32_t>(1), pBody->s_offsetDivisor);
    EQ(0, strcmp(titleString, pBody->s_title));
    
    free(pItem);
}
