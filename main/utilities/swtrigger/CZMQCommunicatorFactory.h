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

/** @file:  CZMQCommunicatorFactory.h
 *  @brief: Specialization of a CCommunicator Factory for ZMQ transports.
 */
#ifndef CZMQCOMMUNICATORFACTORY_H
#define CZMQCOMMUNICATORFACTORY_H

#include "CCommunicatorFactory.h"
#include <string>
#include <map>
/**
 * @class CZMQCommunicatorFactory
 *
 *   This class is a communicator factory that generates transports
 *   for ZMQ
 *   - Fanout is ROUTER/DEALER
 *   - Fanin  is PUSH/PULL - Puller the server
 *   - Pipeline is also PUSH/PULL - Pusher the server.
 *
 * The factory expects a configuration file that defines the correspondences
 * between endpointIds and service URIs.
 * The form of the file is a simple text file:
 *    id uri
 * The configuration file is the UNION of files found at:
 * -  $HOME/.zmqservices
 * -  ./zmqservices
 * -  Wherever the Environment variable ZMQ_SERVICES points.
 *
 *   Again, this all files (that can be found) are read in in the order given
 *   above.  Duplicate ids result in an override.  E.g. suppose for testing,
 *   ~/.zmqservices has:
 *
 *   1 inproc://test
 *
 *  But ./zmqservices has
 *
 *    1 tcp://workerbee1:1234
 *
 *   endpointId 1 will be tcp://workerbee1:1234.
 *
 *  File processing is pretty stupid.  No checks are made for validity or
 *  conflict.
 */
class CZMQCommunicatorFactory : public CCommunicatorFactory
{
private:
    typedef std::map<int, std::string> EndpointTable;
private:
    EndpointTable m_endpoints;
public:
    CZMQCommunicatorFactory();
    
    virtual CTransport* createFanoutTransport(int endpointId);
    virtual CTransport* createFanoutClient(int endpointId, int clientId);
    virtual CTransport* createFanInSource(int endpointId) ;
    virtual CTransport* createFanInSink(int endpointId);
    virtual CTransport* createOneToOneSource(int endpointId);
    virtual CTransport* createOneToOneSink(int endpointid);
    
    std::string getUri(int endpointId);
private:
    void readEndpointFiles();
    void readEndpointFile(const char* filename);
    
};

#endif