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

/** @file:  CEventAccumulatorSimple.cpp
 *  @brief: Implement the CEventAccumulatorSimple class.
 */

#include "CEventAccumulatorSimple.h"
#include <stdexcept>
#include <string.h>
#include <io.h>


/**
 * constructor
 *    For the most part initialize data, however we also
 *    allocate the output buffer:
 *
 *  @param fd  - file descriptor on which output is done.
 *  @param maxFlushTime - Maximum time between full event flushes.
 *  @param bufferSize - number of bytes to allocate to the output buffer.
 *  @param maxfrags - Maximum fragments in an event.
 *  @param policy   - Policy used to compute the final event timestamp.
 */
CEventAccumulatorSimple::CEventAccumulatorSimple(
    int fd, time_t maxFlushTime, size_t bufferSize, size_t maxfrags,
    TimestampPolicy policy
) :
    m_nFd(fd),
    m_maxFlushTime(maxFlushTime),
    m_tsPolicy(policy),
    m_nBufferSize(bufferSize),
    m_nMaxFrags(maxfrags),
    m_pBuffer(nullptr),
    m_nBytesInBuffer(0),
    m_pCursor(nullptr),
    m_pCurrentEvent(nullptr)
{
    // Allocate the buffer:
    
    m_pBuffer = malloc(bufferSize);
    if (!m_pBuffer) {
        throw std::bad_alloc();
    }
    m_pCursor = reinterpret_cast<uint8_t*>(m_pBuffer);
    m_lastFlushTime = time(nullptr);
    
}
/**
 * Destructor, finish any pending event and flush before
 * destroying the buffer.
 */
CEventAccumulatorSimple::~CEventAccumulatorSimple()
{
    finishEvent();
    flushEvents();
    free(m_pBuffer);
}
/**
 * addFragment
 *    Add a fragment to the current event
 *    If necessary a new event and new body header are created.
 *    If an event is in progress, and either the output sourceid or
 *    the ring item type in the fragment differ from the
 *    one being built, the one being built is finished and a new event
 *    is started.
 *    If the fragment would make the max fragment count the event is
 *    ended first and a new one started.
 *    If the fragment + event header can't fit in the empty buffer,
 *    an exception (std::range_error) is thrown.
 *    
 * @param pFrag - fragment to add to the event.
 * @param outputSid - output sourceid:
 */
 void
 CEventAccumulatorSimple::addFragment(EVB::pFlatFragment pFrag, int outputSid)
 
{
    size_t fragSize = fragmentSize(pFrag);
    if ((sizeof(EventHeader) + fragSize) > m_nBufferSize) {
        throw std::range_error("Fragment won't fit in the accumulator buffer");
    }
    if (mustFinish(pFrag, outputSid)) {
        finishEvent();
    }
    if (mustFlush(pFrag)) {
        flushEvents();
    }
    if (!m_pCurrentEvent) {

        newEvent(pFrag, outputSid); // Sets ts if first and output sid.        
    }
    
    // Add the fragment to the buffer at the cursor and update
    // all the book keeping stuff.
    
    memcpy(m_pCursor, pFrag, fragSize);
    m_pCurrentEvent->s_lastTimestamp = pFrag->s_header.s_timestamp;
    m_pCurrentEvent->s_timestampTotal += pFrag->s_header.s_timestamp;
    m_pCurrentEvent->s_nFragments++;
    m_pCurrentEvent->s_header->s_itemHeader.s_size += fragSize;
    m_pCurrentEvent->s_header->s_fragBytes         += fragSize;
    
    m_pCursor += fragSize;
    m_nBytesInBuffer += fragSize;
    

 }
 /**
  * addOOBFragment
  *    Adds a fragment that would be an out of band fragment.
  *    - Flushes the events that have been completely buffered
  *    - Writes the body of the item out with its source id
  *      rewritten as requested.
  *
  *  @param pFrag - pointer to the fragment.
  *  @param ouputSid - Source id desired on the output item.
  *  @note This assumes the payload of the fragment is a ring item. 
  */
 void
 CEventAccumulatorSimple::addOOBFragment(EVB::pFlatFragment pFrag, int outputSid)
 {
    flushEvents();
    
    pRingItem pItem = reinterpret_cast<pRingItem>(pFrag->s_body);
    
    // If there's a body header we need to re-write the sid.  Otherwise
    // just write it all out:
    // the > sizeof(uint32_t) handles handles the 11.3 zero
    // and 11.4 sizeof(uint32_t) value
    //
    if (pItem->s_body.u_noBodyHeader.s_mbz > sizeof(uint32_t)) {
        
        pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId = outputSid;
    }
    io::writeData(m_nFd, pItem, pItem->s_header.s_size);    
}
/**
 * finshEvent
 *   - This does not write data.  What it does is
 *   - Figure out the final timestamp for the current event and
 *   - Null out the current event pointer so the next fragment
 *     will start a new event.
 */
