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

/** @file:  RingItemBlockConverter.cpp
 *  @brief: Convert ring items from CRingBlockReader classes into messages.
 */
#include "RingItemBlockConverter.h"
#include <CRingBlockReader.h>

#include <stdlib.h>


/**
 * constructor - no op.
 */
RingItemBlockConverter::RingItemBlockConverter()
{}

/** Destructor - no op.
 */

RingItemBlockConverter::~RingItemBlockConverter()
{}


/**
 * operator()
 *    Performs the conversion.  See the header comments for
 *    what's actually produced.
 *
 * @param item - describes the block of ring items read by the
 *               data source.
 *  @return MessageType::Message - the message-ized version of the data.
 *  
 */
MessageType::Message
RingItemBlockConverter::operator()(std::pair<size_t, void*>& item)
{
    MessageType::Message result;
        
    if ((item.first) != 0 && (item.second != nullptr)) {
        // There's data to return:
        CRingBlockReader::pDataDescriptor pD =
            static_cast<CRingBlockReader::pDataDescriptor>(item.second);
        
        std::pair<size_t, void*> ptrs = makeItemPointers(*pD);
        result.s_messageType = MessageType::PROCESS_ITEM;
        uint32_t* nItems = static_cast<uint32_t*>(malloc(sizeof(uint32_t)));
        *nItems = ptrs.first;
        
        result.s_dataParts.push_back({sizeof(uint32_t), nItems});
        result.s_dataParts.push_back({sizeof(void*)*(*nItems), ptrs.second});
        
    } else {
        // end of data:
        
        result.s_messageType = MessageType::END_ITEM;
    }
    
    return result;
}
/////////////////////////////////////////////////////////////////////////
// Private utilities

/**
 * makeItemPointers
 *    Given a data descriptor produces a pointer to a dynamically
 *    allocated array of pointers to the underlying ring items.
 *
 * @param desc - the data descriptor.
 * @return std::pair<size_t, void*> - first is the number of elements
 *                                    second is a pointer to the pointer array.
 */
std::pair<size_t, void*>
RingItemBlockConverter::makeItemPointers(CRingBlockReader::DataDescriptor& desc)
{
    // Create our result
    
    std::pair<size_t, void*> result;
    result.first = desc.s_nItems;
    result.second = malloc(result.first*sizeof(void*));
    
    // Now fill in the pointer array.
    
    void** pr = static_cast<void**>(result.second); // pointer array.
    
    // pointers to the current rin gitem.
    
    uint32_t* pd = static_cast<uint32_t*>(desc.s_pData);
    uint8_t*  pdp= static_cast<uint8_t*>(desc.s_pData);
    
    for (int i = 0; i < result.first; i++) {
        *pr++ = pd;                      // Store ring item pointer.
        
        pdp+= *pd;                      // Ritems start with a size.
        pd   = reinterpret_cast<uint32_t*>(pdp);
    }
    
    // 
    
    return result;
    
}