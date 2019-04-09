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

/** @file:  DDASSorter.cpp
 *  @brief: Implement the time sorting code.
 */

#include "DDASSorter.h"
#include "HitManager.h"
#include "BufferArena.h"
#include "ReferenceCountedBuffer.h"
#include "RawChannel.h"
#include "ZeroCopyHit.h"

#include <CRingBuffer.h>
#include <CRingBufferChunkAccess.h>
#include <CPhysicsEventItem.h>
#include <DataFormat.h>
#include <string.h>



/**
 * constructor:
 *    @param source - ringbuffer from which data comes.
 *    @param sink   - ringbuffer to which data goes.
 */
DDASSorter::DDASSorter(CRingBuffer& source, CRingBuffer& sink) :
    m_source(source), m_sink(sink), m_sid(0)
{
    m_pHits = new HitManager(10*((uint64_t)(1000000000)));   // 10 second build window.
    m_pArena = new DDASReadout::BufferArena;
}
/**
 * destructor
 */
DDASSorter::~DDASSorter()
{
    delete m_pHits;
    delete m_pArena;
}

/**
 * operator()
 *    Data flow of the sortrer.
 */
void
DDASSorter::operator()()
{
    CRingBufferChunkAccess chunkGetter(&m_source);
    CRingBuffer::Usage ringStats = m_source.getUsage();
    size_t maxChunk = ringStats.s_bufferSpace/4;
    
    while(1) {
        size_t size = chunkGetter.waitChunk(maxChunk, 10000, 100);
        if(size) {
            CRingBufferChunkAccess::Chunk c = chunkGetter.nextChunk();
            processChunk(c);
        }
    }
    
}
////////////////////////////////////////////////////////////////////////////////
//  Private methods.
//

/**
 * processChunk
 *    Processes a chunk of ring items from the ring buffer.
 *    - With the exception of END_RUN items, non PHYSICS_EVENT items just
 *      go to the sink.
 *    - PHYSICS_EVENT items get put in a buffer arena and, eventually
 *      handed to processHits for parsing, and hit management.
 *    - END_RUN items - cause any PHYSICS_EVENT items to be added to the hits
 *      the hit manager flushed and the end run item pushed out.
 *
 * @param chunk - references a chunk of ring items that has been gotten
 *                from the ring buffer.
 */
void
DDASSorter::processChunk(CRingBufferChunkAccess::Chunk& chunk)
{
    for (auto p = chunk.begin(); !(p == chunk.end()); p++) {
        RingItemHeader& item(*p);
        RingItem& fullItem(reinterpret_cast<RingItem&>(item));
        
        // If there's a source id, pull it out and save it in m_sid.
        
        if (fullItem.s_body.u_noBodyHeader.s_mbz) {
            m_sid = fullItem.s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId;
        }
        
        switch (item.s_type) {
        case PHYSICS_EVENT:
            processHits(&item);
            break;
        case END_RUN:
            flushHitManager();           // Flush any hits in the hit manager.
            outputRingItem(&item);       // then output the ring item.
            break;
        default:
            outputRingItem(&item);      // all other ring items pass through.
        }
    }
}
/**
 * outputRingItem
 *    Outputs a ring item to the sink.
 *
 *  @param pItem - pointer to a raw ring item.
 */
void
DDASSorter::outputRingItem(pRingItemHeader pItem)
{
    m_sink.put(pItem, pItem->s_size);
}
/**
 * processHits
 *    Given a pointer to a ring item that contains hits,
 *    -  Puts the ring item body into a reference counted buffer.
 *    -  Parses the reference counted buffer into a deque of
 *       zero copy hits.
 *    -  Adds those hits to the hit manager.
 *    -  Outputs any hits the hit manager says can be output.
 */
