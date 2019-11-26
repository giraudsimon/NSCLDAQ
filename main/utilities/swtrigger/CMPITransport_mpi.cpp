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

/** @file:  CMPITransport_mpi.cpp
 *  @brief: Implements. the base MPI transport class.
 */

#include "CMPITransport_mpi.h"
#include "CGather.h"

#include <mpi.h>

#include <stdlib.h>
#include <stdexcept>
#include <sys/uio.h>


/**
 * constructor
 *    Just initializes data.  Note that the gather is created
 *    when it's first needed.  Thus a send only transport
 *    won't burden itself with a gather object.
 *
 */
CMPITransport::CMPITransport() :
    m_nCurrentReceiver(-1), m_nLastTagReceived(-1),
    m_nLastRankReceived(-1), m_pSendBuffer(nullptr),
    m_communicator(MPI_COMM_WORLD)
{
    
}
/**
 * constructor
 *    constructs and sets the current receiver rank.
 *    This is useful for point to point transports.
 *
 * @param rank - currentReceiver gets set to this.
 */
CMPITransport::CMPITransport(int rank) :
    CMPITransport()
{
    m_nCurrentReceiver = rank;        
}

/**
 * destructor
 *  delete the gather.
 *  Note if it wasn't created delete on a nullptr is a noop.
 */
CMPITransport::~CMPITransport()
{
    delete m_pSendBuffer;
}

/**
 * recv
 *    Receive a message from the current communicator.
 *    - Use MPI_Probe to figure out how big the message is
 *      Who sent it and what its tag was.
 *    - All our messages are assumed to be of type MPI_CHAR
 *    - We then allocate the buffer and receive the message.
 *    - IF the tag was endTag, the user will get a zero length
 *     back for the size.
 *
 *  @parma ppData - where we store the pointer to the data.
 *  @param size   - reference to where we put the data size.
 */
void
CMPITransport::recv(void** pData, size_t& size)
{
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, m_communicator, &status);
    
    m_nLastTagReceived  = status.MPI_TAG;
    m_nLastRankReceived = status.MPI_SOURCE;
    
    int nBytes;
    MPI_Get_count(&status, MPI_CHAR, &nBytes);
    
    if (nBytes) {          // Dont't do zero length mallocs:
        *pData = malloc(nBytes);
        MPI_Recv(
        *pData, nBytes, MPI_CHAR, m_nLastRankReceived, m_nLastTagReceived,
        m_communicator, MPI_STATUS_IGNORE
      );
      size = nBytes;
    } else {
        uint32_t dummy;
        MPI_Recv(
            &dummy, sizeof(dummy), MPI_CHAR, m_nLastRankReceived, m_nLastTagReceived, 
            m_communicator, MPI_STATUS_IGNORE
        );                                      // stll need to recv with non null buffer.
        *pData = nullptr;
        size = 0;
    }
    
    
    
    
    
}
/**
 * send
 *    Sends data to the current receiver.
 *
 *  @param parts - array of part descriptors.  See writev(2).
 *  @param numParts - number of parts.
 *
 *   @note, if there's not yet a CGather object, one gets created.
 */
void
CMPITransport::send(iovec* parts, size_t numParts)
{
    if (m_nCurrentReceiver == -1) {
        throw std::logic_error("MPI Transport send - a receiver must be set");
    }
    gather(parts, numParts);          // Creates the gather if needed.
    
    
    MPI_Send(
        *m_pSendBuffer, m_pSendBuffer->size(), MPI_CHAR,
        m_nCurrentReceiver, dataTag, m_communicator
    );
    
}
/**
 * end
 *    Send an end indicator.  This is an empty message
 *    with the endTag tag.
 *
 */
void
CMPITransport::end()
{
    if (m_nCurrentReceiver == -1) {
        throw std::logic_error("MPI Transport end - no receiver set");
    }
    void* dummy(nullptr);
    MPI_Send(
        dummy, 0, MPI_CHAR, m_nCurrentReceiver, endTag,
        m_communicator
    );
}
//////////////////////////////////////////////////////////////////
// MPI specific publi methods.

/**
 * setReceiver
 *   Sets the rank of the receiver of send/end operations
 *   from this transport.
 *
 *   @param rank - new receiver rank.  Note that this must be
 *                 the rank withink the communicator used to
 *                 send/receive the data (see setCommunicator).
 *   @return int - the prior rank -1 if there was none.
 */
int
CMPITransport::setReceiver(int rank)
{
    int result = m_nCurrentReceiver;
    m_nCurrentReceiver = rank;
    return result;
}

/**
 * getLastReceivedTag
 *    @return int - tag of last received message.  This is
 *    @retval -1 if no message has been received yet.
 */
int
CMPITransport::getLastReceivedTag() const
{
    return m_nLastTagReceived;
}
/**
 * getLastReceivedRank
 * 
 * @return int - rank of most recently received message.
 * @retval -1 if no message has been received yet.
 */
int
CMPITransport::getLastReceivedRank() const
{
    return m_nLastRankReceived;
}
/**
 * setCommunicator
 *    Sets a new communicator to use when sending/receiving
 *    messages.
 *
 * @param newComm -new communicator.
 * @return MPI_Comm - old communicatorl
 * @note This class constructs with the default communicator
 *        MPI_COMM_WORLD
 */
MPI_Comm
CMPITransport::setCommunicator(MPI_Comm newComm)
{
    MPI_Comm result = m_communicator;
    m_communicator = newComm;
    return result;
}
/////////////////////////////////////////////////////////////
// Private methods

/**
 * gather
 *    Gather send data into the m_pSendBuffer.  If that object
 *    has not yet been created it will be by this
 *    method.
 *
 * parts - array of messgae parts.
 * numParts - number of message parts.
 */
void
CMPITransport::gather(const iovec* parts, size_t numParts)
{
    if (! m_pSendBuffer) {
        m_pSendBuffer = new CGather(parts, numParts);
    } else  {
        m_pSendBuffer->gather(parts, numParts);
    }
}