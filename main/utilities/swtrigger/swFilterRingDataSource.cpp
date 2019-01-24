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

/** @file:  swFilterRingDataSource.cpp
 *  @brief: Implements the swFilterRingDataSource class.
 */

#include "swFilterRingDataSource.h"
#include <CDataSource.h>
#include <CRingItem.h>
#include <DataFormat.h> 
#include <stdlib.h>
#include <string.h>
#include <new>

/**
 * constructor
 *    @param src - The actual source of data.  Ownership remains with the
 *                 caller.. that allows them to use statically allocated as
 *                 well as dynamically allocated sources.
 */

swFilterRingDataSource::swFilterRingDataSource(CDataSource& src) :
    m_ActualSource(src)
{}

/**
 * destructor
 *    for now we don't need to do anything.
 */
swFilterRingDataSource::~swFilterRingDataSource()
{}

/**
 * connect
 *    No need to connect as the data source is already connected.
 */
void
swFilterRingDataSource::connect()
{}

/**
 * disconnect
 *    No need to do anything here either.
 */
void
swFilterRingDataSource::disconnect()
{}

/**
 * read
 *    Gets the next ring item from the data source. Since there's now
 *    an assumption the pointer points to storage allocated by
 *    malloc, we copy the raw ring item into malloced' storage.
 *    The ring item we retrieved is deleted prior to return as required
 *    by the ring item data source.
 * @return std::pair<std::size_t, void*> - first is the size of the ring item.
 *                                    second points to the ring item itself.
 */
std::pair<std::size_t, void*>
swFilterRingDataSource::read()
{
    std::pair<std::size_t, void*> result = {0, nullptr};  // assume at end.
    
    CRingItem* pWrappedItem = m_ActualSource.getItem();
    
    // NULL is returned for an end of data stream.
    
    if (pWrappedItem) {
        pRingItem pRawItem = pWrappedItem->getItemPointer();
        result.first = pRawItem->s_header.s_size;
        result.second = malloc(result.first);
        if (!result.second) {
           throw std::bad_alloc(); 
        }
        memcpy(result.second, pRawItem, result.first);
        
        delete pWrappedItem;
    }
    
    return result;
}

