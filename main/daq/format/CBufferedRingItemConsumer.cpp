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

/** @file:  CBufferedRingItemConsumer.cpp
 *  @brief: implements a buffered consumer.
 */
#include "CBufferedRingItemConsumer.h"
#include <CRingBuffer.h>

#include <stdexcept>
#include <system_error>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * constructor.
 *  @param ring - reference to the ring buffer we'll use to get data.
 *  @note ring must remain in scope until you've destroyed this object.
 */
CBufferedRingItemConsumer::CBufferedRingItemConsumer(CRingItem& ring) :
    m_Ring(ring), m_pBuffer(nullptr), m_nBufferSize(0), m_pCursor(nullptr),
    m_nBytesLeft(0)
{
    // Figure out the buffer size:
    
    CRingBugffer::Usage u = m_Ring.getUsage();
    m_nBufferSize = u.s_bufferSpace;
    
    m_pBuffer = malloc(m_nBufferSize);
    if (!m_pBuffer) {
        throw std::system_error(errno, std::generic_category, "Buffer allocation failed");)
    }
    
    m_pCursor = m_pBuffer;
    
}
/**
 * destructor - free the buffer.
 */
CBufferedRingItemConsumer::~CBufferedRingItemConsumer()
{
    free(mPpBuffer);
}
/**
 * get
 *   @return void* - pointer to the next available ring item.
 *   @note This pointer should not be considered valid after the next
 *         get operation.
 *   @note if needed, data is gotten from the ring buffer until the request
 *         can be satisfied.
 */
void*
CBufferedRingItemConsumer::get()
{
    // If necessary, fill the buffer with at least one ring item:
    
    if (mustFill()) fill();
    
    // At this point we know there's at least one complete ring item
    // in the buffer and that m_pCursor points to it:
    
    void* result = m_pCursor;
    next();                        // Advance past the item
    
    return result;
}
///////////////////////////////////////////////////////////////////////////////
// Private utility methods.


/**
 * mustFill
 *  @return bool -true if we need to fill our buffer in order to
 *                satisfy a get operation.
 *
 *   This is the case if:
 *   -   m_pCursor is nullptr indicating we've never gotten data from the ring.
 *   -   m_nBytesLeft is 0 indicating we're at the exact end of data.
 *   -   m_nBytesLeft < sizeof(uint32_t) - indicating we don't even have
 *       a ring item count.
 *   -   m_nBytesLeft < *(uint32_t*)(m_pCursor) indicating we don't have a
 *       complete ring item left.
 */
bool
CBufferedRingItemConsumer::mustFill()
{
    if (!m_pCursor) return true;
    if (m_nBytesLeft < sizeof(uint32_t)) return true;
    
    uint32_t* pItemSize = static_cast<uint32_t*>(m_pCursor);
    return (m_nBytesLeft < *pItemSize);
}
/**
 * fill
 *    Fill the data buffer with new data.  If there's already a partial
 *    item in the ring, we use slideRemainder to position it at the front
 *    of the buffer.  We then read:
 *    -  If we have < sizeof(uint32_t) in the buffer we furst do a
 *       read to get uint32_t so that we know how many bytes are needed
 *       to get a full ringitem.
 *    -  If we have a partial item, at least the bytes needed to satisfy
 *       the get
 *    -  minimum of all the bytes in the ring and the remaining  buffer space.
 *
 *  @note the read timeout depends on the readsize; If we're just trying to
 *      complete a ring item the timeout is infinite.  For the big reads our
 *      timeout is 0.
 */
void
CBufferedRingItemConsumer::fill()
{
    void* pGetPointer = m_pBuffer;
    if (m_nBytesLeft > 0) {
        pGetPointer = slideRemainder();        // Partial ring item.
    }
    // If needed get that ring item size:
    
    if (m_nBytesLeft < sizeof(uint32_t)) {
        uint32_t n = size_t(uint32_t) - m_nBytesLeft
        m_Ring.get(pGetPointer, n, n);
        m_nBytesLeft += n;
        uint8_t* p = static_cast<uint8_t*>(pGetPointer);
        p += n;
        pGetPointer = p;
    }
    // Now we have at least a uint32_t pointed to by m_pCursor (which should
    // also be m_pBuffer after slideRemainder was called.
    // Read the rest of the partial ring item waiting forever if need be.
    
    uint32_t* pSize = static_cast<uint32_t>(m_pCursor);
    uint32_t rItemSize = *pSize;
    if (rItemSize < m_nBytesLeft) {    // Should be.
        uint32_t remainderSize = rItemSize - m_nBytesLeft;
        m_ring.get(pGetPointer, remainderSize, remainderSize);
        m_nBytesLeft += remainderSize;
        
        uint8_t* p = static_cast<uint8_t*>(pGetPointer);
        p += remainderSize;
        pGetPointer = p;
    }
    // Anything we can get now is gravy as we can satisfy the get();
    // We'll try to get the available data or the free data,
    // whichever is smaller - but leave open the possibility of getting
    // the whole damned buffer if we can have it _now_
    
    size_t avail = m_ring.availableData();
    if(avail) {
    uint32_t freeData = m_nBufferSize - m_nBufferSize;
    if (freeData < avail) avail = freeData;
    size_t nRead = m_ring.get(pGetPointer, freeData, avail, 0);
    
    m_nBytesLeft += nRead;
}
/**
 * slideRemainder
 *   Any remaining data are slid to the front of the buffer.
 *   m_pCursor -> front of buffer.
 *
 * @return void*  - pointer just past any slid data.
 */
void*
CBufferedRingItemConsumer::slideRemainder()
{
    if (m_nBytesLeft) {
        memmove(m_pBuffer, m_pCursor, m_nBytesLeft);  // could overlap.
    }
    m_pCursor = m_pBuffer;
    uint8_t* pEnd = static_cast<uint8_t*>(m_pBuffer);
    pEnd += m_nBytesLeft;
    return pEnd;
}
/**
 * next
 *   Does the book keeping needed to advance to the next ring item.
 */
void
CBufferedRingItemConsumer::next()
{
    uint32_t* p = static_cast<uint32_t*>(m_pCursor);
    uint32_t itemSize = *p;
    uint8_t* pB = static_cast<uint8_t*>(m_pCursor);
    pB += itemSize;
    m_pCursor = pB;
    m_nBytesLeft -= itemSize;
}