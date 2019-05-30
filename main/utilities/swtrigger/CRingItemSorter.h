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

/** @file:  CRingItemSorter.h
 *  @brief: Sorts ring items by timestamps.
 */
#ifndef CRINGITEMSORTER_H
#define CRINGITEMSORTER_H

#include "CProcessingElement.h"
#include <DataFormat.h>
#include <deque>
#include <stdint.h>
#include <stddef.h>

class CReceiver;
class CSender;



/**
 * @class CRingItemSorter
 *    Does a time window sort kind of like the one done in DDAS and
 *    the one done by GRETINA's GEB.  Ring items are accumulated
 *    into a sorted dequeue and emitted once the  tail (newest element of the deque)
 *    is newer by at least some fixed time-window than the front (oldest).
 *
 *   Data are assumed to come in chunks of ringitems that are, themselves,
 *   sorted.  Since the initial data source is sorted,  these chunks can be
 *   maintained and emitted as chunks.
 *
 *   @note We're told how many sources we have and when the
 *   last source gives us an end of data, we exit.
 *   @note In order to operate in an online environment, If the tail block
 *         ends with an end of run item, the queue is flushed.  This relies
 *         on the fact that the end of run items are a barrier and, therefore
 *         will be clumped together.
 *         
 */
class CRingItemSorter  : public CProcessingElement
{
    // Data types for the dequeue.  Each dequeue element is a size
    // and a block of ring items:
    
    typedef std::pair<size_t, pRingItem*>  QueueElement;
private:
    CReceiver*   m_pDataSource;
    CSender*     m_pDataSink;
    uint64_t     m_nTimeWindow;
    std::deque<QueueElement>  m_pQueuedData;
    size_t       m_nEndsRemaining;

public:
    CRingItemSorter(
        CReceiver& fanin, CSender& sink, uint64_t window, size_t nWorkers
    );
    virtual ~CRingItemSorter();
    virtual void operator()();
    virtual void process(void* pData, size_t nBytes);
};


#endif
