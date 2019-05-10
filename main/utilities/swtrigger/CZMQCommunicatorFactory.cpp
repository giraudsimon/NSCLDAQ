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

/** @file:  CZMQCommunicatorFactory.cpp
 *  @brief: Implementation of the ZMQ communicator factory.
 */
#include "CZMQCommunicatorFactory.h"
#include "CZMQServerTransport.h"
#include "CZMQClientTransport.h"
#include "CZMQRouterTransport.h"
#include "CZMQDealerTransport.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdexcept>
#include <zmq.hpp>



/**
 * constructor
 *    Read in the endpoint table.  If there are no
 *    endpoints a warning is written to stderr  There are cases this
 *    is allowed but they're rare, so it's not fatal but it does warn.
 */
CZMQCommunicatorFactory()
{
    readEndpointFiles();
    if (m_endpoints.empty()) {
        std::cerr << "Warning ZMC Communicator factory created with empy endpoints table\n";
    }
}

/**
 * createFanoutTransport
 *     In ZMQ this is naturally implemented with a CZMQRouterTransport.
 *  @param endpointId  selects the endpoint from the m_endpoints table.
 *  @return CTransport* - pointer to the dynamically allocated transport
 *  @throw std::invalid_argument - the endpoint has no match in the table.
 */
CTransport*
CZMQCommunicatorFactory::createFanoutTranpsport(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQRouterTransport(URI.c_str());
}
/**
 * createFanoutClient
 *    In ZMQ, this is naturally implemented as a dealer.
 *
 *  @param endpointId - selects the end point from the m_endpoints table.
 *  @param clientId   - The client's id used to route data.
 *  @return CTransport* - pointer to the dynamically allocated transport
 *  @throw std::invalid_argument - the endpoint has no match in the table.
 * 
 */
CTransport*
CZMQCommunicatorFactory::createFanoutClient(int endpointId, int clientId)
{
    std::string URI = getUri(endpointId);
    return new CZMQDealerTransport(URI.c_str(), clientId);
}
/**
 * createFaninSource
 *     Creates a client zmq transport that pushes. pairs with a FaninSink
 *
 *  @param endpointId
 *  @return CCTransport*
 */
CTransport*
CZMQCommunicatorFactory::createFanInSource(int endpointId)
{
    std::string URI= getUri(endpointId);
    return new CZMQClientTransport(URI.c_str(), ZMQ_PUSH);
}
/**
 * createFanInSink
 *    Returns a transport that is the end in end of several PUSHers
 *    (a PUll server).
 * @param endpointId
 * @return CTransport*
 */
CTransport*
CZMQCommunicatorFactory::createFanInSink(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQServerTransport(URI.c_str(), ZMQ_PULL);
}
/**
 * createOneToOneSource
 *    Creates a PUSH that's a server.   The idea is that this will be
 *    a one-to-one pair of sockets.  I'd use ZMQ_PAIR except that the
 *    following text in the docs puts me off:
 *    " ZMQ_PAIR sockets are considered experimental and may have other
 *      missing or broken aspects."
 *
 *    Using PUSH/PULL has the risk that serveral sinks could connect to us.
 *    Guess we have to trust the user :-(
 *
 *  @param endpointId  - Id of the endpoint,.
 *  @return CTransport* pointer to the created  transport.
 */
CTransport*
CZMQCommunicatorFactory::createOneToOneSource(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQServerTransport(URI.c_str(), ZMQ_PUSH);
}
/**
 * createOneToOneSinkn
 *     Creates a pull that's a client.  See CreateOneToOneSource.
 *
 *  @param endpointId
 *  @return CTransport
 */
CTransport*
CZMQCommunicatorFactory::createOnToOneSink(int endpointId)
{
    std::string URI = getUri(endpointId);
    return new CZMQClientTransport(URI.c_str(), ZMQ_PULL);
}

///////////////////////////////////////////////////////////////////////
//  Private utilities.