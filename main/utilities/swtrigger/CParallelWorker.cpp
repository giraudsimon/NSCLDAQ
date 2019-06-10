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

/** @file:  CParallelWorker.cpp
 *  @brief: Implement the parallel worker  ABC. concretes must implement process
 */

#include "CParallelWorker.h"
#include "CReceiver.h"
#include "CFanoutClientTransport.h"
#include "CSender.h"

#include <stdlib.h>

/**
 * constructor
 *    Construction when the fanin transport has already been given
 *    our client id.
 *
 * @param fanin - transport used to get data from the fanout.
 * @param sink  - CSender process should use to send processed data on.
 */

CParallelWorker::CParallelWorker(CFanoutClientTransport& fanin, CSender& sink)
    : m_pReceiverTransport(&fanin), m_pDataSource(nullptr), m_pDataSink(&sink)
{
    m_pDataSource = new CReceiver(fanin);        
}
/**
 * constructor
 *   A constructor where the id is supplied to the transport now.
 * @param fanin - transport used to get data from the fanout.
 * @param sink  - CSender process should use to send processed data on.
 * @param clientId - the id to program into the fanin transport.
 */
CParallelWorker::CParallelWorker(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId
) : CParallelWorker(fanin, sink)
{
    m_pReceiverTransport->setId(clientId);
}
/**
 * destructor
 *    Destroys the sender and receiver.   Destroying the receiver also
 *    destroys the transport.
 */
CParallelWorker::~CParallelWorker()
{
    delete m_pDataSink;
    delete m_pDataSource;
}

/**
 * operator()
 *    Flow of control - get data from the source until there's nothing left
 *    The worker is sent the end record.  It is supposed to relay that to
 *    the transport after doing any cleanup it may need.
 */
void
CParallelWorker::operator()()
{
    void* pData;
    size_t nBytes;
    do {
        m_pDataSource->getMessage(&pData, nBytes);
        process(pData, nBytes);
        free(pData);
    } while (nBytes > 0);
}
///////////////////////////////////////////////////////////////////////
// Utilties for derived classes.

/**
 * getSink
 *   @return CSender* - The data sink.  This allows process to get
 *                      the data sink so that it can output data.
 */
CSender*
CParallelWorker::getSink()
{
    return m_pDataSink;
}