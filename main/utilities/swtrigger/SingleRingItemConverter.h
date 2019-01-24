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

/** @file:  SingleRingItemConverter.h
 *  @brief: DataConverter for sources that provide a single ring item.
 *  @note  This converter will work just dandy with swFilterRingDataSource.
 */

#ifndef SINGLERINGITEMCONVERTER_H
#define SINGLERINGITEMCONVERTER_H

/**
 * @class SingleRingItemConverter
 *    A data converter that, when handed an item consisting of
 *    -  ring item size.
 *    - pointer to a single ring item
 *
 *    Produces an appropriate message data structure
 *    for the processing framework.
 *
 *    @note edge case: If the pair handed is {0, nullptr}, the
 *          message produced is an END_ITEM
 */

#include "DataItemConverter.h"

class SingleRingItemConverter : public DataItemConverter
{
public:
    SingleRingItemConverter();
    virtual ~SingleRingItemConverter();
    
    virtual MessageType::Message operator()(std::pair<size_t, void*>& item);
};

#endif