void
CEventAccumulatorSimple::finishEvent()
{
    // THere must be a current event:
    
    if (m_pCurrentEvent) {
        // Set the timestamp if the policy isn't first.
        
        uint64_t ts = m_pCurrentEvent->s_header->s_bodyHeader.s_timestamp;
        if (m_tsPolicy == last) {
            ts = m_pCurrentEvent->s_lastTimestamp;
        } else if (m_tsPolicy == average) {
            ts = m_pCurrentEvent->s_timestampTotal / m_pCurrentEvent->s_nFragments;
        }
        m_pCurrentEvent->s_header->s_bodyHeader.s_timestamp = ts;
        
        // Null out so a new one must be started:
        
        m_pCurrentEvent = nullptr;
    }
}
/**
 * flushEvents
 *   Writes all complete events to file.
 *
 *   - There are two cases:
 *     1.  There's no current event in which case all bytes
 *         in the buffer are written.
 *     2. There's a current event in which case bytes are written up
 *        to the current event and then the entire current event
 *        is slid down to the start of the buffer memmove is used
 *        since there's a faint possibility of overlap.
 */
void
CEventAccumulatorSimple::flushEvents()
{
    // THere must be some data to flush:
    
    if (m_nBytesInBuffer) {
        if (m_pCurrentEvent) {
            
            // Figure out where the current event starts and, from that,
            // Figure out the # bytes to write:
            
            uint8_t* pEnd = reinterpret_cast<uint8_t*>(m_pCurrentEvent->s_header);
            ptrdiff_t nBytes = (pEnd - reinterpret_cast<uint8_t*>(m_pBuffer));
            
            // There must be at least one complete event to flush:
            
            if (nBytes) {
                io::writeData(
                    m_nFd, m_pBuffer,
                    (pEnd - reinterpret_cast<uint8_t*>(m_pBuffer))
                );
                // The ring item header size field is a running count
                // of the size of the event being built so:
                
                uint32_t eventSize = m_pCurrentEvent->s_header->s_itemHeader.s_size;
                memmove(m_pBuffer, m_pCurrentEvent->s_header,  eventSize);
                
                // fix the pointer to the current event and the insertion
                // cursor as well as bytes in buffer:
                
                m_pCurrentEvent->s_header =
                    reinterpret_cast<pEventHeader>(m_pBuffer);
                m_pCursor = reinterpret_cast<uint8_t*>(m_pBuffer);
                m_pCursor += eventSize;
                m_nBytesInBuffer = eventSize;
            }
        } else {
            // Simple case:
            
            io::writeData(m_nFd, m_pBuffer, m_nBytesInBuffer);
            m_pCursor = reinterpret_cast<uint8_t*>(m_pBuffer);
            m_nBytesInBuffer = 0;
        }
    }
    // This update of the last flush time prevents a continuous
    // attempt to flush if the buffer is essentially empty
    
    m_lastFlushTime = time(nullptr);  
}
/////////////////////////////////////////////////////////////////
// Utilities

/**
 * mustFinish
 *   Determines if the current event must be finished given the
 *   next fragment
 *   Obviously there must be a current event
 *   -  Finish if the type or output source id changed.
 *   -  Finish if adding would exceed maxfragments.
 *   -  Finish if adding would make the whole event exceed the
 *      output buffer size.
 * @param pFrag - pointer to the fragment being added.
 * @param outputSid - Output source id.
 * @return bool -true if the caller should finishEvent()
 */
