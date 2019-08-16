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

/** @file:  CBuiltRingItemExtender
 *  @brief: Implement the CBuiltRingItemExtender class see the header for more.
 */
#include "CBuiltRingItemExtender.h"
#include "CSender.h"
#include "CRingItemSorter.h" // message format.

#include <fragment.h>
#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string.h>




/**
 * constructor
 *   @param fanin - the data source.  We are taking data from a fanout transport.
 *   @param sink  - Where our processed ring items go.  Normally this is a fanin
 *                  to a ring item sorter to put things back in time order.
 *   @param clientId - My fanout client id.  We provide that to the sorter
 *                  so it can manage its queues properly.
 *   @param pExtender - The user code that can provide our event fragments with
 *                   extension data.
 */
CBuiltRingItemExtender::CBuiltRingItemExtender(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    CRingItemExtender* pExtender
) :
    CBuiltItemWorker(fanin, sink, clientId), m_pExtender(pExtender), m_nId(clientId),
    m_nBlocks(0),
    m_pIoVectors(nullptr), m_nIoVectorCount(0), m_nUsedIoVectors(0)
{}

/**
 * destructor
 *    The I/O vectors are dynamic and need to be freed (free(3) not delete).
 */
CBuiltRingItemExtender::~CBuiltRingItemExtender()
{
    free(m_pIoVectors);
}
/**
 * process
 *   This gets data from the fanout.  Each datum is just a block of ring items.
 *   When we send our reply block we need to know how many I/O vectors we might
 *   be in for.
 *   *  One that points to our client id for the sorter.
 *   *  For each ring item:
 *      - One for the ring item header up through the body size from the
 *        event builder.
 *      - One for each fragment.
 *      - One for the potential extension data for each fragment.s
 *
 * @param pData  - Data block.
 * @param nBytes - Number of bytes of data.
 * @note the base class gets the data from the fanout and frees dynamic
 *       storage associated with pDataq for us.
 * @note nBytes == 0 is legal and means end of data.
 */
