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
# @file   phystests.cpp
# @brief  Test format of physics ring items.
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
  
  EQ(static_cast<uint32_t>(sizeof(uint32_t)), pItem->s_body.u_noBodyHeader.s_empty);

  free(pItem);
}


// Construct an empty timestamped item:

void
PhysicsItemOutput::emptyTimestamped()
{
    pPhysicsEventItem pItem = formatTimestampedEventItem(
        0x1234567876543210ll, 123, 0, 0, NULL);
    
    // Make sure the header is right
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    
    EQ(static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t)), pItem->s_header.s_size);
    EQ(PHYSICS_EVENT, pItem->s_header.s_type);
    
    // Make sure the Body header is right:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pBH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)),   pBH->s_size);
    EQ(static_cast<uint64_t>(0x1234567876543210ll), pBH->s_timestamp);
    EQ(static_cast<uint32_t>(123), pBH->s_sourceId);
    EQ(static_cast<uint32_t>(0),   pBH->s_barrier);
    
    
    // Check the payload.. should be a uint32_t sizeof(uint32_t)/sizeof(uint16_t)
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

    
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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader)
        + sizeof(uint32_t) + 10*sizeof(uint16_t)),
        pItem->s_header.s_size
    );
    EQ(PHYSICS_EVENT, pItem->s_header.s_type);
    
    // Check the body header
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
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
          
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.
     
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
