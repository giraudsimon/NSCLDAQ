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

/** @file:  CRingItemFileTransport.cpp
 *  @brief: Implement Reading/writing ring items from file.
 */
#include "CRingItemFileTransport.h"

#include <CBufferedOutput.h>

#include <DataFormat.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <new>
#include <stdexcept>

static const size_t READ_SIZE(32*1024*1024);          // Big reads.

/**
 * constructor (read)
 *    @param reader - the block reader to use to get data from the file.
 */
CRingItemFileTransport::CRingItemFileTransport(CRingFileBlockReader& reader) :
    m_pReader(&reader), m_pWriter(nullptr), m_pDescriptor(nullptr)
{}

/**
 * constructor(write)
 *   @param writer - the block writer to use to put data to a file.
 */
CRingItemFileTransport::CRingItemFileTransport(io::CBufferedOutput& writer) :
    m_pReader(nullptr), m_pWriter(&writer), m_pDescriptor(nullptr)
{}

/**
 * destructor
 *   The assumption is that the readers/writers are dynamically allocated.
 */
CRingItemFileTransport::~CRingItemFileTransport()
{
    delete m_pReader;             // C++ standard makes delete of nullptr
    delete m_pWriter;             // a no-op.
    
    delete m_pDescriptor;
}

/**
 * recv
 *    Get the next ring item from file.
 *
 * @param ppData - pointer to where a malloced pointer to the ring item
 *                 will be put.
 * @param size   - Reference to where the size of the ring item will be put.
 */
void CRingItemFileTransport::recv(void** ppData, size_t& size)
{
    if (!m_pReader) {
        throw std::runtime_error(
            "Attempted read from a write-only ring file transport"
        );
    }
    if (!m_pDescriptor) nextDescriptor();
    
    // If there are no items left we've hit the end of file.
    // Store a nullptr for that.
    
    if (m_nItemsRemaining == 0) {
        *ppData = nullptr;
        size    = 0;
        return;
    }
    // There's an item to return and m_pCursor points to it.
    
    pRingItem pHeader = reinterpret_cast<pRingItem>(m_pCursor);
    size                    = itemSize(pHeader);
    void* pItem             = malloc(size);
    if (!pItem) {
        throw std::bad_alloc();    
    }
    memcpy(pItem, m_pCursor, size);
    *ppData = pItem;
    m_pCursor += size;
    m_nItemsRemaining--;
    
    // If there are no items remaining now, release the data descriptor
    // and ensure we try to get another one next time.
    
    if (m_nItemsRemaining == 0) {
        freeDescriptor();
    }
}
/**
 * end - must be a writer   flush the output buffer.
 */
void CRingItemFileTransport::end()
{
    if (m_pWriter) {
        m_pWriter->flush();
    } else {
        throw std::runtime_error("Attempted end() call on read only transport");
    }
    
}
/**
 * send
 *    Sends data to the writer.
 * @param parts - pointer to the parts of the message (probably each part
 *                is a ring item but that's not required).
 * @param numParts - number of parts present.
 */
void
CRingItemFileTransport::send(iovec* parts, size_t numParts)
{
    if (!m_pWriter) {
        throw std::runtime_error(
            "Attempted write to a read-only ring file transport"
        );
    }
    //  @todo Note, in the future, we may want to, for large numParts
    // flush, get the fd and do a writev to avoid the copies.
    
    for (int i =0; i <numParts; i++) {
        m_pWriter->put(parts[i].iov_base, parts[i].iov_len);
    }
}

////////////////////////////////////////////////////////////////////////
//  Read utilities:

/**
 * nextDescriptor
 *    Get a new descriptor from the block reader and
 *    set up the book keeping:
*/
void
CRingItemFileTransport::nextDescriptor()
{
    auto descr    = m_pReader->read(READ_SIZE);
    m_pDescriptor = new CRingBlockReader::DataDescriptor(descr);
    
    m_pCursor         = static_cast<uint8_t*>(m_pDescriptor->s_pData);
    m_nItemsRemaining = m_pDescriptor->s_nItems;
}
/**
 *  freeDescriptor
 *     Called when we're done with a data descriptor.
 */
void
CRingItemFileTransport::freeDescriptor()
{
    if (m_pDescriptor) {
        free(m_pDescriptor->s_pData);
    }
    delete              m_pDescriptor;
    m_pDescriptor     = nullptr;
    m_pCursor         = nullptr;
    m_nItemsRemaining = 0;
}