void
CBuiltRingItemExtender::process(void* pData, size_t nBytes)
{

    if (nBytes) {
        size_t maxVecs = iovecsNeeded(pData, nBytes);
        allocateIoVectors(maxVecs);
    
        // We send our source id at the front.
        
        m_pIoVectors[0].iov_len = sizeof(uint32_t);
        m_pIoVectors[0].iov_base = &m_nId;
        m_nUsedIoVectors = 1;
        
        // This vector of I/O vectors is used to keep track of the extensions
        // that have to be freed:
        
        std::vector<iovec> extensions;
        
        // Now we need to loop over the ring items:
        
        size_t nRingItems = countItems(pData, nBytes);
        void* pItem = pData;
        for (int i =0; i < nRingItems; i++) {
            
            pEventHeader pItemHeader = static_cast<pEventHeader>(pData);
            
            
            
            
            if (pItemHeader->s_ringHeader.s_type == PHYSICS_EVENT) {
                // For each ring item we have an I/Vec for the
                // data in the ring item header, its body header and the
                // event builder body size.  We'll also hold a pointer to that
                // data so that we can adjust sizes as extensions are added:
                
                
                m_pIoVectors[m_nUsedIoVectors].iov_base = pData;
                m_pIoVectors[m_nUsedIoVectors].iov_len  = sizeof(EventHeader);
                m_nUsedIoVectors++;

                
                // Now we need to  loop over the fragments in each event giving
                // the extender a chance to add an extension to the fragment:
                
                void* pEvent = reinterpret_cast<void*>(&(pItemHeader->s_evbBodySize));
                size_t nFragments = countFragments(pEvent);
                void*  pFrag      = firstFragment(pEvent);
                
                // Since we're going to corrupt the ring item sizes etc. we need
                // to do this before iterating over the fragments:
                
                pData = nextItem(pData);
                
                for (int j =0; j < nFragments; j++) {
                    // The fragment points to something like:
                    
                    pFragmentItem pFragFront = static_cast<pFragmentItem>(pFrag);
                    
                    // The iovec for the fragment as a whole:
                    
                    m_pIoVectors[m_nUsedIoVectors].iov_base =  pFrag;
                    m_pIoVectors[m_nUsedIoVectors].iov_len  =
                        sizeof(EVB::FragmentHeader) +
                        pFragFront->s_ringItemHeader.s_size;
                    m_nUsedIoVectors++;
                    
                    pRingItem pFragmentRingItem =
                        reinterpret_cast<pRingItem>(&(pFragFront->s_ringItemHeader));
                    iovec extension = (*m_pExtender)(pFragmentRingItem);
                    
                    // There's an extension if the size is nonzero:
                    
                    pFrag = nextFragment(pFrag);    // Need to do this before adjustments tosize.
                    if(extension.iov_len) {
                        m_pIoVectors[m_nUsedIoVectors] = extension;
                        m_nUsedIoVectors++;
                        extensions.push_back(extension);
                        
                        // Adjust the sizes:
                        
                        pItemHeader->s_evbBodySize += extension.iov_len;     // EVB size,
                        pFragFront->s_ringItemHeader.s_size += extension.iov_len; // Current fragment ringitem.
                        pFragFront->s_fragHeader.s_size     += extension.iov_len; // Fragment payload size.
                        pItemHeader->s_ringHeader.s_size += extension.iov_len;  // full ring .
                    }
                    
                    
                }
            } else {
                // Non physics items are just tossed out as a single beast:
                
                m_pIoVectors[m_nUsedIoVectors].iov_base = pData;
                m_pIoVectors[m_nUsedIoVectors].iov_len
                    = sizeof(uint64_t)  + pItemHeader->s_ringHeader.s_size;
                m_nUsedIoVectors++;
                pData = nextItem(pData);
            }
            
        }
        // At this point, the IOVec has been built and m_nUsedIoVectors is the
        // number of vectors.  Send the message then free the extension data:
        
        getSink()->sendMessage(m_pIoVectors, m_nUsedIoVectors);
        for (int i =0; i < extensions.size(); i++) {
            m_pExtender->free(extensions[i]);
        }      
    } else {
        getSink()->sendMessage(&m_nId, sizeof(m_nId));  // end of data for our id.
    }
    m_nBlocks++;    
}
///////////////////////////////////////////////////////////////////////////////
// Private utilities:



/**
 * iovecsNeeded
 *    @param pData - pointer to the raw data block.
 *    @return size_t - maximum number of I/O vectors we need to represent the output.
 *                     This is computed by assuming all fragments need an extension.
 */
size_t
CBuiltRingItemExtender::iovecsNeeded(const void* pData, size_t nBytes)
{
    size_t result = 1;                 // For our id.
    size_t nRingItems = countItems(pData, nBytes);
    result += nRingItems;              // Each Event needs one item.
    
    for (int i =0; i < nRingItems; i++) {
                           // Need one for the event or header.
        const EventHeader* pEventHdr = static_cast<const EventHeader*>(pData);
        if (pEventHdr->s_ringHeader.s_type == PHYSICS_EVENT) {
            const void* pBody             = &(pEventHdr->s_evbBodySize);
            size_t frags = countFragments(pBody);
            
            // Each fragment needs at most two iovecs:
            
            result += frags*2;
        }                             // Only physics event have fragments.
        pData = nextItem(pData);
    }

    return result;
}
/**
 * allocateIoVectors
 *    If needed, replaces the m_pIoVectors storage with enough
 *    to fit the requested numnber of iovecs..
 *  @param needed - number of IO vectors needed.
 */
void
CBuiltRingItemExtender::allocateIoVectors(size_t needed)
{
    // Only adjust if needed is > than what we have:
    
    
    if (needed > m_nIoVectorCount) {
        free(m_pIoVectors);             // Harmless if nullptr.
        m_pIoVectors = static_cast<iovec*>(malloc(needed * sizeof(iovec)));
        if (!m_pIoVectors) {
            // probably fatal:
            
            throw std::string(
                "Could not allocate sufficient i/o vector for CBuitlRingItemExtender"
            );
        }
        m_nIoVectorCount = needed;
    }
    // If needed <= what we have, leave everything alone.
}