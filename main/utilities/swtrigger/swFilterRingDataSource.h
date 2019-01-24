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

/** @file:  swFilteRingDataSource.h
 *  @brief: Ring item data source for sw trigger/parallel processing framework.
 *  @note swFilterRingDataSource is used to clearly distinguish this from
 *        CRingDataSource -- which this data source uses.
 */
#ifndef SWFILTERRINGDATASOURCE_H
#define SWFILTERRINGDATASOURCE_H
#include "DataSource.h"
class CDataSource;

/**
 * @class swFilterRingDataSource
 *    This is a data source intended for use with the software
 *    online framework for processing data in threaded parallel (and trigger
 *    processing).
 *    It fetches ring items one at a time from a CRingDataSource object
 *    it contains.  Note that the class hierarchy for CRingData source
 *    supports getting objects both from live ring buffers and from file,
 *    however to get live data, the host must be an active NSCLDAQ host
 *    (specifically the port manager and rinmaster servers must be running.
 */
class swFilterRingDataSource :  public DataSource
{
private:
    CDataSource&   m_ActualSource;
public:
    swFilterRingDataSource(CDataSource& src);
    virtual ~swFilterRingDataSource();
    
    virtual void connect();
    virtual void disconnect();
    virtual std::pair<std::size_t, void*> read();
};

#endif