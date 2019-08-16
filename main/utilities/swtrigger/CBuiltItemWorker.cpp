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

/** @file:  CBuiltItemWorker.cpp
 *  @brief: Implement abstract base class for workers that iterate over built events.
 */

#include "CBuiltItemWorker.h"



/**
 * constructor
 *    Just defers to CParallelWorker.
 * @param fanin - Transport that is getting data from a fanout.
 * @param sink  - where we send processed data.
 * @param clientId - our client id.
 */
CBuiltItemWorker::CBuiltItemWorker(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId
) : CParallelWorker(fanin, sink, clientId)
{}


/**
 * countFragments
 *    Given a pointer to an event built body, returns the number
 *    of fragment in the event.  The pointer is pointing to the
 *    uint32_t at the front of the event body.
 *
 *  @param pEvent - pointer to the front of the event body.
 */
size_t
CBuiltItemWorker::countFragments(const void* pEvent)
{
    const uint32_t* p = static_cast<const uint32_t*>(pEvent);
    size_t nBytes = *p;
    nBytes -= sizeof(uint32_t);                        // Self Counting.
    
    void* pFrag = firstFragment(pEvent);
    size_t result(0);
    
    while(nBytes) {
        result++;
        pFragmentItem pItem = static_cast<pFragmentItem>(pFrag);
        
        pFrag = nextFragment(pFrag);
        nBytes -= sizeof(EVB::FragmentHeader) + pItem->s_ringItemHeader.s_size;
    }
    
    return result;
}

/**
 * firstFragment
 *    Returns a pointer to the first fragment of an event given a pointer
 *    to the event body.
 *
 *  @param pEvent - pointer to the entire event.
 *  @return void* - Pointer to the first event fragment.
 */
void*
CBuiltItemWorker::firstFragment(const void* pEvent)
{
    const uint32_t* p = static_cast<const uint32_t*>(pEvent);
    p++;
    
    return reinterpret_cast<void*>(const_cast<uint32_t*>(p));
}
/**
 * nextFragment
 *   Given a pointer to a fragment returns a pointer to the byte after
 *   the fragment ends.
 *
 * @param pData - pointer to the fragment.
 * @return void* - pointer to the byte after the fragment.
 */
void*
CBuiltItemWorker::nextFragment(const void* pData)
{
    const FragmentItem* pFrag = static_cast<const FragmentItem*>(pData);
    size_t fragmentSize = sizeof(EVB::FragmentHeader) + pFrag->s_ringItemHeader.s_size;
    
    uint8_t* pResult = reinterpret_cast<uint8_t*>(const_cast<pFragmentItem>(pFrag));
    pResult += fragmentSize;
    
    
    return pResult;
}

/**
 * countItems
 *    Count the number of ring items in a block.
 * @param pData - pointer to the data block that is stuffed with ring items.
 * @param nBytes - Number of bytes of data.
 * @return size_t - number of ring items in the block.
 */
size_t
CBuiltItemWorker::countItems(const void* pData, size_t nBytes)
{   size_t result = 0;
    while (nBytes) {
        result++;
        const EventHeader* p = static_cast<const EventHeader*>(pData);
        nBytes -= p->s_ringHeader.s_size + sizeof(uint64_t);
        
        pData = nextItem(pData);
    }
    
    return result;
}
/**
 * nextItem
 *    Given a pointer to a ring item returns a pointer to data just after it.
 *
 * @param pData - pointer to the ring item.
 * @return void* - pointer to the byt following the ring item.
 */
void*
CBuiltItemWorker::nextItem(const void* pData)
{
    const EventHeader* pItem = static_cast<const EventHeader*>(pData);
    uint8_t*  p = reinterpret_cast<uint8_t*>(const_cast<pEventHeader>(pItem));
    p += pItem->s_ringHeader.s_size + sizeof(uint64_t);
    
    return p;
}

