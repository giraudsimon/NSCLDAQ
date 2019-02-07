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

/** @file:  CRingItemProcessor.cpp
 *  @brief: Provide Implementations of non-pure virtual methos of CRingItemProcessor.
 */
#include "CRingItemProcessor.h"
#include <CRingItem.h>
#include <DataFormat.h>


CRingItemProcessor::CRingItemProcessor() {}
CRingItemProcessor::~CRingItemProcessor() {}

/**
 * operator()
 *     Called with a message that consists of a PROCESS_ITEN whose payload
 *     is actually a CRingItem*.  The output is intended to be used with the
 *     ZMQ message passing sources and sinks.   As such,
 *     The input message is a PROCESS_ITEM message whose payload is a single
 *     data part whose pointer is a CRingItem*.  This gets passed to the
 *     user's process method (pure virtual here) which returns a
 *     list of output ring items (possibly empty).
 *     After deleting the input ring item, these output ring items get
 *     marshalled into a message:
 *     -   If there are no output ring items, an IGNORE messagse is produced.
 *     -   If there are output ring items, then a PROCESS_ITEM is produced
 *         with each data part containing a pointer to one of the ring items.
 *         in the list.
 * @param msg - the input message described above.
 * @return MessageType::Message - the output message described above.
 */
MessageType::Message
CRingItemProcessor::operator()(MessageType::Message& msg)
{
    // Get the ring item and call the user's code.
    
    CRingItem* pItem = static_cast<CRingItem*>(msg.s_dataParts.front().second);
    std::list<CRingItem*> output = process(*pItem);
    delete pItem;
    
    // Now format the output message
    
    MessageType::Message result;
    while (!output.empty()) {
        pItem = output.front();
        
        std::pair<uint32_t, void*> part;
        part.first = pItem->getItemPointer()->s_header.s_size;
        part.second = pItem;
        result.s_dataParts.push_back(part);
        
        output.pop_front();
    }
    
    // What we return for the type depends on whether or not any items
    // were actually produced.
    
    if (result.s_dataParts.empty()) {
        result.s_messageType = MessageType::IGNORE;   // no data.
    } else {
        result.s_messageType = MessageType::PROCESS_ITEM; // data to process.
    }
    
    return result;
}