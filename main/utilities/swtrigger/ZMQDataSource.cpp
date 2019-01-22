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

/** @file:  ZMQDataSource.cpp
 *  @brief: base class for ZMQ data sources
 */
#include "ZMQDataSource.h"
#include "ZMQContext.h"
#include "MessageTypes.h"
#include <zmq.hpp>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <string.h>
#include "zhelpers.hpp"

/**
 * constructor.
 * @param name - thread name
 * @param service - URI of service we're creating.
 */
ZMQDataSource::ZMQDataSource(const char* name, const char* service) :
    Thread(std::string(name)),
    m_serviceURI(service), m_pService(nullptr), m_barrierInProgress(false)
{
    // Create the socket and set it to not linger on disconnection
    
    zmq::context_t& ctx(ZMQContext::getContext());
    m_pService = new zmq::socket_t(ctx, ZMQ_ROUTER);
    int linger(0);
    m_pService->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
    
    // Advertise ourselves as a service:
    
    m_pService->bind(service);
}
/**
 * destructor
 *   If the socket is still in existence, close and delete it:
 *
 */
ZMQDataSource::~ZMQDataSource()
{
    delete m_pService;              // delete of nullptr is no-op.
}
/**
 * run
 *    The service thread.    Process data requests, getting work items
 *    as needed to satisfy them.
 */
void
ZMQDataSource::run()
{
    // This loop assumes that the first thing clients do is register and
    // that they hold that registration until they've processed all the
    // data they can:
    do {
        // request.first is the request type and request.second is the
        // client identifier.
        
        std::pair<std::string, int> request = getClientRequest();
        int code = request.second;
        std::string& id(request.first);
        if (code == REGISTER) {
            addClient(id);
        } else if (code == DATA) {
            respondToDataRequest(id);   // Handles barriers too.
        } else if (code == UNREGISTER) {
            removeClient(id);
        } else {
            throw std::invalid_argument("ZMQDataSource::run - invalid client request code");
        }
        
    } while (!m_clients.empty());       // All clients unregistered.
    
    delete m_pService;                  // Shutdown the service.
    m_pService = nullptr;               // so destructor does not die.
}
/**
 * sendWorkItem
 *    Sends a work item to a client.  The work item is characterized
 *    by the identity.  and a (possibly empty) list of message parts to send to the
 *    client.   Since we're a ROUTER, we need to send the identity and
 *    a delimeter.  If there are message chunks we then send
 *    those as well.
 *
 *  @param identity  - Who gets the work item.
 *  @param msg       - List of additional message parts.
 *                     note that each message part is assumed to have been
 *                     'new'd.  The ownership of the contents of those
 *                     messages, however is passed on to the receiving client.
 */
void
ZMQDataSource::sendWorkItem(
    std::string identity, std::list<zmq::message_t*>& workitem)
{
    s_sendmore(*m_pService, identity);
    if (workitem.empty()) {
        s_send(*m_pService, "");           // The delimeter ends it.
    } else {
        s_sendmore(*m_pService, "");       // there's more than the delimiter.
        sendMessageList(workitem);         // Send the rest of the message.
    }
}
/**
 * sendBarrierItem
 *    This is just sendWorkItem but with book keeping to know when
 *    we've sent to all membrers of the barrier.
 *  @param identity - where the barrier message goes. The body of the message
 *                    is in m_barrierMessage.   The caller must ensure that
 *                    this identity is not in the set m_barriersSent.
 */
void
ZMQDataSource::sendBarrierItem(std::string identity)
{
    sendWorkItem(identity, m_barrierMessage);
    m_barriersSent.insert(identity);
}
/**
 * getClientRequest
 *    Gets the next client request.  Client request are messages that
 *    consist of a string identity, an empty string delimiter and
 *    an integer that is an operation code.
 *  @return std::pair<std::string, int> first element is the requesting
 *                  identifier, second is the request code.
 */
std::pair<std::string, int>
ZMQDataSource::getClientRequest()
{
    std::pair<std::string, int> result;
    result.first = s_recv(*m_pService);
    s_recv(*m_pService);              // Throw the delimiter away.
    zmq::message_t body;
    m_pService->recv(&body);
    if (body.size() != sizeof(uint32_t)) {
        throw std::invalid_argument(
            "ZMQDataSource Invalid payload size in client request"
        );
    }
    uint32_t* p = static_cast<uint32_t*>(body.data());
    result.second = *p;
    
    return result;
}
//////////////////////////////////////////////////////////////////////////
// Client API,  these methods are called from client threads rather
// than the sender thread.  They send and receive messages to and from
// the sender cooking them as needed to help the client (worker) understand
// them.
//

