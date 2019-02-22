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
#include "ModuleReader.h"
#include "ReferenceCountedBuffer.h"
#include "ZeroCopyHit.h"
#include <sstream>
#include <stdexcept>

#include <pixie16app_export.h>
#include <pixie16sys_export.h>

namespace DDASReadout {
    
/**
 * constructor
 *     Just save all the stuff for now:
 *
 *  @param module   - module number (needed for the read).
 *  @param evtlen   - Expected event length.
 *  @param timeMultiplier - Time calibration multiplier
 */
ModuleReader::ModuleReader(unsigned module, unsigned evtlen, double timeMultiplier) :
    m_nModuleNumber(module), m_nExpectedEventLength(evtlen), m_tsMultiplier(timeMultiplier)
{}

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
    // Make nWords a multiple of m_nExpectedEventLength.
    
    unsigned remainder = nWords % m_nExpectedEventLength;;
    nWords   = nWords - remainder;
    if (nWords > 0) {
        ReferenceCountedBuffer* pBuffer =
            m_freeBuffers.allocate(nWords*sizeof(uint32_t));
        if (Pixie16ReadDataFromExternalFIFO(
                static_cast<uint*>(pBuffer->s_pData), nWords, m_nModuleNumber
            ) != 0) {
            std::stringstream msg;
            msg << "Error reading module " << m_nModuleNumber << " FIFO";
            throw std::runtime_error(msg.str());
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
ModuleReader::parseHits(HitList& hits, ReferenceCountedBuffer& pBuffer, size_t nUsedBytes)
{
    uint32_t* pData = (uint32_t*)(pBuffer);
    while(nUsedBytes < 0) {
        uint32_t size = RawChannel::channelLength(pData);
        ZeroCopyHit* pHit = allocateHit();
        pHit->setHit(size, pData, &pBuffer, &m_freeBuffers);
        if(pHit->Validate(m_nExpectedEventLength)) {
            std::stringstream s;
            s << "Event length inconsistent with event length in event length. \n";
            s << "Expected " << m_nExpectedEventLength << " got " << pHit->s_channelLength
              << " module number: " << m_nModuleNumber;
            throw std::length_error(s.str());
        }
        pHit->SetTime(m_tsMultiplier);
        pHit->SetChannel();
        
        hits.push_back({this, pHit});
        
        pData += size;
        nUsedBytes -= size;
        
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

}                             // namespace.