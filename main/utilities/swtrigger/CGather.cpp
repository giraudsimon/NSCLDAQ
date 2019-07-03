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

/** @file:  CGather.cpp
 *  @brief: Implement CGather class.
 */

#include "CGather.h"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <new>


/**
 * contructor
 *    Construct an empty gather buffer.
 */
CGather::CGather() : m_pData(nullptr),
    m_nBufferSize(0), m_nUsedBytes(0)
{
    
}
/**
 * constructor
 *    Construct and gather in one operation.
 *
 * @param parts - array of parts.
 * @param nParts - number of parts.
 */
CGather::CGather(const iovec* parts, int nParts) : CGather()
{
    gather(parts, nParts);
}

/**
 * destructor
 *    Release any data associated with the gather object.
 */
CGather::~CGather()
{
    free(m_pData);
}
/**
 *  gather
 *     Gather new data into the buffer.
 *
 *     @param parts -descriptors for each part of the data.
 *     @param nParts - Number of parts to gather.
 */
void
CGather::gather(const iovec* parts, int nParts)
{
    size_t nBytes = gatherSize(parts, nParts);
    if (m_nBufferSize < nBytes) {
        m_pData = realloc(m_pData, nBytes);
        if (!m_pData) throw std::bad_alloc();
        m_nBufferSize = nBytes;
    }
    
    m_nUsedBytes = nBytes;
    uint8_t* pCursor = static_cast<uint8_t*>(m_pData);
    for (int i = 0; i < nParts; i++) {
        pCursor = gatherItem(parts[i], pCursor);
    }
    
    
}
/**
 * operator void*
 *    Convert the object to a void* pointer to its data buffer.
 */
CGather::operator void*()
{
    return m_pData;
}
/**
 * size
 *    Return the number of bytes in the gather buffer.
 */
size_t
CGather::size() const
{
    return m_nUsedBytes;
}

///////////////////////////////////////////////////////////////////////////////
// Private utilities

/**
 * gatherSize
 *   See how many bytes are needed to hold a gather
 *
 *   @praam parts - array of part descriptors.
 *   @param nParts - number of parts.
 *   @return - total number of bytes in the part.
 */
size_t
CGather::gatherSize(const iovec* parts, int nParts) const
{
    size_t result(0);
    for (int i=0; i < nParts; i++) {
        result += parts[i].iov_len;
    }
    return result;
}
/**
 * gatherItem
 *   Copy an item into the buffer.
 *
 *   @param part - references the part to copy.
 *   @param cursor - where to copy it.
 *   @return uint8_t* - pointer to the next free byte of the buffer.
 */
uint8_t*
CGather::gatherItem(const iovec& part, uint8_t* cursor)
{
    memcpy(cursor, part.iov_base, part.iov_len);
    return cursor + part.iov_len;
}