/**
 * connect
 *   @return zmq::socket_t*  - a dynamically created socket connected
 *             as a DEALER to the data sender's ROUTER.
 *   @note Ownership of the socket and associated storage is transferred to the
 *         caller-- a fancy way of saying the caller has to eventually delete it.
 */
zmq::socket_t*
ZMQDataSource::connect()
{
    zmq::socket_t* result =
        new zmq::socket_t(ZMQContext::getContext(), ZMQ_DEALER);
    
    s_set_id(*result);
    // Set the socket to not linger on remote disconnect:
    
    int linger = 0;
    result->setsockopt(ZMQ_LINGER, &linger, sizeof(int));
    
    // Connect to the server:
    
    result->connect(m_serviceURI.c_str());
    
    return result;
}
/**
 * registerClient
 *    Sends a message to the sender registering the sender as a client.
 *  @param pSock - pointer to a socket created by connect above.
 *  @param identity - the calling thread's unique identity.
 */
void
ZMQDataSource::registerClient(zmq::socket_t* pSock)
{
    sendRequest(pSock, REGISTER);
}
/**
 * unregisterClient
 *    Sends a message to the sender unregistering the sender as a client.
 *
 *   @param pSock - the socket connected to the sender (return from connect).
 *   @param identity - unique thread identity.
 *   @note The socket will be closed and delete'd.  It should not be referenced
 *         after this method.
 */
void
ZMQDataSource::unregisterClient(zmq::socket_t* pSock)
{
    sendRequest(pSock, UNREGISTER);
    delete pSock;
}
/**
 * getWorkItem
 *    Ineract with the sender to get the next work item from the sender.
 *
 *  @param pSock - socket gotten from connect.
 *  @param identity - our identity.
 *  @return std::pair<int, std::list<zmq::message_t*> > -
 *      A pair consisting of the message type code followed by a possibily
 *      empty list of message parts.  See below.
 *  @note Once you're done with the work item,
 *      freeWorkItem must be called to release any storage associated
 *      with the work item.  That includes the message_t objects.
 */
std::pair<uint32_t, std::list<zmq::message_t*>>
ZMQDataSource::getWorkItem(zmq::socket_t* pSock)
{
    
    sendRequest(pSock, DATA);
    
    // We are gauranteed to get two parts:
    // -  blank delimeter
    // -  Code indicating what we got (see MessageTypes.h).
    
    std::pair<int, std::list<zmq::message_t*>> result;
    
    s_recv(*pSock);                  // Delimeter.
    zmq::message_t code;
    pSock->recv(&code);
    if (code.size() != sizeof(uint32_t)) {
        throw std::logic_error("ZMQ::DataSource - client message code in invalid size");
    }
    uint32_t* pCode = static_cast<uint32_t*>(code.data());
    result.first = *pCode;
    
    // If there's any additional message segments fill in the list:
    
    int64_t more;
    size_t moreSize = sizeof(more);
    pSock->getsockopt(ZMQ_RCVMORE, &more, &moreSize);
    while(more) {
        zmq::message_t* pPart = new zmq::message_t;
        pSock->recv(pPart);
        result.second.push_back(pPart);
        
        pSock->getsockopt(ZMQ_RCVMORE, &more, &moreSize);
    }
    
    
    return result;
}

/**
 * barrierDone
 *    @return bool - true if all members of m_clients are in m_barriersSent
 *    @note the condition above is not the same as m_clients == m_barriersSent
 *          It's the (m_clients * m_barriers_sent) == m_clients where *
 *          is a set intersection operator.  This is important because
 *          while the barriers are being sent, clients can unregister/exit
 *          removing them from m_clients.  This _will_ happen  on an END_ITEM
 *          barrier, e.g.
 */
