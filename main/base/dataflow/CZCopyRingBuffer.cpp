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

/** @file:  CZCopyRingBuffer.cpp
 *  @brief: Implementation of the zero copy ring buffer class.
 *  
 */
#include "CZCopyRingBuffer.h"
#include "CRingBuffer.h"

#include <stdlib.h>

/**
 * constructor.
 *   @param pRing -the ring buffer we'll be encapsulating.
 */
CZCopyRingBuffer::CZCopyRingBuffer(CRingBuffer* pRing) :
    m_pRing(pRing), m_pLocalData(nullptr), m_nLocalDataSize(0), m_nLastSize(0)
{}
/**
 * destructor
 *    Get rid of any local data storage we might have had to make:
 */
CZCopyRingBuffer::~CZCopyRingBuffer()
{
    free(m_pLocalData);
}

/**
 * get
 *    - Block until that much data is available in the ring buffer.
 *    - If the data doesn't wrap  just return a pointer into the ring.
 *      Otherwise, copy the data locally and return a pointer to it.
 *
 * @param size - Number of bytes desired.
 */
void*
CZCopyRingBuffer::get(size_t nBytes)
{
    void* pResult;
    while (m_pRing->availableData() < nBytes) usleep(100); // Make this settable?
    
    if (m_pRing->wouldWrap(nBytes)) {
        if (m_nLocalDataSize < nBytes) {
            free(m_pLocalData);                // cheaper than realloc
            m_pLocalData = malloc(nBytes);     // since we don't need to copy.
        }
        m_pRing->get(m_pLocalData, nBytes, nBytes);
        pResult = m_pLocalData;
    } else {
        pResult = m_pRing->getPointer();
    }
    m_nLastSize = nBytes;
    return pResult;
}
/**
 * done
 *     Release the most recently gotten pointer by advancing the
 *     ring buffer get pointer by m_nLastSize bytes.
 */
void
CZCopyRingBuffer::done()
{
    m_pRing->skip(m_nLastSize);
    m_nLastSize = 0;
}