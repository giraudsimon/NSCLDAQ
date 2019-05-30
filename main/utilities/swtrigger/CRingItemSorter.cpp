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

/** @file:  CRingItemSorter.cpp
 *  @brief: Implements the sorter.
 */
#include "CRingItemSorter.h"
#include "CSender.h"
#include "CReceiver.h"
#include <stdlib.h>

/**
 * constructor
 *    @param fanin - the receiver for data fanned in from the data sources.
 *    @param sink -  Where we send sorted data.
 *    @param window - the timestamp tick window that determines when to emit.
 *    @param nWorkers - Numbrer of workers that will send us end data messages.
 */
CRingItemSorter::CRingItemSorter(
    CReceiver& fanin, CSender& sink, uint64_t window, size_t nWorkers
) : m_pDataSource(&fanin), m_pDataSink(&sink), m_nTimeWindow(window),
    m_nEndsRemaining(nWorkers)
{
    
}

/**
 * destructor
 */
CRingItemSorter::~CRingItemSorter()
{
    delete m_pDataSource;
    delete m_pDataSink;
}

/**
 * operator()
 *   Main flow of control of the processing element.
 *   - For the most part just get messages from the
 *     data source and pass them on to process.
 *   - If an empty message is received, that's an end of data from a worker.
 *     decrement the ends remaining member and exit if it hits zero.
 *
 *  @note it's the process method's responsibility to destroy data.
 */
void
CRingItemSorter::operator()()
{
    void* pData;
    size_t nBytes;
    while (m_nEndsRemaining) {
        m_pDataSource->getMessage(&pData, nBytes);
        process(pData, nBytes);
    }
    flush();                    // Flush everything.

}

/**
 * process
 *    Called when a clump of ring items has been rpesented to the
 *    sorter from one of the clients.
 *    Each chunk of data is placed in the proper point of the Dequeue.
 *    If the difference between the front and back timestamps is
 *    larger than m_nTimeWindow; m_nTimewindow added to the front is passed
 *    to flush to flush that data out to the sink.
 *    
 * @param pData - pointer to the ring items.
 * @param nBytes - Number of bytes of data.
 */
void
CRingItemSorter::process(void* pData, size_t nBytes)
{
    // Pull out the timestamp of the first item:
    
    pItem p = static_cast<pItem>(pData);
    uint64_t timestamp = p->s_timestamp;
    QueueElement q = {nBytes, p};

    // insert the queue elements.
    
    // Special case: If the dequeue is empty just shove it in the front,
    // or the last element has a timestamp < ours just push back:
    
    if (
        m_QueuedData.empty() ||
        (m_QueuedData.back().second->s_timestamp < timestamp)
    ) {    
        m_QueuedData.push_back(q);
    } else if(m_QueuedData.front().second->s_timestamp > timestamp) {
        // Special case it's at the front:
        
        m_QueuedData.push_front(q);
    } else {
        // Need to find where to insert -- std::lower_bound gives us the
        // iterator to insert before.
        
        auto i = std::lower_bound(
            m_QueuedData.begin(), m_QueuedData.end(), q
        );
        --i;                    // Insert after here:
        m_QueuedData.insert(i, q);
    }

    // See if we can emit any:
    
    uint64_t tsFront = m_QueuedData.front().second->s_timestamp;
    uint64_t diff =
        m_QueuedData.back().second->s_timestamp - tsFront;
    if (diff > m_nTimeWindow) {
        flush(tsFront + m_nTimeWindow);
    }
    // If the last element of the last item is an end of run flush all.
    
    if (flushRun()) flush();
    
}
/////////////////////////////////////////////////////////////////////////
// Private methods:

/**
 * flush
 *    FLushes the data until the next item in the dequeue has
 *    a timestamp larger than the specified stamp.
 *    -  First the number of items is enumerated.
 *    -  Then an iovec is created to output the items as blocks.
 *    -  Then the sender's sendMessage is used to send the data.
 *
 * @param until - blocks with first timestamp larger than this won't be sent.
 */

void
CRingItemSorter::flush(uint64_t until)
{
    // Figure out how many blocks we can send:L
    
    size_t numBlocks;
    for (auto p = m_QueuedData.begin(); p != m_QueuedData.end(); ++p) {
        if (p->second->s_timestamp >= until) {
            break;
        }
        numBlocks++;
    }
    // Pass 2 to create the iovector.
    
    iovec parts[numBlocks];
    for (int i =0; i < numBlocks; i++) {
        parts[i].iov_base = m_QueuedData[i].second;   // Data...
        parts[i].iov_len = m_QueuedData[i].first;        // size of data.
    }
    
    // Send the data.
    
    m_pDataSink->sendMessage(parts, numBlocks);
    
    // Remove the sent blocks freeing the data.
    
    for (int i =0; i < numBlocks; i++) {
        free(m_QueuedData.front().second);
        m_QueuedData.pop_front();
    }
    
}
/**
 * flushRun
 *    
 *  @return bool - true if the last queue item has an end run in it.
 */
bool CRingItemSorter::flushRun()
{
    pItem p       = m_QueuedData.back().second;
    size_t nBytes = m_QueuedData.back().first;
    while (nBytes) {
        if(p->s_item.s_header.s_type = END_RUN) return true;
        
        // Point at the next item in the block:
        
        uint8_t* pBytes = reinterpret_cast<uint8_t*>(p);
        size_t  itemSize = sizeof(uint64_t) + p->s_item.s_header.s_size;
        pBytes += itemSize;
        nBytes -= itemSize;
        
        p = reinterpret_cast<pItem>(pBytes);
    }
    return false;
}
/**
 * operator< for queue elements.  This compares the timestamp in the item
 *           headers.
 * @return bool true if e1's timestamp is < e2's.
 */
bool operator<(CRingItemSorter::QueueElement& e1, CRingItemSorter::QueueElement&  e2)
{
    return e1.second->s_timestamp < e2.second->s_timestamp;
}