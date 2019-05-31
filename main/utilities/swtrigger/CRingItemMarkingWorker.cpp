/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file  CRingItemMarkingWorker.cpp
* @brief Implements the CRingItemMarkingWorker class.
*/

#include "CRingItemMarkingWorker.h"
#include "CSender.h"
#include "CRingItemSorter.h" // message format.

#include <CRingItem.h>
#include <CRingItemFactory.h>
#include <DataFormat.h>
#include <stdexcept>
#include <sys/uio.h>


// Shorthand for the messages:

typedef CRingItemSorter::Item Message, *pMessage;



/**
 * constructor
 *    Construct the base class and save our classifier instance.
 *
 * @param fanin - Transport that is a client to a fanout so we run in parallel.
 * @param sink  - Data sink - we'll put data here.
 * @param clientId - Our id with the fanout.
 * @param pClassifier - pointer to the classifier functional.
 */
CRingMarkingWorker::CRingMarkingWorker(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    Classifier* pClassifier
) : CParallelWorker(fanin, sink, clientId), m_pClassifier(pClassifier) {}

/**
 * process
 *    - Non physics items just pass through.
 *    - Physics items with no body header are an std::invalid_argument exception.
 *    - Physics items with body headers are passed to the classifier
 *      and emitted with the classification added  to the body header.
 */
void
CRingMarkingWorker::process(void* pData, size_t nBytes)
{
    CRingItem* pItem = CRingItemFactory::createRingItem(pData);
    if (pItem->type() != PHYSICS_EVENT) {
        outputItem(*pItem);
    } else if (pItem->hasBodyHeader()) {
        outputItem(*pItem, (*m_pClassifier)(*pItem));
    } else {
        throw std::invalid_argument("Physics event ring item with no body header");
    }
    delete pItem;
}
///////////////////////////////////////////////////////////////////////////////
// Private members:

/**
 * outputItem
 *   This version outputs the unmodified Ring item.
 *
 * @param item - references the CRingItem object.
 */
void
CRingMarkingWorker::outputItem(CRingItem& item)
{
    pRingItem p = item.getItemPointer();
    getSink()->sendMessage(p, p->s_header.s_size);
}
/**
 * outputItem
 *   This version outputs the ring item with the body header extended by
 *   a uint32_t containing the item's classification.
 *
 * @param item - the original ring item.
 * @param classification - the classification to insert.
 */
void
CRingMarkingWorker::outputItem(CRingItem& item, uint32_t classification)
{
    pRingItem p = item.getItemPointer();
    
    // We're assured by prior logic this has a body pointer, add sizeof(uint32_t)
    // to its size.
    
    p->s_body.u_hasBodyHeader.s_bodyHeader.s_size += sizeof(uint32_t);
    p->s_header.s_size += sizeof(uint32_t);      // Entire ring item is bigger too:
    
    iovec parts[3];      // up to old body header, classification and body.
    parts[0].iov_base = p;
    parts[0].iov_len  = sizeof(RingItemHeader) + sizeof(BodyHeader);
    
    parts[1].iov_base = &classification;
    parts[1].iov_len  = sizeof(uint32_t);
    
    // Since we've modified the body header etc... getBodyPointer works now:
    // getBodySize is only useable when building a ring item :-(
        
    parts[2].iov_base = item.getBodyPointer();
    parts[2].iov_len  =
        p->s_header.s_size  - p->s_body.u_hasBodyHeader.s_bodyHeader.s_size;
        
    getSink()->sendMessage(parts, 3);
    
}