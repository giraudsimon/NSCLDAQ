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

/** @file:  RingChunk.h
 *  @brief: Get chunksof contiguous ring items.
 */
#ifndef RINGCHUNK_H
#define RINGCHUNK_H
#include <stddef.h>
#include <stdint.h>

/**
 *  This is the data structure that defines a chunk
 */
typedef struct _RingItemHeader RingItemHeader, *pRingItemHeader;

typedef struct _Chunk {
    void*    s_pStart;   //< Pointer to start of chunk.
    size_t   s_nBytes;   //< Number of bytes of contiguous ring items
    unsigned s_nBegins;  //< Number of begin runs seen.
    unsigned s_nEnds;    //< Number of end items in chunk.
} Chunk, *pChunk;


// Forward definitions:

class CRingBuffer;

/**
 * @class RingChunk
 *    This class is used by the event logger to support direct, copy-free
 *    writes of large chunks of data from its ringbuffer data source.
 *    The idea is that if we wait long enough (but not too long),
 *    the ring buffer will be populated with a usually contiguous
 *    chunk of data that has many ring items and what we want to do
 *    most of the time, is just point a write right at that
 *    chunk.
 *       This allows us to do, most of the time, copy free writes
 *    of large buffers of data directly from the ring buffer data source
 *    to the output file.  Our religion of amortization  tells us this is
 *    will give us the highest rates of data to disk in the case where we
 *    have high rates of data in to the ring buffer.   If we dont' have
 *    high data rates into the ring buffer, this is all moot and we really
 *    don't care if this isn't the best way to get data to disk for those
 *    rates.
 *
 */
class CRingChunk
{
private:
    CRingBuffer* m_pRing;             //< The ring buffer we get data from.
    bool         m_fChangeRunOk;      //< True if --combine-runs is set.
    uint32_t     m_nRunNumber;        //< Current run number.
    int          m_nFd;               //< Current file descriptor.
public:
    CRingChunk(CRingBuffer* pBuffer, bool combine=false);
    
    // For book-keeping we need to be able to set these.
    
    void setRunNumber(uint32_t newRun);
    void setFd(int newFd);
    
    // What used to be in eventlogMain
    
    void getChunk(Chunk& nextChunk);
    bool nextItemWraps();
    void closeEventSegment();
    void waitForData(size_t nBytes);
    void* getBody(void* pItem);
private:

    bool badBegin(void* h);
};


#endif