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

/** @file:  SingleRingItemConverter.cpp
 *  @brief: Implement the converter.
 */
#include "SingleRingItemConverter.h"
#include <DataFormat.h>
#include "MessageTypes.h"

/**
 * constructor is pretty empty for now:
 */

SingleRingItemConverter::SingleRingItemConverter()
{}

/**
 * destructor is also empty for now:
 */
SingleRingItemConverter::~SingleRingItemConverter()
{}

/**
 * operator()
 *    Converts a pair that represents a single ring item into
 *    a message with an appropriate type and payload pointer.
 *    The ring item contents are already assumed to be malloced so
 *    we don't copy.
 */
MessageType::Message
SingleRingItemConverter::operator()(std::pair<size_t, void*>& item)
{
    MessageType::Message result;
    // null sized means its' end:
    
    if (item.first > 0) {
        uint32_t size = item.first;
        result.s_messageType = MessageType::PROCESS_ITEM;    //It's an item to process.
        result.s_dataParts.push_back({size, item.second});
    } else {
        result.s_messageType = MessageType::END_ITEM;    // No more data expected
    }
    
    
    return result;
}