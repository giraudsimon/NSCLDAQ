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

/** @file:  CRingBufferChunkAccess.cpp
 *  @brief: Implementation of the classes in CRing BufferChunkAccess.h
 *  @note   See the header for more information about these classes and their
 *          purpose.
 */
#include "CRingBufferChunkAccess.h"
#include <CRingBuffer.h>
#include <DataFormat.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <new>


// These internal typedefs are for syntactical brevity in the implementations.

typedef CRingBufferChunkAccess::Chunk ChunkClass;
typedef CRingBufferChunkAccess::Chunk::iterator ChunkIterator;

/*===========================================================================
 *  CRingBufferChunkAccess implementation.
 */

/**
 * constructor
 *    @param pRingBuffer - A ring buffer objects pointer.  The ring buffer
 *                         must be attached as a consumer or all holy hell will
 *                         break loose.
 *    @throws std::logic_error if we're not amonst the consumers.
 *    @todo - the check for not being a consumer is not 100% reliable;
 *            if a producer, that is also a consumer passes its producer
 *            ringbuffer the check won't catch that.  Fortunately, the most
 *            common use case for being a producer and consumer of the same
 *            ring is unit testing.  It would really be best for CRingBuffer
 *            to expose a  getter to its m_mode member.
 */
CRingBufferChunkAccess::CRingBufferChunkAccess(CRingBuffer* pRingBuffer) :
    m_pRingBuffer(pRingBuffer), m_pWrappedItem(nullptr), m_nWrapSize(0)
{
    CRingBuffer::Usage u = pRingBuffer->getUsage();
    m_nRingBufferBytes   = u.s_bufferSpace;
    
    pid_t mypid = getpid();
    for (int i =0; i < u.s_consumers.size(); i++) {
        if (mypid == u.s_consumers[i].first) return;
    }
    // Nope not a consumer.
    
    throw std::logic_error("Constructed CRingBufferChunkAccess but not a consumer");
}
/**
 * destructor
 *    free the wrapped chunk storage:
 *    @note - the caller should soon after destroy the ring buffer.
 */
CRingBufferChunkAccess::~CRingBufferChunkAccess()
{
    // If there's a chunk with sizes we skip it's data on the assumption
    // that we're really done with this
    
    size_t s = m_chunk.size();
    if (s > 0) {
        m_pRingBuffer->skip(s);
        m_chunk.setChunk(0, nullptr);  // Zero the chunk
    }
    free(m_pWrappedItem);
}

/**
 * waitChunk
 *    Wait for at least the specified amount of data to be available for
 *    consumption.
 *
 *  @param maxChunk - wait finishes when at least this number of bytes is in the
 *                    ring buffer.
 *  @param polls    - Maximum number of polls to wait (0, the default is no limit).
 *  @param usecPoll - Sleep time between polls, 0 - none.
 *  @return size_t  - Actual number of bytes in the pool.  Note that if there's not
 *                    at least a full ring item, this will return 0.
 *  @throws std::invalid_argument if maxChunk is bigger than the ring item
 *                    storage.
 *  @note this invalidates any existing chunk.
 *  
 */
size_t
CRingBufferChunkAccess::waitChunk(size_t maxChunk, int polls, int usecPoll)
{
    if (maxChunk > m_nRingBufferBytes) {
     throw std::invalid_argument(
         "CRingBufferChunkAccess::waitChunk - maxChunk is bigger than ring buffer"
     );
    }
    
    // If there's a current chunk we need to skip it's data to get an accurate
    // read on the remaining data:
    
     if (m_chunk.size() > 0) {
        m_pRingBuffer->skip(m_chunk.size());
        m_chunk.setChunk(0, m_chunk.getStorage());  // Zero the chunk.
    }   
    
    int pollCount = 0;
    while (m_pRingBuffer->availableData() < maxChunk) {
     pollCount++;
     if ((polls != 0) && (pollCount > polls)) break;
     if (usecPoll > 0)
     usleep(usecPoll);
    }
    
    return haveFullRingItem();
} 
/**
 * getChunk
 *    Construct a chunk from the data in the ring.
 *    There are two cases:
 *    - There are complete ring items before the buffer wrap:
 *      In that case all complete ring items available are put into the
 *      chunk.
 *    - The first ring item wraps. In that case, m_pWrappedItem is sized
 *      to hold the wrapped item and the chunk consists only of the
 *      wrapped item.
 *
 * @note   If there are not sufficient bytes in the ring buffer to hold a ring
 *         item, this throws std::logic_error.
 * @return Chunk
 */
