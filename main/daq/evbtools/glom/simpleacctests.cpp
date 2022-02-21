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

/** @file:  simpleacctests.cpp
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// Support white box testing.

#define private public
#include "CEventAccumulatorSimple.h"
#undef private
#include <string>
#include <sys/mman.h>    // where memfd_create lives in buster.
#include <unistd.h>
#include <sys/types.h>
#include <fragment.h>
#include <DataFormat.h>

// memory file name - we need something

const std::string memoryFilename("output");

// Default event accumulator settings.
// Note there are tests that will delete the default one
// and create a new one with different settings.
// These are the settings that will be used in setUp

const time_t      maxFlushTime(1);
const size_t      bSize(1024);
const size_t      maxfrags(10);
const CEventAccumulatorSimple::TimestampPolicy policy(
            CEventAccumulatorSimple::first
);
// Data structures:

// This is really a flat fragment with some large fixed capacity

#pragma pack(push, 1)
typedef struct _TestFragment {
    EVB::FragmentHeader s_header;
    uint8_t             s_payload[bSize];
} TestFragment, * pTestFragment;

class simpleacctest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(simpleacctest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(empty_1);
    CPPUNIT_TEST(empty_2);
    CPPUNIT_TEST(add_1);
    CPPUNIT_TEST(add_2);
    CPPUNIT_TEST(add_3);
    
    CPPUNIT_TEST(flush_1);
    CPPUNIT_TEST(flush_2);
    
    CPPUNIT_TEST(flush_3);
    CPPUNIT_TEST(flush_4);
    CPPUNIT_TEST(flush_5);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    
    void empty_1();
    void empty_2();
    
    void add_1();
    void add_2();
    void add_3();

    void flush_1();
    void flush_2();
    
    void flush_3();
    void flush_4();
    void flush_5();
private:
    int m_fd;
    CEventAccumulatorSimple* m_pacc;
public:
    void setUp() {
        m_fd = memfd_create(memoryFilename.c_str(), 0);
        ASSERT(m_fd >= 0);
        m_pacc = new CEventAccumulatorSimple(
            m_fd, maxFlushTime, bSize, maxfrags, policy
        );
    }
    void tearDown() {
        delete m_pacc;
        close(m_fd);
        
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(simpleacctest);
// Whitebox check all the attributes are as I think they should be:
void simpleacctest::construct_1()
{
    EQ(m_fd, m_pacc->m_nFd);
    EQ(maxFlushTime, m_pacc->m_maxFlushTime);
    EQ(policy, m_pacc->m_tsPolicy);
    EQ(bSize, m_pacc->m_nBufferSize);
    EQ(maxfrags, m_pacc->m_nMaxFrags);
    ASSERT(m_pacc->m_pBuffer);
    EQ(size_t(0), m_pacc->m_nBytesInBuffer);
    EQ((uint8_t*)(m_pacc->m_pBuffer), m_pacc->m_pCursor);
    ASSERT(!m_pacc->m_pCurrentEvent);
}
// If there's no data we must not need to flush:

void simpleacctest::empty_1()
{
    m_pacc->finishEvent();
    m_pacc->flushEvents();
    
    // The file should be empty_1...that meens off_t of current position
    // is the same as the rewound fd.
    
    off_t current = lseek(m_fd, 0, SEEK_CUR);
    off_t start   = lseek(m_fd, 0, SEEK_SET);
    EQ(current, start); 
}
// Just putting a fragment in does not output anything.
// only finish/flushing does:
void simpleacctest::empty_2()
{
    // For this we don't need a fragment payload of any specific content:
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
    
    m_pacc->addFragment(
        reinterpret_cast<EVB::pFlatFragment>(&f), 2
    );
    m_pacc->flushEvents();         // output should be empty
    
    off_t current = lseek(m_fd, 0, SEEK_CUR);
    off_t start   = lseek(m_fd, 0, SEEK_SET);
    EQ(current, start); 
}

// Adding that empty fragment should set the current event fields
// correctly.  Note we're still not setting a payload.

void simpleacctest::add_1()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
    
    m_pacc->addFragment(
        reinterpret_cast<EVB::pFlatFragment>(&f), 2
    );
    // Note m_pCurrent event should (and we verified did) point to the
    // m_currentEvent member.  m_pCurrentEvent is only null to show that
    // no event is being built.
    ASSERT(m_pacc->m_pCurrentEvent);
    EQ(f.s_header.s_timestamp, m_pacc->m_currentEvent.s_lastTimestamp);
    EQ(f.s_header.s_timestamp, m_pacc->m_currentEvent.s_timestampTotal);
    EQ(size_t(1), m_pacc->m_currentEvent.s_nFragments);
}
// adding the first item gets the item header, body header and
// fragment byte count set up in the new event.
// For this we need to set up the ring item headers for the
// event but not the actual body payload:

void simpleacctest::add_2()
{
    // Make the fragment:
    
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    CEventAccumulatorSimple::pEventHeader p = m_pacc->m_currentEvent.s_header;
    EQ(PHYSICS_EVENT, p->s_itemHeader.s_type);
    uint32_t totalsize = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        sizeof(EVB::FragmentHeader) + 100;
    EQ(totalsize, p->s_itemHeader.s_size);
    EQ(f.s_header.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(sizeof(BodyHeader), size_t(p->s_bodyHeader.s_size));
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);    // output sourceid.
    EQ(uint32_t(0), p->s_bodyHeader.s_barrier);
    
    // Value of the fragbytes:
    
    uint32_t fragsize = sizeof(uint32_t) + sizeof(EVB::FragmentHeader) + 100;
    EQ(fragsize, p->s_fragBytes);
    
    
}
// adding another fragment should still not force an event to end. we'll
// add the same fragment 2x.

void simpleacctest::add_3()
{
    // Make the fragment:
    
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    off_t begin = lseek(m_fd, 0, SEEK_CUR);   
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x124356800;   // change up the ts and 
    f.s_header.s_sourceId  = 2;             // sid.
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    
    m_pacc->flushEvents();             // Should be no output.
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    EQ(begin, end);
    
    // Whitebox assertions:
    
    CEventAccumulatorSimple::Event& e((m_pacc->m_currentEvent));
    EQ(f.s_header.s_timestamp, e.s_lastTimestamp);
    EQ(uint64_t(0x124356789 + 0x124356800), e.s_timestampTotal);
    EQ(size_t(2), e.s_nFragments);
    
    CEventAccumulatorSimple::EventHeader& eh(*(e.s_header));
    
    uint32_t totalsize = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        2*(sizeof(EVB::FragmentHeader) + 100);
    EQ(totalsize, eh.s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, eh.s_itemHeader.s_type);
    EQ(uint64_t( 0x124356789), eh.s_bodyHeader.s_timestamp);  // sb from first.
    EQ(uint32_t(10), eh.s_bodyHeader.s_sourceId);    // STill output sid.
    
    uint32_t payloadSize = sizeof(uint32_t) + 2*(sizeof(EVB::FragmentHeader) + 100);
    EQ(payloadSize, eh.s_fragBytes);
    
    // The cursor should have advanced by all those bytes too:
    
    EQ(
       ptrdiff_t(totalsize),
       (m_pacc->m_pCursor - reinterpret_cast<uint8_t*>(m_pacc->m_pBuffer))
    );
    
    
}
// Make a single fragment event and do a forced flush.  there's no current
// event but the cursor should still be advanced.

void simpleacctest::flush_1()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    off_t begin = lseek(m_fd, 0, SEEK_CUR);   
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    
    m_pacc->finishEvent();           // Close off the event.
    
    // There's no current event but the cursor is still advanced.
    
    ASSERT(!m_pacc->m_pCurrentEvent);
    off_t end = lseek(m_fd, 0, SEEK_CUR);
    EQ(begin, end);               // NOthing got written.
    
    ptrdiff_t size = sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t) +
        (sizeof(EVB::FragmentHeader) + 100);
    EQ(size,
       (m_pacc->m_pCursor - reinterpret_cast<uint8_t*>(m_pacc->m_pBuffer))
    );
    
}
// Let's make sure the headers got finished properly when an event is finished.


void simpleacctest::flush_2()
{
    TestFragment f;
    f.s_header.s_timestamp = 0x124356789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    uint32_t payload = sizeof(uint32_t) + 100 + sizeof(EVB::FragmentHeader);
    uint32_t total   = sizeof(RingItemHeader) + sizeof(BodyHeader) + payload;
    
    EQ(total, p->s_itemHeader.s_size);
    EQ(PHYSICS_EVENT, p->s_itemHeader.s_type);
    
    EQ(f.s_header.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);
    EQ(sizeof(BodyHeader), size_t(p->s_bodyHeader.s_size));
    EQ(uint32_t(0), p->s_bodyHeader.s_barrier);
    
    EQ(payload, p->s_fragBytes);
}
// two frags with first policy gives first timestamp.

void simpleacctest::flush_3()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::first;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t(0x123456789), p->s_bodyHeader.s_timestamp);
    
}

// two frags with last policy gives last timestamp.
void simpleacctest::flush_4()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::last;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t(0x123456800), p->s_bodyHeader.s_timestamp);
}

// two frags with average pollicy gives average timestamp
void simpleacctest::flush_5()
{
    m_pacc->m_tsPolicy = CEventAccumulatorSimple::average;
    TestFragment f;
    f.s_header.s_timestamp = 0x123456789;
    f.s_header.s_sourceId  = 1;
    f.s_header.s_size      = 100;
    f.s_header.s_barrier   = 0;
 
    pRingItemHeader pHeader = reinterpret_cast<pRingItemHeader>(f.s_payload);
    pBodyHeader     pbHeader= reinterpret_cast<pBodyHeader>(pHeader+1);
    
    pHeader->s_type = PHYSICS_EVENT;
    pHeader->s_size = 100;
    pbHeader->s_size = sizeof(BodyHeader);
    pbHeader->s_timestamp = f.s_header.s_timestamp;
    pbHeader->s_sourceId  = f.s_header.s_sourceId;
    pbHeader->s_barrier   = f.s_header.s_barrier;
    
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    f.s_header.s_timestamp = 0x123456800;
    pbHeader->s_timestamp    = 0x123456800;
    m_pacc->addFragment(reinterpret_cast<EVB::pFlatFragment>(&f), 10);
    m_pacc->finishEvent();
    
    CEventAccumulatorSimple::pEventHeader p =
        reinterpret_cast<CEventAccumulatorSimple::pEventHeader>(m_pacc->m_pBuffer);
    EQ(uint64_t((0x123456789 + 0x123456800)/2), p->s_bodyHeader.s_timestamp);
}