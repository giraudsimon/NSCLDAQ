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

/** @file:  CRingItemZMQSourceElement.cpp
 *  @brief: Implement the ring item ZMQ router.
 */
#include "CRingItemZMQSourceElement.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemTransport.h"
#include "CZMQRouterTransport.h"
#include "CReceiver.h"
#include "CSender.h"

#include <CRingBuffer.h>
#include <DataFormat.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <array>
#include <fragment.h>

/**
 * constructor
 *    Create the receiver (data source) using a transport produced by  the
 *    CRingItemTrasnportFactory and hand a CZMQRouter as the transport.
 *
 *
 *   @param ringUri - specifies the ring data source.
 *   @param routerUri - Specifies the URI of the ZMQ router.
 *   @param chunkSize - Number of ring items that are sent in each message.
 *
 *   @note each ring item is sent preceded by a 64 bit timestamp.  Where possible,
 *   this timestamp is taken from the body header.  Otherwise a synthetic timestamp
 *   is generated that preserves ordering.
 */
CRingItemZMQSourceElement::CRingItemZMQSourceElement(
    const char* ringUri, const char* routerUri,
    size_t chunkSize
) :
    CDataSourceElement(*(new CReceiver(
        *CRingItemTransportFactory::createTransport(
            ringUri, CRingBuffer::consumer
        ))),
        *(new CZMQRouterTransport(routerUri))
    ), m_nChunkSize(chunkSize), m_nLastTimestamp(0)
{}
/**
 * operator()
 *     Flow control.  This must be overridden because we want process
 *     to control the memory allocation of messages to allow for chunking
 *     without undue data movement.
 */

void
CRingItemZMQSourceElement::operator()()
{
    void* pData;
    size_t nBytes(0);
    do {
        getSource()->getMessage(&pData, nBytes);
        process(pData, nBytes);
 
    } while(nBytes > 0);
}
/**
 * process
 *    - If the message is an end (nBytes == 0), the chunk is sent.
 *    - The timestamp is extacted or creatd as required by the data
 *    - A Message is built and stuffed on to the back of the m_chunk array.
 *    - If m_chunk.size() == m_nChunkSize, sendChunk is called to send the chunk
 *      to the next requestor.
 *
 *  @param pData -pointer to the rung item received.
 *  @param nBytes - size of the ring item.
 */
void
CRingItemZMQSourceElement::process(void* pData, size_t nBytes)
{
    if (nBytes == 0) {                       // End of data.
        sendChunk();
        
    } else {                               // Real data.
        Message m;
        m.s_pData = pData;
        m.s_nBytes = nBytes;
        
        // Figure out the timestamp.. if there's a body header it comes from there.
        // otherwise it comes from the last timestamp seen.
        
        pRingItem pItem = static_cast<pRingItem>(pData);
        if (pItem->s_header.s_type == RING_FORMAT) {
            // Beginning of a run so:
            
            m_nLastTimestamp = 0;
        } else if (pItem->s_body.u_noBodyHeader.s_mbz) {   // have a body header else:
            uint64_t bheadertstamp =
                pItem->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp;
            if (bheadertstamp == NULL_TIMESTAMP) {
                bheadertstamp = m_nLastTimestamp;            // Null timestamp means last
            }
            m_nLastTimestamp = bheadertstamp;
           
        }
        // By now m_nLasTimetamp is what we want.
        m.s_timestamp = m_nLastTimestamp;
        m_chunk.push_back(m);
        
        if(m_chunk.size() >= m_nChunkSize) {
            sendChunk();
        }
    }
}
/**
 * sendChunk
 *    Send a chunk of data to the sender
 *    Once the chunk is sent, the data in the m_chunk vector is freed
 *    and the vector itself cleared for the next chunk.
 */
void
CRingItemZMQSourceElement::sendChunk()
{
    // For each chunk we need to iov elements
    // one for the timestamp and one for the ring item itself:
    
    std::vector<iovec> parts(m_chunk.size()*2);
    size_t n(0);
    for (int  i =0; i < m_chunk.size(); i++) {
        parts[n].iov_base = &m_chunk[i].s_timestamp;
        parts[n].iov_len  = sizeof(uint64_t);
        
        n++;
        parts[n].iov_base = m_chunk[i].s_pData;
        parts[n].iov_len  = m_chunk[i].s_nBytes;
        n++;
    }
    // Send the message:
    
    CSender* pSender = getSender();
    pSender->sendMessage(parts.data(), parts.size());
    
    // Finally release chunk storage and clear the vector:
    
    clearChunk();
}
/**
 * clearChunk:
 *     - free each data chunk of each message in chunk and
 *     - clear the vector m_chunk.
 */
void
CRingItemZMQSourceElement::clearChunk()
{
    for (int i = 0; i < m_chunk.size(); i++ ) {
        free(m_chunk[i].s_pData);
    }
    
    m_chunk.clear();
}