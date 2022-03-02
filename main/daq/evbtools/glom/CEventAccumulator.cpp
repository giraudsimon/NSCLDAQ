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
    TimestampPolicy policy) :
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
        reserveSize();
    }
    // If the current item type is different from the type of the current
    // event, finish the event.
    
    if (itemType(pFrag) != m_pCurrentEvent->s_eventHeader.s_itemHeader.s_type) {
        finishEvent();
        m_pCurrentEvent = allocEventInfo(pFrag, outputSid);
        reserveSize();
    }
    
    // If this fragment won't: fit flush the complete events.
    
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        flushEvents();
    }
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        finishEvent();
        flushEvents();
        m_pCurrentEvent = allocEventInfo(pFrag, outputSid);
        reserveSize();
    }
    if ((sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size) > freeSpace()) {
        throw std::range_error("Encountered a fragment bigger than the buffer");    
    }
    //  Add the fragment and finish if it exceeds the
    //  maximum fragment count:
    
    appendFragment(pFrag);
   
    // Finally, if now differs from m_nLastFlushTim by more than
    // m_maxFlushTime, force a flush as well:
    
    if ((time(nullptr) - m_lastFlushTime) >= m_maxFlushTime) {
        flushEvents();
    }
}
/**
 * addOOBFragment
 *    Adds an out of band fragment.  Out of band fragments:
 *    - Cause the assembled events to be flushed to file.
 *    - Don't affect any event being assembled.
 *    - Bodies are output.  If the fragment has a body header,
 *      it's sid is changed to the outputSid
 *
 * @param pFrag - pointer to the out of band fragment.
 * @param outputSid - source id to use for the output fragment.
 */