ChunkClass
CRingBufferChunkAccess::nextChunk()
{
    // If the current chunk has a size we need to skip that number of bytes.
    
    if (m_chunk.size() > 0) {
        m_pRingBuffer->skip(m_chunk.size());
        m_chunk.setChunk(0, m_chunk.getStorage());  // Zero the chunk.
    }

    // do we have at least one full ring item?  If not return a null chunk.

    size_t availData =   haveFullRingItem();
    if (!availData) {                             // nope return null chunk.
      m_chunk.setChunk(0, nullptr);
      return m_chunk;                  
    }
    // If there's not at least one full ring item then make and return a null chunk
    
    
    
    // Distinguish between the cases described in the comment header:
    
    if (firstItemWraps())  {
        makeWrappedItemChunk();
        
    } else {
        // The max data we want to give the chunk is the min of
        // the distance to wrap and the available data:
        
        size_t distToWrap= m_pRingBuffer->bytesToTop();
        size_t chunkMax  = availData < distToWrap ? availData : distToWrap;
        
        void *pItems = m_pRingBuffer->getPointer();
        size_t fullItemSize = sizeChunk(pItems, chunkMax);
        makeInPlaceChunk(pItems, fullItemSize);
        
    }
    return m_chunk;
}

/////////////////////////////////////////////////////////////////////////////
// Private methods:

/**
 * sizeWrapBuffer
 *    Ensures the wrap buffer is big enough to hold the requested storage.
 * @param nRequired - number of required bytes.
 */
void
CRingBufferChunkAccess::sizeWrapBuffer(size_t nRequired)
{
    if (nRequired > m_nWrapSize)  {
        free(m_pWrappedItem);
        m_pWrappedItem = malloc(nRequired);
        if (!m_pWrappedItem) {
            throw std::bad_alloc();
        }
        m_nWrapSize = nRequired;
    }
}
/**
 * Determines if there's sufficient data to hold a ring item.
 * @return  - available data in the ring buffer.
 * @retval   0 - there's not a complete ring item.
 */
size_t
CRingBufferChunkAccess::haveFullRingItem()
{
    size_t availData =  m_pRingBuffer->availableData();
    if (availData < sizeof(uint32_t)) return 0;
    uint32_t size;
    m_pRingBuffer->peek(&size, sizeof(uint32_t));  // peek takes care of wrapping.
    
    return (availData >= size) ? availData : 0;
}
/**
 * sizeChunk
 *    Given a pointer to a chunk of data in the ring buffer and
 *    the number of contiguous bytes available,
 *    returns a new size that only encompasses the complete ring items.
 *
 *  @param pChunk   - pointer to the chunk
 *  @param nBytes   - Amount of contiguous data.
 *  @return size_t - number of bytes that make up complete ring items.
 */
size_t
CRingBufferChunkAccess::sizeChunk(void* pChunk, size_t nBytes)
{
    uint8_t *p = static_cast<uint8_t*>(pChunk);
    size_t result(0);
    while(nBytes >= sizeof(uint32_t)) {    // Finish when we don't have a full size.
        uint32_t* pSize = reinterpret_cast<uint32_t*>(p);
        if (*pSize <= nBytes) {      // Item fits.
            result += *pSize;        // Count the ring item size.
            nBytes -= *pSize;        // Count down the remaining bytes.
            p      += *pSize;        // Point to the next ring item.
        } else {
            break;                   // Item doesn't fit.
        }
    }
    
    return result;
}
/**
 * firstItemWraps
 *    @return bool - true if the first available item in the ring buffer
 *                   wraps.  The assumption is that there's at least
 *                   one ring item.
 */
bool
CRingBufferChunkAccess::firstItemWraps()
{
    uint32_t size;
    m_pRingBuffer->peek(&size, sizeof(uint32_t));   //  Handles wrapping.
    return m_pRingBuffer->wouldWrap(size);
}
/**
 * makeWrappedItemChunk.
 *    Makes the chunk when the next ring item in the buffer
 *    wraps.
 *    - The m_pWrapedItem buffer is sized to hold the item.
 *    - The item is peeked into the wrap buffer and
 *    - Chunk is set up with the wrap buffer as storage:
 *   @note caller must have ensured there's at least one full ring item
 *         available to us.
 */
void
CRingBufferChunkAccess::makeWrappedItemChunk()
{
    uint32_t itemSize;
    m_pRingBuffer->peek(&itemSize, sizeof(uint32_t));
    sizeWrapBuffer(itemSize);
    
    m_pRingBuffer->peek(m_pWrappedItem, itemSize);
    m_chunk.setChunk(itemSize, m_pWrappedItem);
    
}
/**
 * makeInPlaceChunk
 *    Makes the chunk when there are ring items prior to the wrap:
 *
 *  @param pData - pointer to the ring item get areay.
 *  @param nBytes - number of bytes worth of complete ring items.
 */
void
CRingBufferChunkAccess::makeInPlaceChunk(void* pData, size_t nBytes)
{
    m_chunk.setChunk(nBytes, pData);
}
/////////////////////////////////////////////////////////////////////////
// Implements CRingBufferChunkAccess::Chunk

