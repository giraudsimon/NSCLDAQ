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

/** @file:  CEventAccumulator.cpp
 *  @brief: Impelement the multi-event accumulator class.
 */
#include "CEventAccumulator.h"
#include <fragment.h>
#include <io.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <new>
#include <stdexcept>

/**
 * constructor
 *    Data initialization and allocation of stuff we know how to allocate.
 *
 * @param fd           - File descriptor to which writes will be done.
 * @param maxFlushTime - # of seconds after which a flush is forced.
 * @param bufferSize   - Size of the event accumulation buffer.
 * @param maxfrags     - Maximum # fragment sin an event.
 * @param policy       - Timestamp assignment policy.
 */
CEventAccumulator::CEventAccumulator(
    int fd, time_t maxFlushTime, size_t bufferSize, size_t maxfrags,
    TimestampPolicy policy
) :
    m_nFd(fd), m_maxFlushTime(maxFlushTime), m_tsPolicy(policy),
    m_pBuffer(nullptr), m_nBufferSize(bufferSize), m_nMaxFrags(maxfrags),
    m_nBytesInBuffer(0), m_pIoVectors(nullptr), m_nMaxIoVecs(0), 
    m_nIoVecs(0), m_pCurrentEvent(0)
{
    // Allocate the fragment buffer:
    
    m_pBuffer = malloc(m_nBufferSize);
    if (!m_pBuffer) {
        throw std::bad_alloc();
    }
    // We can pre-allocate the free frags at m_nMaxFrags,
    
    for (int i = 0; i < m_nMaxFrags; i++) {
        m_freeFrags.push_back(new EventInformation);
    }
    // Start the flush timer:
    
    m_lastFlushTime = time(nullptr);
}
/**
 * destructor
 *    Free dynamic storage... we first flush any pending fragments.
 */
CEventAccumulator::~CEventAccumulator()
{
    flushEvents();
    
    free(m_pBuffer);
    while(!m_freeFrags.empty() ) {
        delete m_freeFrags.front();
        m_freeFrags.pop_front();
    }
    while (!m_fragsInBuffer.empty()) {
        delete m_fragsInBuffer.front();
        m_fragsInBuffer.pop_front();
    }
    free(m_pIoVectors);
    
}
/**
 * addFragment
 *    If we don't have an current event allocate one.
 *    If the fragment would overflow the buffer, flush the events.
 *    If the fragment would still overflow the buffer, finish the
 *    current event.
 *    If the fragment would _still_ overflow the buffer, throw an exception.
 *
 *    Add the fragment to the event.
 *    If the added fragment hits the maximum fragment size, commit the
 *    finish the event.
 *
 *  @param pFrag     - the framgment to add.
 *  @param outputSid - The source id to use on any new event.
 */
void
CEventAccumulator::addFragment(EVB::pFlatFragment pFrag, int outputSid)
{
    if (!m_pCurrentEvent) {
        m_pCurrentEvent = allocEventInfo(pFrag, outputSid);
    }
    // If the current item type is different from the type of the current
    // event, finish the event.
    
    if (itemType(pFrag) != m_pCurrentEvent->s_eventHeader.s_itemHeader.s_type) {
        finishEvent();
        m_pCurrentEvent = allocEventInfo(pFrag, outputSid);
    }
    
    // If this fragment won't fit flush the complete events.
    
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        flushEvents();
    }
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        finishEvent();
        flushEvents();
        m_pCurrentEvent = allocEventInfo(pFrag, outputSid);
    }
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        throw std::range_error("Encountered a fragment bigger than the buffer");    
    }
    //  Add the fragment and finish if it exceeds the
    //  maximum fragment count:
    
    appendFragment(pFrag);
    if (m_pCurrentEvent->s_eventInfo.s_nFragments == m_nMaxFrags) {
        finishEvent();
    }
    // Finally, if now differs from m_nLastFlushTim by more than
    // m_maxFlushTime, force a flush as well:
    
    if ((time(nullptr) - m_lastFlushTime) >= m_maxFlushTime) {
        flushEvents();
    }
}
/** finishEvent
 *    Finish accumulating an event.  This should be called after the
 *    last fragment in an event has been added.
 *    - Calling without a current event is a no-op.
 *    - The timestamp is determined and put in the body header.
 *    - The current event is appended to the m_fragsInBuffer deque.
 *    - The current event is nulled out.
 */
void
CEventAccumulator::finishEvent()
{
    // If first or last, appendFragment has been taking care of us so:
    
    if (m_tsPolicy == average) {
        m_pCurrentEvent->s_eventHeader.s_bodyHeader.s_timestamp =
            m_pCurrentEvent->s_eventInfo.s_TimestampTotal /
            m_pCurrentEvent->s_eventInfo.s_nFragments;
    }
    m_fragsInBuffer.push_back(m_pCurrentEvent);
    m_pCurrentEvent = nullptr;
}
/**
 * flushEvents
 *    - Write all complete events out to file.
 *    - Slide the partial event down to the front of the buffer.
 */
void
CEventAccumulator::flushEvents()
{
    // Each event requires an I/O vector for the header,
    // its body header and the body itself:
    
    sizeIoVecs(m_fragsInBuffer.size()*3);
    makeIoVectors();
    io::writeDataVUnlimited(m_nFd, m_pIoVectors, m_nIoVecs);
    
    // transfer the event infos to the free list.
    
    m_freeFrags.insert(
        m_freeFrags.end(), m_fragsInBuffer.begin(), m_fragsInBuffer.end()
    );
    m_fragsInBuffer.clear();
}