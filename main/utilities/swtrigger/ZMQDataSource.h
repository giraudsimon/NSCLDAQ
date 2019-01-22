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

/** @file:  ZMQDataSource.h
 *  @brief: Provides an abstract class that can be derived for ZMQ data sources.
 */
#ifndef ZMQDATASOURCE_H
#define ZMQDATASOURCE_H
#include <zmq.hpp>

#include <Thread.h>
#include <set>
#include <string>
#include <list>
#include <stdint.h>

class Worker;

/**
 * @class ZMQDataSource
 *    This is an abstract class using the strategy pattern that implements
 *    a ZMQ data source.   The class uses a strategy pattern to ensure
 *    that it can be generalized to other applications.  The
 *    class assumes the clients are using a simple application protocol
 *    on a broker/dealer socket we hold to request and receive work items.
 *
 *    Work items consist of a message type and a sequence of message_t's.
 *    The implemented client code filters out things like the delimeters
 *    from the messages sent around.m
 */
class ZMQDataSource : public Thread
{

private:
    std::string            m_serviceURI;
    zmq::socket_t*         m_pService;              // Router/dealer.
    std::set <std::string> m_clients;              // Known set of clients.
    
    // Barrier handling is complex.  What we do for now is recognize
    // that it's pretty rare and serialize at a barrier.  We need
    // the data below to keep track of barrier handling.
    
    bool                   m_barrierInProgress;
    std::set<std::string>  m_barriersSent;       // Who's gotten the barrier.
    std::list<std::string> m_pendingRequests;    // Got the barrier wants data while still processing the barrier.
    std::list<zmq::message_t*> m_barrierMessage;  // payload barrier message segs.

    // Request types.   These are the request type uint32_t's sent by clients:
    // Since the client messages are formed by our client api, 
protected:  
    static const int REGISTER = 1;
    static const int DATA = 2;
    static const int UNREGISTER = 3;

public:
    ZMQDataSource(const char* name, const char* service);
    ~ZMQDataSource();
    
    virtual void run();
    
    // API that must be implemented by the concrete class.
protected:    
    virtual void getNextItem(std::list<zmq::message_t*>& workItem) = 0;
    virtual bool isBarrier(std::list<zmq::message_t*>&  workItem) = 0;
    virtual std::pair<std::string, int>  getClientRequest();    
    virtual void freeWorkItem(std::list<zmq::message_t*>& msg) = 0;    
    
    // Implementations that can be overridden but should be fine:
    
    virtual void sendWorkItem(
        std::string identity, std::list<zmq::message_t*>& msg);
    virtual void sendBarrierItem(std::string identity);
    
    // Client api
    
public:
    zmq::socket_t* connect();
    virtual void registerClient(zmq::socket_t* pSock);
    virtual void unregisterClient(zmq::socket_t* pSock);
    virtual std::pair<uint32_t, std::list<zmq::message_t*> >
        getWorkItem(zmq::socket_t* pSock);
    
    
    
    
    // Utility private methods.
private:
    bool barrierDone();
    void addClient(std::string& client);
    void removeClient(std::string& client);
    void sendMessageList(std::list<zmq::message_t*> msg);
    void sendRequest(zmq::socket_t* pSock,  int reqCode);
    void respondToDataRequest(std::string& client);
    void flushPendingRequests();
};


#endif