bool
CEventAccumulatorSimple::mustFinish(
    EVB::pFlatFragment pFrag, int outputSid
)
{
    bool result(false);
    if (m_pCurrentEvent) {
        pRingItemHeader pFragItem =
            reinterpret_cast<pRingItemHeader>(pFrag->s_body);
        if (pFragItem->s_type !=
            m_pCurrentEvent->s_header->s_itemHeader.s_type
           ) {
            result = true;
        }
        if (m_pCurrentEvent->s_header->s_bodyHeader.s_sourceId != outputSid) {
            result = true;
        }
        if (m_pCurrentEvent->s_nFragments >= m_nMaxFrags) {
            result = true;
        }
        uint32_t fragSize = fragmentSize(pFrag);
        if ((fragSize + m_pCurrentEvent->s_header->s_itemHeader.s_size) > m_nBufferSize)  {
            result = true;
        }
    } 
    return result;
}
/**
 * mustFlush
 *    Determines if it's time to flush any events
 *    This is the case if:
 *    - The next fragment would overflow the buffer if there was
 *      not a flush done. Or:
 *    - The time since the last flush was larger than the m_maxFlushTime
 *   @param pFrag - pointer to the next fragment to put in the buffer.
 *   @return bool - true if the events must be flushed.   
 */
bool
CEventAccumulatorSimple::mustFlush(EVB::pFlatFragment pFrag)
{
    if (fragmentSize(pFrag) > (m_nBufferSize - m_nBytesInBuffer)) {
        return true;
    }
    time_t now = time(nullptr);
    if ((now - m_lastFlushTime) >- m_maxFlushTime) {
        return true;
    }
    
    return false;
}
/**
 * newEvent
 *    Do all the book keeping to make an initial event:
 *    - Point m_pCurrentEvent at m_pCursor
 *    - Fill in what we can of the header.
 *    - Point m_pCursor past the header
 *    - and adjust the bytes in buffer count.
 * @param pFrag - Pointer to the first fragment of the event.
 * @param outputSid - desired source id for the output event.
 */
void
CEventAccumulatorSimple::newEvent(EVB::pFlatFragment pFrag, int outpuSid)
{
    if(m_pCurrentEvent) {
        throw std::logic_error("new event but there's already a current event!!!");
    }
    m_pCurrentEvent = &m_currentEvent;
    // Fill in the current event:
    m_pCurrentEvent->s_header = reinterpret_cast<pEventHeader>(m_pCursor);
    m_pCurrentEvent->s_lastTimestamp = pFrag->s_header.s_timestamp;
    m_pCurrentEvent->s_timestampTotal = 0;
    m_pCurrentEvent->s_nFragments  = 0;

    
    // We assume the fragment payload is a ring item:
    // Fill in what we can of the event header now:
    
    pRingItemHeader pItem = reinterpret_cast<pRingItemHeader>(pFrag->s_body);
    pEventHeader     pEvent = m_pCurrentEvent->s_header;
    
    pEvent->s_itemHeader.s_type = pItem->s_type;
    pEvent->s_itemHeader.s_size = sizeof(EventHeader);
    pEvent->s_bodyHeader.s_size      = sizeof(BodyHeader);
    pEvent->s_bodyHeader.s_timestamp = pFrag->s_header.s_timestamp;
    pEvent->s_bodyHeader.s_sourceId = pFrag->s_header.s_sourceId;
    pEvent->s_bodyHeader.s_barrier  = pFrag->s_header.s_barrier;
    pEvent->s_fragBytes = 0;
    
    m_pCursor += sizeof(EventHeader);
    m_nBytesInBuffer += sizeof(EventHeader);
}
/**
 * fragmentSize
 *    Given a pointer to a flat fragment, computes and returns
 *    the size of the fragment in byts.
 * @param pFrag - pointer to the fragment.
 * @return size_t
 */
size_t
CEventAccumulatorSimple::fragmentSize(EVB::pFlatFragment pFrag)
{
    return sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size;
}