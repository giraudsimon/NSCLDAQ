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
# @file   scalerformattests.cpp
# @brief  Test formatting of scaler items.
# @author <fox@nscl.msu.edu>
*/
 
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
   ASSERT((pItem->s_body.u_noBodyHeader.s_mbz == 0) ||
	  (pItem->s_body.u_noBodyHeader.s_mbz == sizeof(uint32_t)));
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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(ScalerItemBody)
        ), pItem->s_header.s_size
    );
    EQ(PERIODIC_SCALERS, pItem->s_header.s_type);
    
    // Check body header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1111111122222222ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(5), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(4), pBh->s_barrier);
    
    // Check body:
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(PERIODIC_SCALERS, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(ScalerItemBody)
            + 10*sizeof(uint32_t)                 // (one long in the body)
        ),
        pItem->s_header.s_size    
    );
    
    // Check body header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pBh = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pBh->s_size);
    EQ(static_cast<uint64_t>(0x1111111122222222ll), pBh->s_timestamp);
    EQ(static_cast<uint32_t>(5), pBh->s_sourceId);
    EQ(static_cast<uint32_t>(4), pBh->s_barrier);
    
    
    // Check body:
    // Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

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

