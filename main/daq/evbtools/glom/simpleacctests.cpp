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
    uint8_t             s__payload[bSize];
} TestFragment, * pTestFragment;

class simpleacctest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(simpleacctest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(empty_1);
    CPPUNIT_TEST(empty_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    void empty_1();
    void empty_2();
    
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