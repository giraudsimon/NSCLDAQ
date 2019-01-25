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

/** @file:  RingItemBlockConverter.h
 *  @brief: Converts data from a ring item block reader into a message.
 *  @note   An empty item results in an END_ITEM
 */

#ifndef RINGITEMBLOCKCONVERTER_H
#define RINGITEMBLOCKCONVERTER_H
#include "DataItemConverter.h"
#include "MessageTypes.h"

#include <CRingBlockReader.h>
#include <utility>
#include <stddef.h>

/**
 * @class RingBLockItemConverter
 *     Takes data from a swFilterRingBlockDataSource and
 *     turns it into message.  If there is any data it, the message
 *     type will be a PROCESS_ITEM and the first message part will be the
 *     number of ring items while the second one will be a dynamically
 *     allocated array of pointers to the ring items themselves.
 */
class RingItemBlockConverter : public DataItemConverter
{
public:
    RingItemBlockConverter();
    virtual ~RingItemBlockConverter();
    
    virtual  MessageType::Message operator()(std::pair<size_t, void*>& item);

private:
    std::pair<size_t, void*> makeItemPointers(
        CRingBlockReader::DataDescriptor& desc
    );
};


#endif
