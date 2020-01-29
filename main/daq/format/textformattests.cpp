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
# @file   textformattests.cpp
# @brief  Test formatting of text items.
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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(MONITORED_VARIABLES, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(TextItemBody)
        ), pItem->s_header.s_size
    );
    
    // Check body header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pB = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pB->s_size);
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pB->s_timestamp);
    EQ(static_cast<uint32_t>(1), pB->s_sourceId);
    EQ(static_cast<uint32_t>(1), pB->s_barrier);
    
    
    // Check body proper.
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.
    
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
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    EQ(MONITORED_VARIABLES, pItem->s_header.s_type);
    EQ(
        static_cast<uint32_t>(
            sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(TextItemBody)
            + stringSizes
        ), pItem->s_header.s_size
    );
    
    // Check body header:
    // Note this is contrived data so sizeof(BodyHeader) is ok.
	
    pBodyHeader pH = &(pItem->s_body.u_hasBodyHeader.s_bodyHeader);
    EQ(static_cast<uint32_t>(sizeof(BodyHeader)), pH->s_size);
    EQ(static_cast<uint64_t>(0x1122334455667788ll), pH->s_timestamp);
    EQ(static_cast<uint32_t>(1), pH->s_sourceId);
    EQ(static_cast<uint32_t>(1), pH->s_barrier);
    
    
    // Check body:
	// Use of u_hasBodyHeader.s_body is ok here because we are generating
	// synthetic events that don't have a body header extension.

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