void
DDASSorter::processHits(pRingItemHeader pItem)
{
    auto pBuffer = m_pArena->allocate(pItem->s_size);
    uint32_t* pBodySize = reinterpret_cast<uint32_t*>(pItem + 1);
    uint32_t bodySize   = *pBodySize++;
    uint32_t moduleType = *pBodySize++;
    bodySize           -= 2*sizeof(uint32_t);
    
    memcpy(pBuffer->s_pData, pBodySize, bodySize);   //Copy the raw data.
    uint8_t* p(*pBuffer);
    std::deque<DDASReadout::ZeroCopyHit*> hitList;
    while(bodySize) {
        uint32_t hitSize = DDASReadout::RawChannel::channelLength(p);
        DDASReadout::ZeroCopyHit* pHit= allocateHit();
        pHit->setHit(hitSize, p, pBuffer, m_pArena);
        pHit->s_moduleType = moduleType;
        pHit->SetTime();
        pHit->SetLength();
        pHit->SetTime(DDASReadout::RawChannel::moduleCalibration(moduleType));
        pHit->SetChannel();
        pHit->Validate(hitSize);
        
        hitList.push_back(pHit);
        
        p += hitSize;
        bodySize -= hitSize;
    }
    m_pHits->addHits(hitList);
    // Now see if there are any hits we can output:
    
    while(m_pHits->haveHit()) {
        DDASReadout::ZeroCopyHit* pHit = m_pHits->nextHit();
        outputHit(pHit);
        freeHit(pHit);
    }
}
/**
 * flushHitManager
 *    Called when an end run is encountered.  All hits left in the hit manager
 *    are output regardless of the build window.
 *
 */
void
DDASSorter::flushHitManager()
{
    DDASReadout::ZeroCopyHit* pHit;
    while (pHit = m_pHits->nextHit()) {
        outputHit(pHit);
        freeHit(pHit);
    }
}
/**
 * allocateHit
 *    Attempts to allocate a hit from the pool of hits in m_hits.
 *    IF that pool is exhausted a new one is created.
 * @return DDASReadout::ZeroCopyHit* - pointer to the allocated hit.
 */
DDASReadout::ZeroCopyHit*
DDASSorter::allocateHit()
{
    if(m_hits.empty()) {
        return new DDASReadout::ZeroCopyHit;
    } else {
        auto result = m_hits.front();
        m_hits.pop_front();
        return result;
    }
}
/**
 * freeHit
 *    Returns a hit to the free pool where it can be allocated again without
 *    dynamic memory management.
 *
 *  @param pHit - the hit to return.
 */
void
DDASSorter::freeHit(DDASReadout::ZeroCopyHit* pHit)
{
    m_hits.push_back(pHit);
}
/**
 * outputHit
 *    Given a pointer to a zero copy hit, creates a ring item for that hit
 *    and outputs it. Note that the zero copy construction ofthe
 *    output ring item is used so that there's only  a single copy -
 *    from data in the zero copy hit to the ring buffer.
 *    Note as well that it's up to the caller to decide when the zero copy hit
 *    can be released.
 */
void
DDASSorter::outputHit(DDASReadout::ZeroCopyHit* pHit)
{
    uint64_t ts = pHit->s_time;
    uint32_t bodySize  = pHit->s_channelLength + 2*sizeof(uint32_t)
                       + sizeof(BodyHeader) + sizeof(RingItemHeader) + 100;
    CPhysicsEventItem item(ts, m_sid, 0, bodySize, &m_sink);
    
    // Make this look like an old DDASReadout hit body:
    
    uint32_t* pBody = static_cast<uint32_t*>(item.getBodyCursor());
    *pBody++  = pHit->s_channelLength + 2*sizeof(uint32_t);
    *pBody++  = pHit->s_moduleType;
    memcpy(pBody, pHit->s_data, pHit->s_channelLength*sizeof(uint32_t));
    pBody    += pHit->s_channelLength;
    item.setBodyCursor(pBody);
    item.updateSize();
    item.commitToRing(m_sink);
    
}