/**
 * constructor
 *    Constrcutor (default).
 *       Initialize an unset chunk.  Attempts to use this will segfault.
 *       @note the typedef ChunkClass for CRingBufferChunkAccess:Chunk.
 */
ChunkClass::Chunk() :
    m_nBytesInChunk(0), m_pStorage(nullptr)
{    
}
/**
 * setChunk
 *    Give the chunk access t osome data:
 *  @param bytesInChunk - number of bytes in the chunk.
 *  @param pStorage     - Pointer to the storage.
 */
void
ChunkClass::setChunk(size_t bytesInChunk, void* pStorage)
{
    m_nBytesInChunk = bytesInChunk;
    m_pStorage      = pStorage;
}
/**
 * getStorage
 *    Return a pointer to the chunk storage.
 * @return void*
 */
void*
ChunkClass::getStorage()
{
    return m_pStorage;
}
/**
 *  size
 *    @return size_t - number of bytes in the current chunk.
 *    @retval 0 - the chunk has not yet been set.
 */
size_t
ChunkClass::size() const
{
    return m_nBytesInChunk;
}
/**
 * begin
 *    Returns an iterator into the chunk that 'points' to the beginning of the
 *    chunk.  The iterator assumes that the chunk contains items that lead off with
 *    a uint32_t size in bytes (inclusive).
 *
 * @return CRingBufferChunkAccess::Chunk::iterator (note ChunkIterator typedef is used)
 */
ChunkIterator
ChunkClass::begin()
{
    return ChunkIterator(m_pStorage, m_nBytesInChunk);
}
/**
 * end
 *   Returns an iterator into the chunk that 'points' past the end of the chunk.
 *   This iterator is designated by a null storage pointer and chunksize of zero:
 *
 *  @return CRingBufferChunkAccess::Chunk::iterator (ChunkIterator).
 */
ChunkIterator
ChunkClass::end()
{
    return ChunkIterator(nullptr, 0);
}
////////////////////////////////////////////////////////////////////////////////////////
// implementation of CRingBufferChunkAccess::Chunk::iterator (ChunkIterator typedef).

/*
 * Constructor
 *    Constructs an iterator from chunk storage.
 *
 *   @param pStorage - pointer to the iterated storage.
 *   @param bytes    - Numberof bytes in the storage.
 */
ChunkIterator::iterator(void* pStorage, size_t bytes) :
    m_pData(pStorage), m_nOffset(0), m_nTotalBytes(bytes)
{}
/**
 * Copy constructor
 *
 * @param rhs - the object we're constructing from
 */
ChunkIterator::iterator(const ChunkIterator& rhs) :
    m_pData(rhs.m_pData),
    m_nOffset(rhs.m_nOffset),
    m_nTotalBytes(rhs.m_nTotalBytes)
{
    
}
/**
 * assignment
 *   @param rhs - the iterator we're being assigned from.
 *   @return *this.
 */
ChunkIterator&
ChunkIterator::operator=(const ChunkIterator& rhs)
{
    m_pData       = (rhs.m_pData),
    m_nOffset     = (rhs.m_nOffset),
    m_nTotalBytes = (rhs.m_nTotalBytes);
    
    return *this;
}
/**
 *  Equality comparison.
 *    @param rhs - the iterator we're comparing with.
 *    @return bool.  True if the two are equal.
 */
bool
ChunkIterator::operator==(const ChunkIterator& rhs) const
{
    return (m_pData == rhs.m_pData)                &&
           (m_nOffset == rhs.m_nOffset)            &&
           (m_nTotalBytes == rhs.m_nTotalBytes);
}

/**
 * operator->
 *   Base of ring item header struct reference operator.
*  @return pRingItemHeader
*  @note m_pData moves with ++, so this is just a cast of that:
*/
pRingItemHeader
ChunkIterator::operator->()
{
    return static_cast<pRingItemHeader>(m_pData);
}
/**
 * operator*
 *   Dereferences the iterator as a ring item header.
 *
 * @return RingItemHeader&
 */
RingItemHeader&
ChunkIterator::operator*()
{
    return *(operator->());
}
/**
 * operator++ (prefix)
 * @return - the iterator after incremented.
 */
ChunkIterator&
ChunkIterator::operator++()
{
    if(m_pData) {                         // Don't increment end.
        pRingItemHeader pH = (operator->());
        m_nOffset += pH->s_size;
        if(m_nOffset < m_nTotalBytes) {    
        
            uint8_t*        p  = static_cast<uint8_t*>(m_pData);
            p                 += pH->s_size;
            m_pData = p;
        } else {                      // End.
            m_pData = nullptr;
            m_nOffset = 0;
            m_nTotalBytes = 0;
        }
    }    
    return *this;
}
/**
 * operator++ (postfix)
 *   @return the iterator prior to the increment.
 */
ChunkIterator
ChunkIterator::operator++(int ignored)
{
    ChunkIterator result(*this);
    ++(*this);
    return result;
    
}