bool
ZMQDataSource::barrierDone()
{
    std::set<std::string> intersection;
    std::set_intersection(
        m_clients.begin(), m_clients.end(),
        m_barriersSent.begin(), m_barriersSent.end(),
        std::inserter(intersection, intersection.begin())
    );
    
    return (intersection == m_clients);
}
/**
 * addClient
 *    Adds a new client to the sender.  This executes in the sender's thread
 *    in response to a REGISTER message.
 *
 * @param client -id of the client.
 */
void
ZMQDataSource::addClient(std::string& client)
{
    m_clients.insert(client);
}
/**
 * removeClient
 *    Called in the sender thread whena client unregisters.  Removes
 *    the client from the m_clients set.
 *
 *  @param client - the client to remove.
 */
void
ZMQDataSource::removeClient(std::string& client)
{
    m_clients.erase(client);
}
/**
 * sendMessageList
 *    Send the message list to m_pService.  Prior to this, it's important
 *    that the caller have sent an identity and delimiter.  See, for example,
 *    sendWorkItem above.  We send all message parts as parts of an atomic
 *    message.
 * @param msg - list of message parts to send.
 * @note msg cannot be empty!!!
 * @note msg is passed by value because we're not going to take ownership
 *           of its storage.
 */
void
ZMQDataSource::sendMessageList(std::list<zmq::message_t*> msg)
{
    if (msg.empty()) {
        throw std::invalid_argument("ZMQDataSource - message list empty");
    }

    while (! msg.empty()) {
        zmq::message_t* pMessage = msg.front();
        msg.pop_front();
        m_pService->send(pMessage, msg.empty() ? 0 : ZMQ_SNDMORE);
    }
}
/**
 *  sendRequest
 *    Sends a request message.  The request message consist of the
 *    -  id of the requestor (filled in automatically by the DEALER socket).
 *    -  an empty string delmiter
 *    -  The request code integer.
 *
 * @param pSock - socket used to transport the data.
 * @param the client string.
 */
void
ZMQDataSource::sendRequest(zmq::socket_t* pSock, int reqCode)
{
    uint32_t code = reqCode;
    
    s_sendmore(*pSock, "");                   // Delimiter.
    zmq::message_t msg(code);
    memcpy(msg.data(), &code, sizeof(code));
    pSock->send(msg);                         // Send the code - no more segs.
    
}
/**
 * respondToDataRequest
 *     This method handles the top level of a client's request for data.
 *      We're going to assume the client is going through getWorkItem and therefore
 *      won't have more than one outstanding item.
 * @param client - the client requesting data.
 */
void
ZMQDataSource::respondToDataRequest(std::string& client)
{
    // If there's a barrier in progresss:
    
    if (m_barrierInProgress) {
        // If this client has not yet recieved the barrier it gets it.
        // if it has, it's added to the pending requests list and we won't
        // satisfy the request until barrier processing is complete.
        
        if (m_barriersSent.count(client)) {
            m_pendingRequests.push_back(client);
        } else {
            sendBarrierItem(client);
        }
        // If the barrier is complete, flush the pending requests.
        // Note that might result in another barrier.
        
        if (barrierDone()) {
            m_barrierInProgress = false;
            m_barriersSent.clear();
            freeWorkItem(m_barrierMessage);  // Free the remainder of the msg.
            flushPendingRequests();
        }
    } else {
        // Get the next work item and send it.  If it's a barrier, we need
        // to initiate barrier processing as well:
        
        std::list<zmq::message_t*> workItem;
        getNextItem(workItem);
        if(isBarrier(workItem)) {
            m_barrierInProgress = true;
            m_barrierMessage    = workItem;
            sendBarrierItem(client);
            flushPendingRequests();    // Pending requests get barriers.
            
        } else {
            sendWorkItem(client, workItem);
            freeWorkItem(workItem);
        }
    }
}

/**
 * flushPendingRequests.
 *    Invoked at the end of barrier processing.
 *    for each item in the pending requests queue,
 *    respondToDataRequest is invoked.   Note that this could
 *    result in initiation of another barrier request but,
 *    if we copy and clear the pending request queue first,
 *    respondToDataRequest should handle that just fine.
 */
void
ZMQDataSource::flushPendingRequests()
{
    std::list<std::string> pending = m_pendingRequests;
    m_pendingRequests.clear();
    
    while(!pending.empty()) {
        std::string client = pending.front();
        respondToDataRequest(client);
        pending.pop_front();
    }
}