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

/** @file:  CBufferedRingItemConsumer.h
 *  @brief: Defines a high performance ring item consumer from ring buffers.
 */
#ifndef CBUFFEREDRINGITEMCONSUMER_H
#define CBUFFEREDRINGITEMCONSUMER_H
#include <stdint.h>

class CRingBuffer;

/**
 * @class CBufferedRingConsumer
 *
 * This class tries to minimize the number of memcpy calls a consumer
 * needs to get data from a ring.   It also uses/assumes that big memcpys
 * are faster than small ones as they can recruit  the high performance
 * schemes processors have or VM have for doing these copies.
 *
 * Here's the strategy
 *   - At construction time, a buffer as big as the ring buffer is created
 *     and initialized to an empty state.
 *   - Each time a get is done, if necessary, the buffer is filled with as
 *     much data as is available in the ring buffer (minimized to the buffer size).
 *     The get returns a pointer to the next ring item in the ring.
 *   - At the end of the read, partial items are slid down to the start of the
 *     buffer so that we just append the remainder on the end of them.
 */
class CBufferedRingItemConsumer {
private:
    CRingBuffer&    m_Ring;              // Must be a consumer ring.
    void*           m_pBuffer;            // _Big_ buffer.
    uint32_t        m_nBufferSize;
    
    // For getting data:
    
    void*           m_pCursor;            // where the next item comes from.
    uint32_t        m_nBytesLeft;         // Bytes left in the buffer.
public:
    CBufferedRingItemConsumer(CRingBuffer& ring);
    virtual ~CBufferedRingItemConsumer();
    
    void* get();                        // pointer to next ring item.
public:
    bool mustFill();                    // Need to fill to satisfy get?
    void fill();                        // Fill the buffer.
    void* slideRemainder();             // Slide the remaining bytes to the buffer front.
    void  next();                       // Adjust pointers for next ring item.
};
#endif