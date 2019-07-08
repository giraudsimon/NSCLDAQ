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

/** @file:  CRingItemMPIDataSource_mpi.cpp
 *  @brief:  Implement the CRingItemMPIDataSource class.
 */
#include "CRingItemMPIDataSource_mpi.h"
#include "CMPIFanoutTransport_mpi.h"

/**
 * constructor
 *    @param ringUri - ring buffer data source uri.
 *    @param chunkSize - number of ring items in chunks sent.
 */
CRingItemMPIDataSource::CRingItemMPIDataSource(const char* ringUri, size_t chunkSize) :
    CRingItemBlockSourceElement(ringUri, *(new CMPIFanoutTransport()), chunkSize)
{}
                          