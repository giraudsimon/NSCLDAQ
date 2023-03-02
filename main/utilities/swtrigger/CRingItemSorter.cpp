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
#include <DataFormat.h>

#include <stdlib.h>
#include <fstream>
#include <stdexcept>


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
    // Create the vector of queues.
    
    DataQueue q;
    q.s_NoMore = false;                  // Worker still have more to contributes.
    for (int i =0; i < nWorkers;i++) {
        m_queues.push_back(q);           // index is worker id -1.
        m_activeWorkers.insert(i+1);       // So we know when we're done.
    }
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
    while (!m_activeWorkers.empty()) {
        m_pDataSource->getMessage(&pData, nBytes);
        if (nBytes == 0) {
            break;                               // Should not happen but:
            
        } else {
            // The first uint32_t is the id of the worker:
            // If that's all there is, that's an end for that worker.
            
            uint32_t* p = static_cast<uint32_t*>(pData);
            m_nCurrentWorker = *p;
            p++;
            nBytes -= sizeof(uint32_t);
            
            if (nBytes == 0) {
                workerExited();
            } else {
                process(p, nBytes);        // Remainder of message is what we care about.
            }

        }
    }
    if (canFlush()) flush();
    m_pDataSink->end();

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
 * @note  m_nCurrentWorker  contains the workerId of the worker the data came
 *        from.
 */
void
CRingItemSorter::process(void* pData, size_t nBytes)
{
    int index = m_nCurrentWorker - 1;       // Queue index.
    QueueElement q;
    q.first = nBytes;
    q.second = static_cast<pItem>(pData);
    
    // Data from each worker is time ordered so we just need to shove it in the
    // back of the queue and try to flush what we can flush:
    
    m_queues[index].s_DataQ.push_back(q);
    
    flush();
 
}
/////////////////////////////////////////////////////////////////////////
// Private methods:

/**
 * flush
 *    While all active queues have data, flush until there's an empty queue
 *    or all workers are done.
 */

void
CRingItemSorter::flush()
{
    // This simple algorithm assumes that we've got a big enough chunksize
    // that this O(2n) process is not too bad.
    
    std::vector<QueueElement> flushables;
    while(canFlush()) {
        flushables.push_back(earliestElement());
    }
    // Now construct the I/O vector and send the data:
    
    if (flushables.size()) {
        iovec parts[flushables.size()];
        
        for (int i =0; i < flushables.size(); i++) {
            parts[i].iov_base = flushables[i].second;
            parts[i].iov_len  = flushables[i].first;
        }
        m_pDataSink->sendMessage(parts, flushables.size());
        
        for (int i = 0;  i < flushables.size(); i++) {
            uint32_t* pItem = static_cast<uint32_t*>(parts[i].iov_base);
            pItem--;                     // Allow for the id
            free(pItem); 
        }
    }
    
}
/** workerExited
 *   The m_nCurrentWorker exited.
 *   -  Mark its queue as not expecting more data.
 *   -  remove it's id from the active worker set.
 */
void
CRingItemSorter::workerExited()
{
    m_queues[m_nCurrentWorker-1].s_NoMore = true;
    m_activeWorkers.erase(m_nCurrentWorker);
    if (canFlush()) flush();                          // Might be flushable now.
}

/**
 * canFlush
 *    @return true if it's ok to flush another data chunk:  It's ok to flush if the only
 *            empty queues are marked as s_NoMore.  Note that those queues
 *            _could_ contain data.
 */
bool
CRingItemSorter::canFlush()
{
    int counter(0);
    for (int i =0; i < m_queues.size(); i++) {
        if(!m_queues[i].s_NoMore && m_queues[i].s_DataQ.empty()) return false;
        if (!m_queues[i].s_DataQ.empty()) counter++;
    }
    return counter > 0;                 // No unflushable condition.
}
/**
 * earliestElement
 *    @return the queue element with the earliest timestamp.  This will be at the
 *            front of a queue.  The element will be popped off the queue.
 *    @throw std::logic_error - if all the queues are empty.
 */
CRingItemSorter::QueueElement
CRingItemSorter::earliestElement()
{
    uint64_t earliest = UINT64_MAX;
    QueueElement result;
    int      q        = -1;
    for (int i =0; i < m_queues.size(); i++) {
        if (!m_queues[i].s_DataQ.empty()) {
            QueueElement& candidate = m_queues[i].s_DataQ.front();
            if (candidate.second->s_timestamp < earliest) {
                result = candidate;
                q = i;
                earliest = candidate.second->s_timestamp;                
            }
        }
    }
    // If all queues were empty, q = -1
    
    if (q == -1) {
        throw std::logic_error("CRingItemSorter::canFlush called with all queues empty!");
    }
    m_queues[q].s_DataQ.pop_front();
    
    return result;
}
