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

/** @file:  CMPITransport_mpi.h
 *  @brief: Base classes for MPI transports.
 */
#ifndef CMPITRANSPORT_MPI_H
#define CMPITRANSPORT_MPI_H
#include "CTransport.h"
#include <mpi.h>
class CGather;


/**
 * @class CMPITransport
 *    Base class for all? Most? MPI transports.
 *    it can send messages to some 'current rank' or
 *    receive messages from any sender.
 *
 *    Note that prior to performing data transfers on an MPI transport
 *    MPI_Init must have been called.  We can't easly supply this here
 *    because it needs the command line parameters.
 *
 *    Since some transports may exchage data at setup time we strongly
 *    repeat _strongly_ urge MPI_Init be called as about the first
 *    statement prior to setting up the computation.
 */
class CMPITransport : public CTransport
{
private:
    int      m_nCurrentReceiver;
    int      m_nLastTagReceived;
    int      m_nLastRankReceived;
    CGather* m_pSendBuffer;
    MPI_Comm m_communicator;
public:
    CMPITransport();
    CMPITransport(int currentReceiver);
    virtual ~CMPITransport();
    
    virtual  void    recv(void** ppData, size_t& size);
    virtual  void    send(iovec* parts, size_t numParts);
    virtual  void    end();
    
    // MPI specific stuff:
    
    int setReceiver(int rank);
    int getLastReceivedTag() const;
    int getLastReceivedRank() const;
    MPI_Comm setCommunicator(MPI_Comm newComm);
    
private:
    void  gather(const iovec* parts, size_t numParts);           // creates if needed
protected:
    static const int endTag  = 0;    // Tag signifying end of data
    static const int dataTag =  1;   // Tag signifying data

    
};

#endif