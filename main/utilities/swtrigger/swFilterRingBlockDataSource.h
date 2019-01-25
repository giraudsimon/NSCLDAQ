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

/** @file:  swFilterRingBlockDataSource.h
 *  @brief: Data source that uses a CRingBlockReader to get data.
 */
#ifndef SWFILTERRINGBLOCKDATASOURCE_H
#define SWFILTERRINGBLOCKDATASOURCE_H
#include "DataSource.h"
#include <stddef.h>

class CRingBlockReader;

/**
 * @class swFilterRingBlockDataSource
 *    encapsulates a CRingBlockReader and provides the
 *    DataSource interface to read data from it.
 *
 *    read will return a size_t that is the size of the data descriptor
 *    and a pointer to the data descriptor.
 *
 *    This can be paired with a RingItemBlockConverter to
 *    provide messages for the framework.
 *
 */
class swFilterRingBlockDataSource : public DataSource
{
private:
    CRingBlockReader&   m_Reader;
    size_t              m_nReadSize;
public:
    swFilterRingBlockDataSource(CRingBlockReader& reader, size_t readSize);
    virtual ~swFilterRingBlockDataSource();
    
    virtual void connect();
    virtual void disconnect();
    virtual std::pair<std::size_t, void*> read();
};

#endif