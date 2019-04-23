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

/** @file:  CZMQDealerTransport.cpp
 *  @brief: Implement the CZMQDealerTransport class.
 *
 */
#include "CZMQDealerTransport.h"
#include "CZMQClientTransport.h"
#include <zmq.hpp>
#include <stdexcept>

/**
 * constructor
 *   - Create the transport.
 *   - Set the id to not set.
 *
 *  @param pUri - URI we're connecting to (the Router's uri).
 */
CZMQDealerTransport::CZMQDealerTransport(const char* pUri) :
    m_pTransport(nullptr), m_idSet(false)
{
    m_pTransport = new CZMQClientTransport(pUri, ZMQ_DEALER);
}
/**
 * constructor
 *    In this case we already know the ID we'll use.
 *
 *  @param pUri - the URI of the router.
 *  @param id   - our client id.
 */
CZMQDealerTransport::CZMQDealerTransport(const char* pUri, uint64_t id) :
    CZMQDealerTransport(pUri)
{
    setId(id);        
}
/**
 * destructor
 *    Destroys our transport.
 */
CZMQDealerTransport::~CZMQDealerTransport()
{
    delete m_pTransport;
    m_pTransport = nullptr;
}

/**
 * recv
 *    Receive data from the peer.  The Router will send a multipart
 *    ZMQ message consisting of our id, a null delmeter and the
 *    data payload.  The id will get stripped off by the dealer socket.
 *    We must strip off the delimeter.
 *
 *  @param ppData - pointer to where to put the received data pointer.
 *  @param size   - referernce to where to put the size.
 *  @note size will be zero and ppData undefined on an end message.
 *  @throw std::logic_error - if idSet is not true. 
 */
void
CZMQDealerTransport::recv(void** ppData, size_t& size)
{
    if (!m_idSet) {
        throw std::logic_error(
            "CZMQDealerTransport - recv attempted prior to setting client id"
        );
    }
    requestData();
    
    size = 0;             // In case this is an end:
    if(stripDelimeter()) {
        m_pTransport->recv(ppData, size);
    }
}
/**
 * send
 *    @param parts - message parts the user is trying to send.
 *    @param numParts - number of message parts.
 *    @throw std::logic_error - because this is a recv only transport.
 */
void
CZMQDealerTransport::send(iovec* parts, size_t numParts)
{
    throw std::logic_error(
        "CZMQDealerTransport attempted send.  This transport is recv only."    
    );
}
/**
 * setId
 *    Set the client's id.  This must be done prior to the first
 *    recv.
 * @param id - the client id to use.
 */
void
CZMQDealerTransport::setId(uint64_t id)
{
    zmq::socket_t* pSock = *m_pTransport;
    
    pSock->setsockopt(ZMQ_IDENTITY, &id, sizeof(uint64_t));
    
    m_idSet = true;
}
/////////////////////////////////////////////////////////////////////
//   Private utilities.

/**
 *  requestData
 *     Does a pull for data from the router peer. The pull consists
 *     of an empty message.  The Dealer socket we're using will
 *     prepend our id to that message part.
 */
void
CZMQDealerTransport::requestData()
{
    m_pTransport->send(nullptr, 0);           // should send null frame.
}
/**
 * stripDelimeter.
 *   Data messages send back by the Router peer consist of an id
 *   followed by a delimeter (empty frame) followed by either data
 *   or nothing if this is an end data message.
 *   the id is stripped off by the Dealer socket we're using.
 *   This method strips off the delimeter and determines if more data
 *   follows.
 *
 *   @return bool - true if there's more data false if this was and end.
 */
bool
CZMQDealerTransport::stripDelimeter()
{
    zmq::socket_t* pSock = *m_pTransport;
    zmq::message_t delim;
    pSock->recv(&delim, 0);
    
    int64_t more(0);
    size_t s(sizeof(more));
    
    pSock->getsockopt(ZMQ_RCVMORE, &more, &s);
    
    return more != 0;
}
