/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CStateMonitor.h
# @brief  Header that defines the C++ state monitor class
# @author <fox@nscl.msu.edu>
*/

#ifndef CSTATEMONITOR_H
#define CSTATEMONITOR_H

#include <zmq.hpp>
#include <vector>
#include <string>
#include <map>


/*
* @class CZMQPollEventLoop
*     Polling event loop which supports dispatching callbacks when
*     events happen on file descriptors or zmq::sockets.
*/
class CZMQEventLoop {
    
    //Private data types:
private:
    
    struct SocketEvent {
        zmq::socket_t*         s_socket;
        int                    s_events;
    };
    struct FdEvent {
        int                    s_fd;
        int                    s_events;
    };
    // Public data types:
public:

    typedef void (*Callback)(CZMQEventLoop*, zmq::pollitem_t*, void* param);
    typedef bool (*IdleCallback)(CZMQEventLoop*);
    struct SocketRegistration {
        SocketEvent           s_event;
        Callback              s_cb;
        void*                 s_param;
    };
    struct FdRegistration {
        FdEvent               s_event;
        Callback              s_cb;
        void*                 s_param;
    };
    // Private data
    
private:

    std::vector<SocketRegistration>   m_socketRegistrations;
    std::vector<FdRegistration>       m_fdRegistrations;
public:
    CZMQEventLoop();
    
    void Register(zmq::socket_t& sock, int mask, Callback cb, void* param = 0);
    void Register(int fd, int mask, Callback cb, void* param = 0);
    void unregister(zmq::socket_t& sock);
    void unregister(int fd);
    void poll(int timeout);
    void pollForever(int timeout, IdleCallback idler=0);
private:
    void dispatch(zmq::pollitem_t* items, size_t count);
    size_t makePollItems(zmq::pollitem_t** items);
    std::pair<Callback, void*> findCallback(zmq::pollitem_t* pItem);
    std::string receive(void* s);


};

/**
 * @class CStateMonitorBase
 *
 * Very simple base class for state monitors.  To use it subclass and
 * override the intialState and transition methods to get the behavior you want.
 * Most clients will use the CStateMonitor class (see below).
 */
class CStateMonitorBase
{
    // public data types:
public:
    typedef void (*InitCallback)(CStateMonitorBase*);
    // object attributes.
private:
    
    zmq::context_t*     m_pContext;
    zmq::socket_t*      m_pRequestSocket;
    zmq::socket_t*      m_pStateSocket;
    CZMQEventLoop       m_eventLoop;
    std::string         m_currentState;
    int                 m_runNumber;
    std::string         m_title;
    bool                m_recording;
public:
    
    CStateMonitorBase(
        std::string transitionReqURI, std::string statePublisherURI, InitCallback cb = 0
    );
    virtual ~CStateMonitorBase();
    
    // Public methods:
public:
    std::string requestTransition(std::string transitionName);
    std::string sendRequestMessage(std::string msg);
    void run();
    CZMQEventLoop&   getEventLoop();   // Allows outsiders to add event loop callbacks.
    zmq::context_t* getContext();     // Allows outsiders to make more sockets.
    std::string     getState();       // Get current state (hide rep from derived classes).
    int             getRunNumber() const;
    std::string     getTitle()     const;
    bool            getRecording() const;
    
    // Protected overrides
protected:
    virtual void initialState(std::string state);
    virtual void transition(std::string transition);
    virtual void runNumMsg(std::string body);
    virtual void titleMsg(std::string body);
    virtual void recordMsg(std::string body);
    

    
    // Private utilities
private:
    void split(std::string& type, std::string& body, std::string message);
    void StateMessage(zmq::pollitem_t* item);
    static void StateMessageRelay(
        CZMQEventLoop* pEventLoop, zmq::pollitem_t* item, void* param
    );
    std::string getMessage(void* socket);
};




/**
 * @class CStateMonitor
 *    This is the state monitor class that most client software should use.
 *    No, really, I mean it.  This class allows the client to register
 *    interest in transitions to specific states via callbacks. This is normally
 *    what you want.. to something specific to a set of states you might be
 *    entering. State handlers get information about the prior state as well
 *    as the state that is being entered.
 *
 *    Note that state names are considered to be case in-sensitive.
 *    
 */
class CStateMonitor : public CStateMonitorBase
{
public:
    typedef void (*Callback)(CStateMonitor*, std::string, std::string, void*);
    typedef void (*TitleCallback)(CStateMonitor*, std::string, void* cd);
    typedef void (*RunNumberCallback)(CStateMonitor*, int, void* cd);
    typedef void (*RecordingCallback)(CStateMonitor*, bool, void* cd);
    
private:
    struct CallbackInfo {
        Callback          s_callback;
        void*             s_parameter;
    };
    std::map<std::string, CallbackInfo>  m_dispatch;
    TitleCallback         m_titleCallback;
    RunNumberCallback     m_runNumberCallback;
    RecordingCallback     m_recordingCallback;
    
    void*                 m_titleCBData;
    void*                 m_runNumberCBData;
    void*                 m_recordCBData;
    
    bool                  m_recordingCallbackCalled;
public:
    CStateMonitor(
        std::string transitionRequestURI, std::string statePublisherURI,
        CStateMonitorBase::InitCallback cb = 0
    );
    virtual ~CStateMonitor();

public:
    virtual void Register(std::string state, Callback cb, void* cbarg);
    virtual void unregister(std::string state);
    virtual void         setTitleCallback(TitleCallback cb, void* cd = 0);
    virtual void         setRunNumberCallback(RunNumberCallback cb, void* cd = 0);
    virtual void         setRecordingCallback(RecordingCallback cb, void* cd = 0);
    
protected:
    virtual void initialState(std::string state);
    virtual void transition(std::string newState);
    virtual void runNumMsg(std::string body);
    virtual void titleMsg(std::string body);
    virtual void recordMsg(std::string body);
    

private:
    void dispatch(std::string prior, std::string current);
    std::string toUpper(std::string in);
    
};

#endif
