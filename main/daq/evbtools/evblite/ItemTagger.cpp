/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  ItemTagger.cpp
 *  @brief: implementation of the ItemTagger class.
 */
#include "ItemTagger.h"

#include <io.h>
#include <stdlib.h>
#include <stdexcept>

/**
 * constructor
 *    We just squirell away the parameters and
 *    set the timetsamp to zero.
 *  @param in - input fd.
 *  @param out output fd.
 *  @param blockSize - size of the read we'll put to  the block reader's
 *                     read method.
 *  @param resetOnBegin - flag that if true will cause the last timestamp
 *                     to 
 */
ItemTagger::ItemTagger(
    int in, int out, size_t blockSize, bool resetOnBegin, uint32_t sid
) :
m_blockSize(blockSize),
m_sourceFd(in),
m_sinkFd(out),
m_resetLastTSOnBegin(resetOnBegin),
m_lastTimestamp(0),
m_defaultSid(sid)
{}

/**
 * operator()
 *    actual entry point for the program
 *    This should  only return on an EOF on input.
 */
void
ItemTagger::operator()()
{
    // Create the data source and loop over all items in the
    // source:
    
    CRingFileBlockReader r(m_sourceFd);
    CRingBlockReader::DataDescriptor info;
    
    do{
        info = r.read(m_blockSize);
        
        processItems(info);
        
        if (info.s_nItems) {
            free(info.s_pData);    
        }
        
    } while (info.s_nItems > 0);
    
}
//////////////////////////////////////////////////////////////////
// private utilities:

/**
 * processItems
 *    Handles a block of ring items.
 *    We do this in a non-zero copy manner. Specifically,
 *    we have a fragment header we fill in for each item
 *    and a 2 element iovec with one element pointing at the
 *    fragment header and the other pointing at the payload.
 *    io::writeDataV is used to write this to file.
 *
 *    In this implementation we only do a ring item at a time:
 *   @param info - The description of a block of items read from
 *             a block reader.
 */
void
ItemTagger::processItems(const CRingBlockReader::DataDescriptor&  info)
{
    EVB::FragmentHeader header;
    iovec v[2];
    
    // the first element is constant:
    
    v[0].iov_base = &header;
    v[0].iov_len  = sizeof(header);
    
    uint8_t* p = reinterpret_cast<uint8_t*>(info.s_pData);
    for (int i = 0; i < info.s_nItems; i++) {
        pRingItem pItem = reinterpret_cast<pRingItem>(p);
        uint32_t size = pItem->s_header.s_size;
        
        v[1].iov_base = pItem;
        v[1].iov_len  = size;
     
        // If necessary reset last timestamp:
        
        if ((pItem->s_header.s_type == BEGIN_RUN) && m_resetLastTSOnBegin) {
            m_lastTimestamp = 0;
        }
        
        // Fill in the fragment header:
        
        fillFragHeader(header, pItem);
        
        // write the item:
        
        io::writeDataV(m_sinkFd, v, 2);
        
        p += size;
    }
    
}
/**
 * fillFragHeader
 *   Fill a fragment header from a ring item.
 *   - If the ring item is physics, it must have a body  header.
 *   - If not physics and no body header, the defaults sid and
 *     last timstamp are put in.
 *   - If not physics and there is a body header beware of the
 *     possibility the timestamp is NULL_TIMESTAMP  in which
 *     case we also need to fill in the prior timestamp.
 *      
 * @param hdr - reference to the header to fill.
 * @param pItem - ring item to fil lit from
 * @note we're going to support both 11 and 12 items when
 *       checking for presence of the body header.
 * @throw std::invalid_argument -physics item without body header.
 */
void
ItemTagger::fillFragHeader(EVB::FragmentHeader& hdr, const RingItem* pItem)
{
    
    hdr.s_size = pItem->s_header.s_size;
    if(hasBodyHeader(pItem)) {
        hdr.s_sourceId =
            pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId;
        hdr.s_barrier  =
            pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier;
        
        hdr.s_timestamp =
            pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp;
        if (hdr.s_timestamp == NULL_TIMESTAMP) {
            hdr.s_timestamp = m_lastTimestamp;    
        } else {
            m_lastTimestamp = hdr.s_timestamp;
        }
    } else {
        // No body header but PHYSICS_EVENT items must have one:
        
        if (pItem->s_header.s_type == PHYSICS_EVENT) {
            throw std::invalid_argument("ItemTagger - physics item without body header is illegal");
        }
        hdr.s_sourceId = m_defaultSid;
        hdr.s_timestamp = m_lastTimestamp;
        hdr.s_barrier = 0;
    }
}