void
CEventAccumulator::addOOBFragment(EVB::pFlatFragment pFrag, int outputSid)
{
    flushEvents();           // Write any buffered, completed events.
    
    // THe remainder of thsi assumes the fragment body is a ring item:
    

    pRingItem pItem = reinterpret_cast<pRingItem>(pFrag->s_body);
    
    // If there's no body header just output as is:
    
    if (pItem->s_body.u_noBodyHeader.s_mbz == 0) {
        io::writeData(m_nFd, pItem, pItem->s_header.s_size);
    } else {
        // Otherwise we need to modify the sid. We assume this is an
        // infrequent case.
        
        uint8_t itemCopy[pItem->s_header.s_size];
        memcpy(itemCopy, pItem, pItem->s_header.s_size);
        pItem = reinterpret_cast<pRingItem>(itemCopy);  // Maka  copy...
        pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId = outputSid;
        io::writeData(m_nFd, pItem, pItem->s_header.s_size);
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
    if (!m_pCurrentEvent) return;
    
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
    
    
    // Write an event at a time:
#ifdef INCREMENTAL_WRITE    
    int n = m_nIoVecs;
    auto v = m_pIoVectors;
    int step = 1024+512;             // This is the largest single pass value.
    for (int i =0; i < m_nIoVecs; i += step) {
      int nWrites = step;
      if (nWrites > n) nWrites = n;
      io::writeDataVUnlimited(m_nFd, v, nWrites);
      v += nWrites;
      n -= nWrites;
    }
#else
    io::writeDataVUnlimited(m_nFd, m_pIoVectors, m_nIoVecs);
#endif
  
    // transfer the event infos to the free list.
    
    m_freeFrags.insert(
        m_freeFrags.end(), m_fragsInBuffer.begin(), m_fragsInBuffer.end()
    );
    m_fragsInBuffer.clear();
    
    slideCurrentEventToFront();
    m_lastFlushTime = time(nullptr);
}
/**
 * allocEventInfo
 *    Get an event info struct - either from the free list or by
 *    newing it into existence.  We can fill in a few things;
 *    under the assumption this will become the current event:
 *    -  s_eventHeader.s_itemHeader.s_size <--- ring item size + body header size.
 *    -  s_eventHeader.s_itemHeader.s_type <--- Payload type from fragment.
 *    -  s_eventHeader.s_bodyHeader.s_timestamp <-- frag timestamp if first.
 *    -  s_eventHeader.s_bodyHeader.s_sourceId <-- from the parameter.
 *    -  s_eventHeader.s_bodyHeader.s_size     <-- sizeof(BodyHeaer)
 *    -  s_eventInfo.s_nBytes   <-- 0
 *    -  s_eventInfo.s_nFragments <- 0
 *    -  s_eventInfo.s_timestampTotal <--0.
 *    -  s_bodyStart <-- m_pBufer + m_nBytesInBuffer.
 *    -  s_pInsertionPoint <-- s_bodyStart.
 *
 *  @param pFrag - fragment for which this item is allocated (used to init).
 *  @param sid   - Source Id to give to the fragments.
 *  @return - Pointer to the allocated info struct.
 *  @throw std::bad_alloc if allocation fails.
 */
CEventAccumulator::pEventInformation
CEventAccumulator::allocEventInfo(EVB::pFlatFragment pFrag, int sid)
{
    pEventInformation result(nullptr);
    
    if (m_freeFrags.empty()) {
        result = new EventInformation;
    } else {
        result = m_freeFrags.front();
        m_freeFrags.pop_front();
    }
    // New should throw but:
    
    if (!result) {
        throw std::bad_alloc();
    }
    // Initialize the output ring item header:
    // Note that this use of sizeof(BodyHeader) is ok because we don't care
    // about the contents of the body.  In this use any body header extension
    // will just be treated as part of the body.
    
    result->s_eventHeader.s_itemHeader.s_size = sizeof(RingItemHeader) + sizeof(BodyHeader);
    result->s_eventHeader.s_itemHeader.s_type = itemType(pFrag);
    
    // Now the body header:
    
    if (m_tsPolicy == first) {
        result->s_eventHeader.s_bodyHeader.s_timestamp =
            pFrag->s_header.s_timestamp;
    } else {
        result->s_eventHeader.s_bodyHeader.s_timestamp = NULL_TIMESTAMP;
    }
    
    // In this case we're building the body header for the output ring item
    // so we know there's no body header extension so the use of
    // sizeof(BodyHeader) is ok.
    
    result->s_eventHeader.s_bodyHeader.s_sourceId = sid;
    result->s_eventHeader.s_bodyHeader.s_size     = sizeof(BodyHeader);
    result->s_eventHeader.s_bodyHeader.s_barrier  = 0;
    
    // The event information struct:
    
    result->s_eventInfo.s_nBytes = 0;
    result->s_eventInfo.s_nFragments = 0;
    result->s_eventInfo.s_TimestampTotal  = 0;
    
    // The start and insertion pointers:
    
    uint8_t* p = static_cast<uint8_t*>(m_pBuffer) + m_nBytesInBuffer;
    result->s_pBodyStart = p;
    result->s_pInsertionPoint = p;
    
    // All ready to roll.
    
    return result;
}
/**
 * freeEventInfo
 *    Just put the event information block into the free frags deque.
 * @param pInfo - the item to free.
 */
void
CEventAccumulator::freeEventInfo(pEventInformation pInfo)
{
    m_freeFrags.push_back(pInfo);
}
/**
 * sizeIoVecs
 *    Make sure there are sufficient I/O vectors in the m_pIoVectors.
 *
 *  @param nVecs -number needed:
 *  @throw std::bad_alloc - needed more but malloc failed.
 */
void
CEventAccumulator::sizeIoVecs(size_t nVecs)
{
    if (nVecs > m_nMaxIoVecs) {
        free(m_pIoVectors);           // No data to save.
        m_pIoVectors = static_cast<iovec*>(malloc(nVecs * sizeof(iovec)));
        if (!m_pIoVectors) {
            throw std::bad_alloc();
        }
        m_nMaxIoVecs = nVecs;
    }
}
/**
 * makeIoVectors
 *    Given the events descsribed by m_fragsInBuffer,
 *    creates the I/O vectors neeed to write them with e.g. writev
 *    or rather io::writeDataVUnlimited.
 *
 *   @return size_t number of iovec items created.  This also gets stored in
 *           m_nIovecs.
 */
size_t
CEventAccumulator::makeIoVectors()
{
    m_nIoVecs = 0;                 // Also used as index:
   
    
    for (auto& info : m_fragsInBuffer) {
        // We need three vectors:
        
        // Ring item header:
        
        m_pIoVectors[m_nIoVecs].iov_base = &(info->s_eventHeader.s_itemHeader);
        m_pIoVectors[m_nIoVecs].iov_len   = sizeof(RingItemHeader);
        
        // Body header -- again in this treatment, any body header extension
        // is just treated as part of the body and, since we don't care aout
        // the body contents that's just fine.
        
        m_pIoVectors[m_nIoVecs+1].iov_base = &(info->s_eventHeader.s_bodyHeader);
        m_pIoVectors[m_nIoVecs+1].iov_len  = sizeof(BodyHeader);
        
        // The Event itself:
        
        m_pIoVectors[m_nIoVecs+2].iov_base = info->s_pBodyStart;
        m_pIoVectors[m_nIoVecs+2].iov_len  = info->s_eventInfo.s_nBytes;
        
        m_nIoVecs += 3;
    }
    return m_nIoVecs;
}
/**
 * slideCurrentEventToFront
 *    Called after writing data.  If there's a current event,
 *    its contents must be repositioned to the front of the buffer
 *    so that new data can be filled in after it.
 *    -  The current event's m_pBodyStart -> front of buffer.
 *    -  The current event's m_pInsertionPoint -> size of event after the buffer.
 *    -  The m_nBytesInBuffer -> size accumulated in the event so far.
 */
void
CEventAccumulator::slideCurrentEventToFront()
{
    if (m_pCurrentEvent) {
        memcpy(
            m_pBuffer, m_pCurrentEvent->s_pBodyStart,
            m_pCurrentEvent->s_eventInfo.s_nBytes
        );
        m_pCurrentEvent->s_pBodyStart = m_pBuffer;
        m_pCurrentEvent->s_pInsertionPoint =
            static_cast<uint8_t*>(m_pBuffer) +
            m_pCurrentEvent->s_eventInfo.s_nBytes;
        m_nBytesInBuffer = m_pCurrentEvent->s_eventInfo.s_nBytes;
            
    } else {
        m_nBytesInBuffer = 0;                // No data in buffer.
    }
    
}
/**
 * freeSpace
 *   Computes the free space in the buffer.
 *   - m_nBufferSize - is the total size of the buffer.
 *   - m_nBytesInBuffer - is the number of bytes in the buffer
 *                        This is maintained on a per-fragment/event basis.
 * @return size_t - number of unused bytes in the buffer:
 */
size_t
CEventAccumulator::freeSpace()
{
    return  m_nBufferSize - m_nBytesInBuffer;
}
/**
 * itemType
 *    @param pFrag - pointer to a flattened fragment that has a ring item
 *                   as a payload.
 *    @return uint32_t - the ring item type.
 */
uint32_t
CEventAccumulator::itemType(EVB::pFlatFragment pFrag)
{
    pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(pFrag->s_body);
    return pH->s_type;
}
/**
 * appendFragment
 *    Appends a fragment to the m_pCurrentEvent.
 *    - If timestamp policy is last - updates the timestamp, sums the stamp into
 *      the events time-stamp sum otherwise (in case  policy is average).
 *    - Updates:
 *      *  The number of fragments in the event.
 *      *  The number of bytes in the event in info block.
 *      *  The Event size in the event itself.
 *      *  Copies the ring item into the event.
 *      *  Updates the insertion pointer.
 */
void
CEventAccumulator::appendFragment(EVB::pFlatFragment pFrag)
{
    // Be defensive:
    
    if (!m_pCurrentEvent) {
        throw std::logic_error("Append fragment called with no current event!!");
    }
    // Timestamp stufff:
    
    uint64_t fragts = pFrag->s_header.s_timestamp;
    if (m_tsPolicy == last) {
        m_pCurrentEvent->s_eventHeader.s_bodyHeader.s_timestamp = fragts;
            
    } else {
        m_pCurrentEvent->s_eventInfo.s_TimestampTotal += fragts;
    }
    // Book-Keeping prior to copy:
    
    uint32_t fragSize = pFrag->s_header.s_size + sizeof(EVB::FragmentHeader);
    m_pCurrentEvent->s_eventInfo.s_nFragments++;
    m_pCurrentEvent->s_eventInfo.s_nBytes += fragSize;
    m_pCurrentEvent->s_eventHeader.s_itemHeader.s_size += fragSize;

    uint32_t* pSize = static_cast<uint32_t*>(m_pCurrentEvent->s_pBodyStart);
    *pSize += fragSize;
    
    // Copy the data in and update the insertion pointer and bytes in buffer:
    
    uint8_t* pInsert = static_cast<uint8_t*>(m_pCurrentEvent->s_pInsertionPoint);
    memcpy(pInsert, pFrag, fragSize);
    m_pCurrentEvent->s_pInsertionPoint = pInsert + fragSize;
    m_nBytesInBuffer += fragSize;
    
    if (m_pCurrentEvent->s_eventInfo.s_nFragments >= m_nMaxFrags)
        finishEvent();
}
/**
 * reserveSize
 *    Given newly created m_pCurrentEvent, reserves and initializes
 *    space for the evnet size.
 */
void
CEventAccumulator::reserveSize()
{
    // Be defensive:
    
    if (!m_pCurrentEvent) {
        throw std::logic_error("reserveSize called with no current event!!");
    }
    
    
    if (
        (m_pCurrentEvent->s_eventInfo.s_nBytes > 0) ||
        (m_pCurrentEvent->s_eventInfo.s_nFragments > 0)
    ) {
        throw std::logic_error("reserveSize called with a non-empty current event!!");
    }
    
    if (freeSpace() < sizeof(uint32_t)) flushEvents();
    
    uint32_t* p = static_cast<uint32_t*>(m_pCurrentEvent->s_pInsertionPoint);
    *p++ = sizeof(uint32_t);            // Size is self inclusive!.
    
    // Update all the book keeping stuff:
    
    m_pCurrentEvent->s_pInsertionPoint = p; 
    m_pCurrentEvent->s_eventInfo.s_nBytes          += sizeof(uint32_t);
    m_pCurrentEvent->s_eventHeader.s_itemHeader.s_size += sizeof(uint32_t);
    m_nBytesInBuffer                  += sizeof(uint32_t);
}
