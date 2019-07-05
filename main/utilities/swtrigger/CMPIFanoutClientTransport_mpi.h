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

/** @file:  CMPIFanoutClientTransport_mpi.h
 *  @brief: Transport that receives data from CMPIFanoutTransport.
 */
#ifndef CMPIFANOUTCLIENTTRANSPORT_MPI_H
#define CMPIFANOUTCLIENTTRANSPORT_MPI_H

#include "CMPITransport_mpi.h"
#include "CFanoutClientTransport.h"

#include <sys/uio.h>
#include <mpi.h>
/**
 * @class CMPIFanoutClientTransport
 *     Client for an MPI transport.
 */
class CMPIFanoutClientTransport
    : public CMPITransport, public CFanoutClientTransport
{
    int m_distributor;
public:
    CMPIFanoutClientTransport(int distributor);
    CMPIFanoutClientTransport(
        MPI_Comm communicator, int distributor
    );
    virtual ~CMPIFanoutClientTransport();
    
    virtual void recv(void** ppData, size_t& size);
    virtual void send(iovec* parts,  size_t numParts);
    virtual void setId(uint64_t id) {} // Id is rank.
private:
    void requestData();
};


#endif