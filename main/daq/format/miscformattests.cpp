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
# @file   miscformattests.cpp
# @brief  Tests of the format of misc. item types
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
    
    EQ(static_cast<uint32_t>(sizeof(uint32_t)), pItem->s_empty);
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
    // sizeof(BodyHeader) is ok in this context since we're building the data.
    
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + 10),
        pItem->s_header.s_size
    );
    EQ(EVB_FRAGMENT, pItem->s_header.s_type);
    
    // Check the body header
    // sizeof(BodyHeader) is fine because we've contrived the data.
    
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
    // Since we've contrived the data, sizeof(BodyHeader) is just fine.
    
    EQ(
        static_cast<uint32_t>(sizeof(RingItemHeader) + sizeof(BodyHeader) + 10),
        pItem->s_header.s_size
    );
    EQ(EVB_UNKNOWN_PAYLOAD, pItem->s_header.s_type);
    
    // Check the body header
    // Since we've contrived the data  sizeof(BodyHeader) is ok.
    
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
    
    EQ(static_cast<uint32_t>(sizeof(uint32_t)), pItem->s_empty);
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
    EQ(uint32_t(sizeof(uint32_t)), pItem->s_empty);
    free(pItem);
}

void
MiscFormat::abnormalendclass()
{
    CAbnormalEndItem item;
    pRingItem pItem = item.getItemPointer();
    EQ(static_cast<uint32_t>(sizeof(AbnormalEndItem)), pItem->s_header.s_size);
    EQ(ABNORMAL_ENDRUN, pItem->s_header.s_type);
    EQ(uint32_t(sizeof(uint32_t)), pItem->s_body.u_noBodyHeader.s_empty);
    
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