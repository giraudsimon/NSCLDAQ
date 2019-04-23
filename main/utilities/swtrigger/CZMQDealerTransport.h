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

/** @file:  CZMQDealerTransport.h
 *  @brief: Provide a receiver transport for CZMQRouterTransport
 */
#ifndef CZMQDEALERTRANSPORT_H
#define CZMQDEALERTRANSPORT_H

#include "CFanoutClientTransport.h"
class CZMQClientTransport;

/**
 * @class CZMQDealerTransport
 *
 *  This transport is intended to receive data from CZMQRouterTransport.
 *  It is a CFanoutClientTransport that encpapsulates a ZMQClientTransport
 *  object configured to be a ZMQ_DEALER.
 *
 * @note that ZMQ requires that setId be called prior to requesting any data
 *       an exception is thrown if recv is called prior to setId.
 * @note that since this transport is unidirectional, send will always throw
 *       an exception.
 */
class CZMQDealerTransport : public CFanoutClientTransport
{
private:
    CZMQClientTransport*  m_pTransport;
    bool                  m_idSet;
public:
    CZMQDealerTransport(const char* pUri);
    CZMQDealerTransport(const char* pUri, uint64_t id);
    virtual ~CZMQDealerTransport();
    
    void recv(void** ppData, size_t& size);
    void send(iovec* parts, size_t numParts);
    void setId(uint64_t id);
private:
    void requestData();
    bool stripDelimeter();
};


#endif