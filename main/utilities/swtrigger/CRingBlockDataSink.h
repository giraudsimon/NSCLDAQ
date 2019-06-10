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

/** @file:  CRingBlockDataSink.h
 *  @brief: Data sink for a block of ring items as sent from CRingItemSorter.
 */

#ifndef CRINGBLOCKDATASINK_H
#define CRINGBLOCKDATASINK_H
#include "CDataSinkElement.h"

/**
 * @class CRingBlockDataSink
 *    This class outputs the ring items from blocks of data sent
 *    to us by a ringitem sorter. (See CRingItemSorter.h for this format).
 *
 *    This class is needed to override the CDataSinkElement's process
 *    method so as to  send only the ring items and not the timestamps.
 */
class CRingBlockDataSink : public CDataSinkElement
{
public:
    CRingBlockDataSink(CReceiver& src, CSender& sink);
    virtual ~CRingBlockDataSink();
    
    virtual void process(void* pData, size_t nBytes);
private:
    size_t countRingItems(void* pData, size_t nBytes);
    size_t itemSize(void* pData);
    void*  nextItem(void* pData);
};

#endif