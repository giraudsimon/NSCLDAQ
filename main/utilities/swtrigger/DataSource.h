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

/** @file:  DataSource.h
 *  @brief: Interface for data sources (e.g. file or ring buffers).
 */
#ifndef DATASOURCE_H
#define DATASOURCE_H
#include <utility>

/*
 * @interface DataSource
 *    Data source are the mechanism to get data from oustide the processing
 *    fabric into the processing fabric.  There will normaly be one or more
 *    Processing Elements that implement GetNextWorkItem as getting
 *    data from a concrete data source and and then using an associated
 *    DataItemConverter to turn that data into a message.
 */
class DataSource
{
public:
    DataSource() {}
    virtual ~DataSource() {}
    
    // All data sources must implement:
    
    virtual void connect()  = 0;
    virtual void disconnect() = 0;
    std::pair<std::size_t, void*> read() = 0;
};

#endif