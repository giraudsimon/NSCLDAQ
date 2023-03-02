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

/** @file:  CRingBlockDataSink.cpp
 *  @brief: Implements the CRingBlockDataSink class.
 */

#include "CRingBlockDataSink.h"
#include "CRingItemSorter.h"
#include "CSender.h"
#include <sys/uio.h>

/**
 * constructor
 *    Just constructs the base class:
 *
 *     @param src - data source.
 *     @param sink - where to write data.
 */
CRingBlockDataSink::CRingBlockDataSink(CReceiver& src, CSender& sink) :
    CDataSinkElement(src, sink)
{}

/**
 * destructor
 *     Currently null to support chaining:
 */
CRingBlockDataSink::~CRingBlockDataSink() {}

/**
 * Presented with a block of items from CRingItemSorter,
 * creates an iovec to write just the ring items and then writes them
 * to the sink.
 *   @param pData - Pointer to the message from the sorter.
 *   @param nBytes - Size of the message in bytes.
 */
void
CRingBlockDataSink::process(void* pData, size_t nBytes)
{
  if(nBytes) {

    size_t nItems = countRingItems(pData, nBytes);

    // nItems can be large, so allocate storage on the heap:
    
    std::vector<iovec> parts(nItems);
        
    CRingItemSorter::pItem p = static_cast<CRingItemSorter::pItem>(pData);
    for (size_t i = 0; i < nItems; i++) {
      parts[i].iov_base = &(p->s_item);
      parts[i].iov_len  = p->s_item.s_header.s_size;            
      p = static_cast<CRingItemSorter::pItem>(nextItem(p));
    }
    
    getSink()->sendMessage(parts.data(), parts.size());
    
  }
}
////////////////////////////////////////////////////////////////////////////
// Private methods:

/**
 * countRingItems
 *    Count the number of items in a message block received from the
 *    sorter.
 *
 *  @param pData - pointer to the data.
 *  @param nBytes - Number of bytes in the block.
 *  @return size_t - number of items in the block.
 */
size_t
CRingBlockDataSink::countRingItems(void* pData, size_t nBytes)
{
    
    size_t result(0);
    while (nBytes) {
        result++;
        nBytes -= itemSize(pData);
        pData   = nextItem(pData);
    }
    
    return result;
    
}
/**
 * itemSize
 *    @param pData - pointer to a message item.
 *    @return size_t - number of bytes in that item.
 */
size_t
CRingBlockDataSink::itemSize(void* pData)
{
    CRingItemSorter::pItem p = static_cast<CRingItemSorter::pItem>(pData);
    return sizeof(uint64_t) + p->s_item.s_header.s_size;
}
/**
 * nextItem
 *    @param pData - pointer to a message object.
 *    @return void* - pointer to the next message object.
 */
void*
CRingBlockDataSink::nextItem(void*  pData)
{
    size_t n = itemSize(pData);
    uint8_t* p = static_cast<uint8_t*>(pData);
    p += n;
    return p;
}
