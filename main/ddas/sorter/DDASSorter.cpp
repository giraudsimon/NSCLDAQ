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
#include <iostream>
#include <sstream>
#include <stdlib.h>

static const uint32_t EXTCLKBIT(1 << 21);

/**
 * constructor:
 *    @param source - ringbuffer from which data comes.
 *    @param sink   - ringbuffer to which data goes.
 *    @param window = accumulation window
 */
DDASSorter::DDASSorter(CRingBuffer& source, CRingBuffer& sink, float window) :
    m_source(source), m_sink(sink), m_sid(0), m_lastEmittedTimestamp(0)
{
    m_pHits = new HitManager(window*((uint64_t)(1000000000)));   // 10 second build window.
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
            if (c.size() > 0) {
                processChunk(c);
            }
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
  try {
    for (auto p = chunk.begin(); !(p == chunk.end()); p++) {
      RingItemHeader& item(*p);
      RingItem& fullItem(reinterpret_cast<RingItem&>(item));
      
      // If there's a source id, pull it out and save it in m_sid.
      
      if (hasBodyHeader(&fullItem)) {
          m_sid =
            (reinterpret_cast<pBodyHeader>(bodyHeader(&fullItem)))->s_sourceId;
      }
      
      switch (itemType(&fullItem)) {
      case PHYSICS_EVENT:
            processHits(&item);
            break;
      case END_RUN:
            flushHitManager();           // Flush any hits in the hit manager.
            outputRingItem(&item);       // then output the ring item.
            m_lastEmittedTimestamp =0;   // For next run.
            break;
        default:
            outputRingItem(&item);      // all other ring items pass through.
      }
    }
  } catch (std::string msg) {
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
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
 *
 *    The ring item body of a physics event has the following contents:
 *    \verbatim
 *
 *    +------------------------------------------------------+
 *    |   Size of the body in 16 bit words (uint32_t)        |
 *    +------------------------------------------------------+
 *    |  Module id uint32_t (note bit 21 says use ext clock) |
 *    +------------------------------------------------------+
 *    | Clock scale factor (double precision).               |
 *    +------------------------------------------------------+
 *    |    soup of hits as they come from the module         |
 *
 *    \endverbatim
 */
void
DDASSorter::processHits(pRingItemHeader pItem)
{
    auto pBuffer = m_pArena->allocate(pItem->s_size);
    
    pRingItem pFullItem = reinterpret_cast<pRingItem>(pItem);
    
    // This is ok because Readout does not put body header extensions in
    // its events.
    
    uint32_t* pBodySize = static_cast<uint32_t*>(bodyPointer(pFullItem));    
    
    uint32_t bodySize   = *pBodySize++;
    uint32_t moduleType = *pBodySize++;
    double*  pScale     = reinterpret_cast<double*>(pBodySize);
    double   clockScale = *pScale++;
    pBodySize           = reinterpret_cast<uint32_t*>(pScale);
    bodySize           -= (2*sizeof(uint32_t)+sizeof(double))/sizeof(uint16_t);
    bool useExtClock    = (moduleType & EXTCLKBIT) != 0;
    memcpy(pBuffer->s_pData, pBodySize, bodySize*sizeof(uint16_t));   //Copy the raw data.
    uint8_t* p(*pBuffer);
    std::deque<DDASReadout::ZeroCopyHit*> hitList;
    bool warnedLate(false);
    while(bodySize) {
        uint32_t hitSize = DDASReadout::RawChannel::channelLength(p);
        DDASReadout::ZeroCopyHit* pHit= allocateHit();
        pHit->setHit(hitSize, p, pBuffer, m_pArena);
        pHit->s_moduleType = moduleType;
        pHit->SetTime();
        pHit->SetLength();
        pHit->SetTime(clockScale, useExtClock);
        pHit->SetChannel();
        pHit->Validate(hitSize);
    
        // Warn if this module's handing out of order hits:
        
        if (!warnedLate && (pHit->s_time < m_lastEmittedTimestamp)) {
            int module = ((*(pHit->s_data) >> 4) & 0xf);
            std::cerr << " Module " << module << " handed us a hit earlier "
                << "than the last one emitted. Last emitted: " << m_lastEmittedTimestamp
                << " hit: " << pHit->s_time << std::endl;
            std::cerr << "This might happen with a FIFO_THRESHOLD too big\n";
            
            warnedLate = true;
        }
        // Figure out the hit timestamp.
        // That's either the coarse timestamp or the
        // external timestamp, in either case multiplied
        // by the clockScale:
        
        
        
        hitList.push_back(pHit);

	
	
        p += hitSize*sizeof(uint32_t);
        size_t  hitWords = hitSize * sizeof(uint32_t)/sizeof(uint16_t);
        if (hitWords > bodySize) {
          std::stringstream msgstr;
            msgstr << "ddasSorter is about to run off the end of a ring item. "
                << " the last hit was " << hitWords << " 32 bit words long "
                << " and came from slotID " << ((*(pHit->s_data) >> 4) & 0xf)
                << " most likely the modevtlen value for this slot is incorrect\n";
            throw msgstr.str();
	}
        bodySize -= hitWords;
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
    pHit->freeHit();            // De-reference and possibly free buffer.
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
    
    // sizeof(BodyHeader) ok here because we're constructing the output
    // event and we know there's no extension.
    
    uint64_t ts = pHit->s_time;
    m_lastEmittedTimestamp = pHit->s_time;
    uint32_t bodySize  = pHit->s_channelLength + 2*sizeof(uint32_t)
                       + sizeof(BodyHeader) + sizeof(RingItemHeader) + 100;
    CPhysicsEventItem item(ts, m_sid, 0, bodySize);
    
    // Make this look like an old DDASReadout hit body:
    
    uint32_t* pBody = static_cast<uint32_t*>(item.getBodyCursor());
    *pBody++  = (pHit->s_channelLength + 2)*sizeof(uint32_t)/sizeof(uint16_t);
    *pBody++  = pHit->s_moduleType;
    memcpy(pBody, pHit->s_data, pHit->s_channelLength*sizeof(uint32_t));
    pBody    += pHit->s_channelLength;
    item.setBodyCursor(pBody);
    item.updateSize();
    item.commitToRing(m_sink);
}
