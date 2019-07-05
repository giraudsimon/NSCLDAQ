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
 *  @brief: Implement the CMPIFanoutClientTransport.
 */

#include "CMPIFanoutClientTransport_mpi.h"
#include "CMPIFanoutTransport_mpi.h" // for the req data tag.
#include <stdexcept>
#include <stdlib.h>
#include <mpi.h>
/**
 * constructor - with distributor
 *   @param distributor - rank of the sender process.
 */
CMPIFanoutClientTransport::CMPIFanoutClientTransport(int distributor) :
    CMPITransport(distributor), m_distributor(distributor)
{}

/**
 * Constructor with distributor and communicator.
 *
 * @param communicator MPI communicator used to request/get data.
 * @param distributor MPI Rank of sender.
 */
CMPIFanoutClientTransport::CMPIFanoutClientTransport(MPI_Comm communicator, int distributor) :
    CMPIFanoutClientTransport(distributor)
{
    setCommunicator(communicator);
}
/**
 * destructor - not assumed to be called until an end received.
 */
CMPIFanoutClientTransport::~CMPIFanoutClientTransport()
{}

/**
 * recv
 *    Receive data from the sender.
 *
 *  @param ppData -where pointer to the data buffer will go.
 *  @param size   - where number of bytres read goes.
 */
void
CMPIFanoutClientTransport::recv(void** ppData, size_t& size)
{
    requestData();
    CMPITransport::recv(ppData, size);
}
/**
 * send
 *    Throws a logic error as this transport only receives.
 */
void
CMPIFanoutClientTransport::send(iovec* vec, size_t n)
{
    throw std::logic_error("CMPIFanoutClientTransport can only recv");
}
///////////////////////////////////////////////////////////////
// Private methods.

/**
 * requestData
 *    Request data of the sender.
 *    - Sends a null data message to the distributor with
 *    - tag of CMPIFanoutTransport::dataRequestTag
 */
void
CMPIFanoutClientTransport::requestData()
{
    MPI_Comm c = setCommunicator(MPI_COMM_WORLD);
    setCommunicator(c);
    
    void* pData(nullptr);
    MPI_Send(
        pData, 0, MPI_CHAR, m_distributor,
        CMPIFanoutTransport::dataRequestTag, c
    );
}