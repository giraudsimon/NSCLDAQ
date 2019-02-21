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

/** @file:  ZeroCopyHit.cpp
 *  @brief: Implement the ZeroCopyHit class seee header comments.
 *  
 */
#include "ZeroCopyHit.h"
#include "ReferenceCountedBuffer.h"
#include "BufferArena.h"

namespace DDASReadout {
   
/**
 * constructor
 *   Stores stuff away and increments the refrence count on the underlying buffer:
 *
 *   @param nWords   - Number of uint32_t's in the hit.
 *   @param pHitData - Pointer to the hit data.
 *   @param pBuffer  - Underlying reference counted buffer.
 *   @param pArena   - Buffer arena to which the  buffer is released when no
 *                     no  longer referenced.
 */
ZeroCopyHit::ZeroCopyHit(
    size_t nWords, void* pHitData, ReferenceCountedBuffer* pBuffer,
    BufferArena* pArena    
) :
    RawChannel(nWords, pHitData), m_pBuffer(pBuffer), m_pArena(pArena)
{
    reference();                         // Count a reference on the buffers.
}
/**
 * destructor
 *    Dereference.
 */
ZeroCopyHit::~ZeroCopyHit()
{
    dereference();
}

/**
 * copy construction
 *   - Copy in the new information.
 *   - reference the new hit.
 */
ZeroCopyHit::ZeroCopyHit(const ZeroCopyHit& rhs) :
    RawChannel(rhs), m_pBuffer(rhs.m_pBuffer), m_pArena(rhs.m_pArena)
{
    reference();
    
}
/**
 * assignment
 *    Dereference,
 *    Copy in
 *    Reference.
 */
ZeroCopyHit&
ZeroCopyHit::operator=(const ZeroCopyHit& rhs)
{
    if (this != &rhs) {
        dereference();
        RawChannel::operator=(rhs);
        m_pBuffer = rhs.m_pBuffer;
        m_pArena  = rhs.m_pArena;
        reference();
    }
    return *this;
}

/**
 * reference
 *    Add a reference to the underlying buffer.
 */
void
ZeroCopyHit::reference()
{
    m_pBuffer->reference();
}
/**
 * dereference
 *     Release our reference to m_pBuffer and return it to its arena
 *     if we were the last reference.
 */
void
ZeroCopyHit::dereference()
{
    m_pBuffer->dereference();
    if(!m_pBuffer->isReferenced()) {
        m_pArena->free(m_pBuffer);
    }
}

}                                              // namespace