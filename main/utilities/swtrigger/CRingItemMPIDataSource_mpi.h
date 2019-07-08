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

/** @file:  CRingItemMPIDataSource_mpi.h
 *  @brief: Ring block data source using MPI transport.
 */
#ifndef CRINGITEMMPIDATASOURCE_MPI_H
#define CRINGITEMMPIDATASOURCE_MPI_H

#include "CRingItemBlockSourceElement.h"


class CRingItemMPIDataSource : public CRingItemBlockSourceElement
{
public:
    CRingItemMPIDataSource(const char* ringUri,  size_t chunkSize=1);
    virtual ~CRingItemMPIDataSource();
};

#endif