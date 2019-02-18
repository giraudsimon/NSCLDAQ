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

/** @file:  CZCopyRingBuffer.h
 *  @brief: Class to provide nearly zero copy access to ring buffer consumers.
 */
#ifndef CZCOPYRINGBUFFER_H
#define CZCOPYRINGBUFFER_H
#include <stddef.h>

class CRingBuffer;

/**
 * @class CZCopyRingBuffer
 *    This class provides near zero copy access to ring buffers under the
 *    assumption the client plays by the rules.  The rules are that the
 *    client must eventually tell us to advance the ring buffer after it
 *    is done with an item, and that the user scrupulously does not write
 *    to the ring.
 *
 *    For the most part, we can satisfy requests by providing a pointer right
 *    into the ring buffer to the item.  For cases where that would cause the
 *    ring to wrap, we'll create a local copy of the item and provide that.
 *    The local copy is done with a peek so that the advance semantics are the
 *    same as for when we give a direct copy.
 *
 *    Under the assumption that most items are significantly smaller than the
 *    ring buffer in which they're inserted, access to the ring is
 *    amortized zero copy.
 *
 *    @note that multiple gets without a free will return a pointer to the same
 *          item.  This allows you to do things like get a ring item size and,
 *          once the ring item is fully available get it and then release the
 *          ring item only.
 */
class CZCopyRingBuffer
{
private:
    CRingBuffer* m_pRing;
    void*        m_pLocalData;
    unsigned     m_nLocalDataSize;
    unsigned     m_nLastSize;
public:
    CZCopyRingBuffer(CRingBuffer* pRing);
    virtual ~CZCopyRingBuffer();
    
    void* get(size_t nBytes);          // Note blocks until nBytes are there.
    void  done();
};


#endif