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

/** @file:  CBuiltItemWorker.h
 *  @brief: Parallel worker with utilities for iterating over event fragments.
 */

#ifndef CBUILTITEMWORKER_H
#define CBUILTITEMWORKER_H
#include "CParallelWorker.h"
#include <DataFormat.h>
#include <fragment.h>
#include <stddef.h>
/**
 * @class CBuiltItemWorker
 *   We have a few utiltities that, in some way, operate on event built
 *   items.   These all have some common features in their process()
 *   method:
 *   - The need to know how many fragments there are in the event.
 *   - The need to iterate over those fragments.
 *
 *   This is a base class for those parallel workers that just extends
 *   CParallelWorker by adding those common  methods.
 */
class CBuiltItemWorker : public CParallelWorker
{
public:
    // Front of the  ring item in a work item has timestamp and the
    // rest of this.
    
    typedef struct __attribute__((__packed__)) _EventHeader {
        uint64_t             s_timestamp;  
        RingItemHeader       s_ringHeader;
        BodyHeader           s_bodyHeader;
        uint32_t             s_evbBodySize;
    } EventHeader, *pEventHeader;

    // This struct actually points to the entire fragment but we define this
    // because we'll need to adjust the payload size in the fragment header
    // and the ringitem header size if an extension is added on.
    
    typedef struct __attribute__((__packed__)) _FragmentItem {
        EVB::FragmentHeader   s_fragHeader;
        RingItemHeader        s_ringItemHeader;
    } FragmentItem, *pFragmentItem;

public:
    CBuiltItemWorker(CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId);
protected:
    
    //  Iterate fragments in a ring item.
    
    void* firstFragment(const void* pEvent);
    size_t countFragments(const void* pEvent);
    void*  nextFragment(const void* pData);
    
    // Iteratoe ring items in a block.
    
    size_t countItems(const void* pData, size_t nBytes);
    void* nextItem(const void* pData);
};

#endif