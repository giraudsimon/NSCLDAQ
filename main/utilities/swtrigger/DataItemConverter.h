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

/** @file:  DataItemConverter.h
 *  @brief: Interface for converting items from DataSources to Messages.
 */
#ifndef DATAITEMCONVERTER_H
#define DATAITEMCONVERTER_H
#include <utility>
#include "MessageTypes.h"
#include <stddef.h>
/**
 * @interface DataItemConverter
 *     Converts data items from a data source into messages.
 *     These are often closely paired with a family of data sources that
 *     produce output in formats well understood by this object.
 */
class DataItemConverter {
public:
    DataItemConverter() {}
    virtual ~DataItemConverter() {}
    
    virtual MessageType::Message operator()(std::pair<size_t, void*>& item) =0;
};


#endif