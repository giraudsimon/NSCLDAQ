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

/** @file:  CRingBufferChunkAccess.h
 *  @brief: Provide zero copy access to chunks of ring items.
 */
#ifndef CRINGBUFFERCHUNKACCESS_H
#define CRINGBUFFERCHUNKACCESS_H

#include <DataFormat.h>

class CRingBuffer;

#include <stddef.h>                // Size_t I think.
/**
 * @class CRingBufferChunkAccess
 *    High performance NSCLDAQ application need faster access to ring buffres
 *    than the CRingItem class hierarcy, with its multi-copies etc. Can provide.
 *    CRingBufferChunkAccess provides access to entities called "chunks" that
 *    represent several contiguous memory items that are in either directly in
 *    the ring buffer or are in a partial ring item buffer in the case of
 *    ring items that wrap around from the end to the start of the buffer.
 *
 *    Nested classes include:
 *
 *  @class Chunk
 *     This represents one chunk of ring items.  It can provide:
 *     - A pointer to the chunk,
 *     - The size of the chunk,
 *     - iterators over the ring items in the chunk.
 *
 *  Which brings us to:
 *
 *  @class iterator
 *    A pointer like object that is an iterator into the individual
 *    ring items in a chunk.
 *    - Dereferencing with * and -> provides access to the data and to the header.
 *    - ++ advances to the next item (this is a forward only iterator).
 *    - Comparison with Chunk::end() for the chunk the iterator is pointing into
 *      tells you when there are n more items left.
 *     
 */    
class CRingBufferChunkAccess {
public:
    class Chunk {
    private:
        size_t m_nBytesInChunk;
        void* m_pStorage;
    public:
        class iterator {
        private:
            void*  m_pData;
            size_t m_nOffset;
            size_t m_nTotalBytes;
        public:
            iterator(void* pStorage, size_t bytes);
            iterator(const iterator& rhs);
            iterator& operator=(const iterator& rhs);
            bool operator==(const iterator& rhs) const;
            
            pRingItemHeader operator->();
            RingItemHeader& operator*();
            iterator& operator++();         // Prefix incr.
            iterator  operator++(int);      // Postfix incr.
        };
        Chunk();
        void setChunk(size_t bytesInChunk, void* pStorage);
    
        void* getStorage();
        size_t size() const;
        iterator begin();
        iterator end();
    };
public:
    size_t       m_nRingBufferBytes;

private:
    CRingBuffer* m_pRingBuffer;
    Chunk        m_chunk;
    void*        m_pWrappedItem;
    size_t       m_nWrapSize;
public:
    CRingBufferChunkAccess(CRingBuffer* pRingBuffer);
    virtual ~CRingBufferChunkAccess();
    size_t waitChunk(size_t maxChunk, int polls = 0, int usecPoll = 0);
    Chunk nextChunk();
private:
    void sizeWrapBuffer(size_t nRequired);
    size_t haveFullRingItem();
    size_t sizeChunk(void* pChunk, size_t nBytes);
    bool   firstItemWraps();
    void   makeWrappedItemChunk();
    void   makeInPlaceChunk(void* pData, size_t nBytes);
    
};


#endif