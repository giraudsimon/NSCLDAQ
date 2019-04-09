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

/** @file:  DDASSorter.h
 *  @brief: Define the class that does all the sorting.
 */
#ifndef DDASSORTER_H
#define DDASSORTER_H

#include <CRingBufferChunkAccess.h>
#include <deque>

class CRingBuffer;
class HitManager;

namespace DDASReadout {
class BufferArena;
class ReferenceCountedBuffer;
class ZeroCopyHit;
}


typedef struct _RingItemHeader *pRingItemHeader;
/**
 * @class DDASSorter
 *    This class manages data flow.   What we do:
 *    -  Take ring items:
 *    -  Non event ring items are just passsed on through.
 *    -  Event items are parsed for hits which are added to the hit manager.
 *    -  If hits are available from the hit manager they are passsed as
 *       output ring items.
 *    -  When the end of run item is seen, the hit manager is flushed prior
 *       to  sending the end run item to the output file.
 */
class DDASSorter
{
private:
    CRingBuffer&  m_source;
    CRingBuffer&  m_sink;
    HitManager*  m_pHits;
    DDASReadout::BufferArena*  m_pArena;
    std::deque<DDASReadout::ZeroCopyHit*>   m_hits;
    uint32_t     m_sid;
    
public:
    DDASSorter(CRingBuffer& source, CRingBuffer& sink);
    ~DDASSorter();
    
    void operator()();
    
private:
    void processChunk(CRingBufferChunkAccess::Chunk& chunk);
    void outputRingItem(pRingItemHeader pItem);
    void processHits(pRingItemHeader    pItem);
    void flushHitManager();
    DDASReadout::ZeroCopyHit* allocateHit();
    void freeHit(DDASReadout::ZeroCopyHit* pHit);
    void outputHit(DDASReadout::ZeroCopyHit* pHit);
};


#endif