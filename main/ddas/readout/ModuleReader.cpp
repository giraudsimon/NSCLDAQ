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

/** @file:  ModuleReader.cpp
 *  @brief: Implement the ModuleReader
 */
#include <config.h>
#include "ModuleReader.h"
#include "ReferenceCountedBuffer.h"
#include "ZeroCopyHit.h"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string.h>

namespace DDASReadout {
    
/**
 * constructor
 *     Just save all the stuff for now:
 *
 *  @param module   - module number (needed for the read).
 *  @param evtlen   - Expected event length.
 *  @param moduleType -the module type word.
 *  @param timeMultiplier - Time calibration multiplier defaults to 1.0
 */
ModuleReader::ModuleReader(
    unsigned module, unsigned evtlen, uint32_t moduleType, double timeMultiplier
) :
    m_nModuleNumber(module), m_nExpectedEventLength(evtlen),
    m_tsMultiplier(timeMultiplier), m_moduleTypeWord(moduleType)
{
    reset();
    
}

/**
 * destructor
 *   kill off dynamic data we have. Note that if this is called prior to having
 *   all buffers returned holy hell will break loose -- eventually as there's
 *   no place to return the reference counted buffers.
 */
ModuleReader::~ModuleReader()
{
    
    // The buffer pool cleans itself up.  We need to clean up our hit pool:
    
    while (!m_freeHits.empty()) {
        ZeroCopyHit* pHit = m_freeHits.front();
        delete pHit;
        m_freeHits.pop_front();
    }
}

/**
 * read
 *    Reads a block of data from the module and marshalls it int oa hit list.
 *    Note that the hit list is not cleared, the data will be appended to any
 *    existing data.
 *
 *  @param hits - will get the parsed hits.
 *  @param nWords - Maximum read size .  If necessary this is reduced to a
 *                  size that is a multiple of the event length.  Note that this
 *                  value and the m_nExpectedEventLength are in uint32_t units.
 *  @note - Zerocopy strategy is used to ensure that once the data are read,
 *          they are not copied.
 *  @return size_t - number of words actually read.
 */
size_t
ModuleReader::read(HitList& hits, size_t nWords)
{
  int readstat;

  
    // Make nWords a multiple of m_nExpectedEventLength.

    unsigned remainder = nWords % m_nExpectedEventLength;;
    nWords   = nWords - remainder;
    if (nWords > 0) {
        ReferenceCountedBuffer* pBuffer =
            m_freeBuffers.allocate(nWords*sizeof(uint32_t));
        if ((readstat = Pixie16ReadDataFromExternalFIFO(
	   static_cast<unsigned int*>(pBuffer->s_pData), (unsigned long)(nWords),
	   (unsigned short)(m_nModuleNumber)
	   )) != 0) {
	    std::cerr << "Error reading module " << m_nModuleNumber << " FIFO\n";
	    std::cerr << "Tried to read " << nWords << " uin32_t's of data\n";
	    std::cerr << " Status: " << readstat << std::endl;
            std::cerr << "Acting as if there are no words to read\n";
            return 0;
        }
        parseHits(hits, *pBuffer, nWords);           // Zero copy process hits.
    }
    
    return nWords;
}
/**
 * freeHit
 *    Free a hit back to its appropriate hit pool.
 *
 *  @param hit - hit information.
 */
void
ModuleReader::freeHit(HitInfo& hit)
{
    hit.second->freeHit();                       // Prepare for re-use.
    hit.first->m_freeHits.push_back(hit.second);
}
/**
 * reset
 *    reset module last timestamps.
 */
void
ModuleReader::reset()
{
  std::cerr << "Resetting last channel timestamps on module: " << m_nModuleNumber << std::endl;
    memset(m_lastStamps, 0, 16*sizeof(double));  // start with stamps of zero.
    
}

////////////////////////////////////////////////////////////////////////////////
// Private utility methods.
//

/**
 * parseHits
 *    Creates a hit list that contains the events in a buffer read from the
 *    system.  Complains if any event is not the correct size.
 *
 *
 *  @param[out] hits - reference the hit list into which these hits will be
 *                     appended.
 *  @param[inout] pBuffer - the buffer containing the events. Note that
 *                     since hits are zero copy the buffer's reference
 *                     count will be incremented once for each event found.
 *  @param[in] nUsedWords - number of words read into the buffer.
 *  
 * @throw std::length_error is thrown if any of the hits in the buffer is not
 *                    the correct size (as defined by m_nExpectedEventLength).
 * @note The members of each hit are fully filled in with the data from
 *       the raw hit information.
 */
void
ModuleReader::parseHits(HitList& hits, ReferenceCountedBuffer& pBuffer, size_t nUsedWords)
{
    uint32_t* pData = (uint32_t*)(pBuffer);
    while(nUsedWords > 0) {
        uint32_t size = RawChannel::channelLength(pData);
        ZeroCopyHit* pHit = allocateHit();
        pHit->setHit(size, pData, &pBuffer, &m_freeBuffers);
        HitInfo hit = std::make_pair(this, pHit);
        if(pHit->Validate(m_nExpectedEventLength)) {
            std::stringstream s;
            s << "Event length inconsistent with event length in event length. \n";
            s << "Expected " << m_nExpectedEventLength << " got " << pHit->s_channelLength
              << " module number: " << m_nModuleNumber;
            throw std::length_error(s.str());
        }
        if(pHit->SetTime(m_tsMultiplier)) {
            std::cerr << "Warning Hit from module" << m_nModuleNumber
                << " does not contain a full header : tossing the  hit\n";
            
            freeHit(hit);
            continue;
        }
        if(pHit->SetChannel()) {
            std::cerr << "Warning Hit from module" << m_nModuleNumber
                << " does not contain a full header : tossing the  hit\n";
                
            freeHit(hit);
            continue;
        }
        checkOrder(pHit);
        hits.push_back(hit);
        
        pData += size;
        nUsedWords -= size;
        
    }
}
/**
 * allocateHit
 *    @return ZeroCopyHit*  - pointer to a neew hit.  If possible, this comes
 *                            from the hit pool.  If not, a new one is created
 *                            and, when it's finally freed, it will go back to the
 *                            hit pool.  The goal is that in the end the hit pool
 *                            will be large enough to satisfy all request without
 *                            dynamic memory allocation.
 */
ZeroCopyHit*
ModuleReader::allocateHit()
{
    if(m_freeHits.empty()) {
        m_freeHits.push_back(new ZeroCopyHit);
    }
    ZeroCopyHit* pResult = m_freeHits.front();
    m_freeHits.pop_front();
    
    return pResult;
}
/**
 * checkOrder
 *    Given a parsed hit, determine if it has a good timestamp and output
 *    an error message if not:
 *    -  The timestamp is bad if it's less than the last one from the channel
 *       since within a channel times monotonically increase.
 *    - The timestamp is bad (different message) if it's the same as the
 *      last timestamp from that channel.
 *
 * @param pHit - Pointer to the hit.
 */
void ModuleReader::checkOrder(ZeroCopyHit* pHit)
{
    double newTime = pHit->s_time;
    int    ch      = pHit->s_chanid;
    double oldTime = m_lastStamps[ch];
    m_lastStamps[ch] = newTime;
    
    if (newTime == oldTime) {
        std::cerr << "**Warning module " << m_nModuleNumber <<
            " channel " << ch << " Time is not incresing at timestamp " << newTime
            << std::endl;
    }
    if (newTime < oldTime) {
        std::cerr << "**Error: module " << m_nModuleNumber <<
            " channel " << ch << " time went backwards!!! Previous timestamp: " <<
            oldTime << " current timestamp: " << newTime << std::endl;
    }
}
}                             // namespace.
