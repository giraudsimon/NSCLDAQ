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
# @file   CStateMonitor.cpp
# @brief  Classes that provide the state monitor framework.
# @author <fox@nscl.msu.edu>
*/

#include "CStateMonitor.h"
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <cctype>



/*----------------------------------------------------------------------------*/
/**
 * Implementation of CZMQEventLoop
 * an event loop over file descriptors and zmq socket objects with
 * settable callbacks.
 */


/**
* constrructor
*   
*/
CZMQEventLoop::CZMQEventLoop() 
{}


/**
 * register
 *    Register a socket event.  This causes the next poll operations
 *    to trigger events when that socket satisfies the event mask described
 *    by the mask parameter.
 *
 *  @param socket - The zeromq socket we want to register an event for.
 *  @param mask   - The mask of events we want to be sensitive to.  This is the
 *                  OR of any of the following:
 *                  * ZMQ_POLLIN - Event fires if the socket is readable.
 *                  * ZMQ_POLLOUT - Event fires if the socket is writable.
 *                  * ZMQ_POLLERR - Event fires if the socket has an error,.
 *  @param callback - Callback that responds to events on the socket.
 *  @param param     - Parameter passed without interpretation to the callback.
 *
 *  @note - a socket can only have a single callback.  Registering another one
 *          wipes out the existing registration.
 */
void
CZMQEventLoop::Register(
    zmq::socket_t& socket, int mask , CZMQEventLoop::Callback callback, void* param)
{
    unregister(socket);                      // Get rid of any existing registration.
    
    SocketRegistration reg;
    reg.s_event.s_socket   = &socket;
    reg.s_event.s_events   = mask;
    reg.s_cb               = callback;
    reg.s_param            = param;
    
    m_socketRegistrations.push_back(reg);
}
/**
 * register
 *   Register a file descriptor event.   This causes the next poll operations
 *   to trigger events when that file descriptor satisfies the event mask
 *   described by the mask parameter.
 *
 *   @param fd    - The file descriptor we want to regsiter a callback for.
 *  @param mask   - The mask of events we want to be sensitive to.  This is the
 *                  OR of any of the following:
 *                  * ZMQ_POLLIN - Event fires if the socket is readable.
 *                  * ZMQ_POLLOUT - Event fires if the socket is writable.
 *                  * ZMQ_POLLERR - Event fires if the socket has an error,.
 *  @param callback - Callback that responds to events on the socket.
 *  @param param    - Parameter passed without interpretation to the callaack.
 *  @note - a file descriptor can only have a single callback.  Registering another one
 *          wipes out the existing registration.
 */
void
CZMQEventLoop::Register(int fd, int mask, CZMQEventLoop::Callback callback, void* param)
{
    unregister(fd);
    
    FdRegistration reg;
    reg.s_event.s_fd       = fd;
    reg.s_event.s_events   = mask;
    reg.s_cb               = callback;
    reg.s_param            = param;
    m_fdRegistrations.push_back(reg);
    
}
/**
 * unregister
 *    Unregister a zmq::socket_t event registration.  If called with a socket
 *    that has not yet been registered, this just silently does nothing.
 *
 *  @param socket - the socket to unregister.
 */
void
CZMQEventLoop::unregister(zmq::socket_t& socket)
{
    for(std::vector<SocketRegistration>::iterator p = m_socketRegistrations.begin();
        p != m_socketRegistrations.end(); p++) {
        if (p->s_event.s_socket == socket) {
            m_socketRegistrations.erase(p);
            break;                                 // p has been invalidated by this.
        }
    }
}
/**
 * unregister
 *    Unregister a filedescriptor event registration.  If called with a fd
 *    that has not yet been registered, this just silently does nothing.
 *
 *  @param fd   - the file descriptor to unregister.
 */
void
CZMQEventLoop::unregister(int fd)
{
    for(std::vector<FdRegistration>::iterator p = m_fdRegistrations.begin();
        p != m_fdRegistrations.end(); p++) {
        if (p->s_event.s_fd == fd) {
            m_fdRegistrations.erase(p);
            break;                                 // p has been invalidated by this.
        }
    }
}
/**
 * poll
 *   Use zmq poll to wait for events on the sockets/fds.
 *
 * @param timeout - ms to wait.
 */
void
CZMQEventLoop::poll(int timeout)
{
    zmq::pollitem_t* items;
    size_t nItems = makePollItems(&items);
    zmq::poll(items, nItems, timeout);
    dispatch(items, nItems);
    delete[] items;
    
}
/**
 * pollForever
 *    Polls until a callback tells us to stop.
 *
 * @param timeout - timeout for each poll operationin usec.
 * @param idler   - Called after each poll operations (whether there are events
 *                  or not).  The idler is passed a pointer to us.
 *                  If the idler returns false, the loop exits.
 */
void
CZMQEventLoop::pollForever(int timeout, CZMQEventLoop::IdleCallback idler)
{
    do {
        poll(timeout);
        
    } while (idler ? idler(this) : true);
}

/*---------------------------------------------------------------------------
 * Private methods for CZMQventLoop.
 */

/**
 * makePollItems
 *    Creates the array of zmq::pollitem_t from the two registration lists.
 *
 * @param ppItems   - Points to a pointer that will beset to point to the
 *                    resulting list.
 * @return size_t   - Number of items that were created.
 */
size_t
CZMQEventLoop::makePollItems(zmq::pollitem_t** ppItems)
{
    size_t nItems = m_socketRegistrations.size() + m_fdRegistrations.size();
    *ppItems      = new zmq::pollitem_t[nItems];
    
    zmq::pollitem_t* pItem = *ppItems;
    
    // Put in the zmq::socket-t's first.
    
    for (std::vector<SocketRegistration>::iterator p = m_socketRegistrations.begin();
         p != m_socketRegistrations.end(); p++) {
         pItem->socket   = *(p->s_event.s_socket);
         pItem->fd       = 0;
         pItem->events   = p->s_event.s_events;
         pItem->revents  = 0;
         
         pItem++;
    }
    
    // put in the fds next.
    
    for (std::vector<FdRegistration>::iterator p = m_fdRegistrations.begin();
         p != m_fdRegistrations.end(); p++) {
         pItem->socket   = 0;
         pItem->fd       = p->s_event.s_fd;
         pItem->events   = p->s_event.s_events;
         pItem->revents  = 0;
         
         pItem++;
    }
    
    // Return the total number of items.
    
    return nItems;
}
/**
 * findCallback
 *    Given an event, if appropraite locates and returns the callback for it.
 *    *   If revents  is 0, null is returned.
 *    *   If the socket is non zero the m_socketRegistrations is searched.
 *    *   If the fd  is non zero the m_fdRegistrations is searched.
 *    Note that the ordering above preserves the documentated behavior that if
 *    both an fd and a socket are specified the socket wins.
 *
 * @param item - A zmq::pollitem_t* pointing to the item to consider.
 */
std::pair<CZMQEventLoop::Callback,void*>
CZMQEventLoop::findCallback(zmq::pollitem_t* pItem)
{
    if(pItem->revents != 0) {
        if (pItem->socket) {
            for (std::vector<SocketRegistration>::iterator p = m_socketRegistrations.begin();
                 p != m_socketRegistrations.end(); p++) {
	      if (*(p->s_event.s_socket) == (pItem->socket)) {
                    return std::pair<Callback, void*>(p->s_cb, p->s_param);
                }
            }
            
            return std::pair<Callback, void*>(0,0);
        } else {
             for (std::vector<FdRegistration>::iterator p = m_fdRegistrations.begin();
                 p != m_fdRegistrations.end(); p++) {
                if (p->s_event.s_fd == pItem->fd) {
                    return std::pair<Callback, void*>(p->s_cb, p->s_param);
                }
            }
            
            return std::pair<Callback, void*>(0,0);           
        }
    }
    return std::pair<Callback, void*>(0,0);
}
/**
 * dispatch
 *    Given the items from a poll, dispatches an and all events that deserve
 *    a dispatch.
 *  @param pItems - Pointer to the modified pollitems.
 *  @param count  - Number of pollitems.
 */
void
CZMQEventLoop::dispatch(zmq::pollitem_t* pItems, size_t count)
{
    for (int i =0; i < count; i++) {
        if (pItems->revents) {
            std::pair<Callback, void*>cbInfo  = findCallback(pItems);
            Callback pFunction = cbInfo.first;
            void*    param     = cbInfo.second;
            if (pFunction) {
                (*pFunction)(this, pItems, param);
            }
        }
        pItems++;
    }
}
/*------------------------------------------------------------------------------
 * Implementation of CStateMonitorBase
 */

/**
 * constructor
 *    *   Create the event loop.
 *    *   Construct sockets for the transition requestor and the state subscription.
 *    *   Set our subscriptions.
 *    *   Set up the poller so that we have our callbacks
 *    
 * @param transitionReqURI  -- The URI of the transition request port.
 * @param statePublisherURI -- The URI of the state publisher.
 * @param cb                -- Function called when everything is initialized.
 */
CStateMonitorBase::CStateMonitorBase(
    std::string transitionReqURI, std::string statePublisherURI, CStateMonitorBase::InitCallback cb
) : m_pContext(0), m_pRequestSocket(0), m_pStateSocket(0), m_runNumber(-1), m_recording(false)
{
    // Actually the event loop constructs itself
    
    // Create the ZMQ context and and the sockets:
    
    m_pContext = new zmq::context_t(1);
    
    m_pRequestSocket = new zmq::socket_t(*m_pContext, ZMQ_REQ);
    m_pRequestSocket->connect(transitionReqURI.c_str());
    
    m_pStateSocket   = new zmq::socket_t(*m_pContext, ZMQ_SUB);
    m_pStateSocket->connect(statePublisherURI.c_str());
    
    // Set up the subscriptions for TRANSITION: and STATE:
    
    m_pStateSocket->setsockopt(ZMQ_SUBSCRIBE, "STATE:", 6);
    m_pStateSocket->setsockopt(ZMQ_SUBSCRIBE, "TRANSITION:", 10);
    m_pStateSocket->setsockopt(ZMQ_SUBSCRIBE, "RUN:", 4);
    m_pStateSocket->setsockopt(ZMQ_SUBSCRIBE, "TITLE:", 6);
    m_pStateSocket->setsockopt(ZMQ_SUBSCRIBE, "RECORD:", 7);
    
    //  Register the state socket readability with the event loop:
    
    m_eventLoop.Register(
        *m_pStateSocket, ZMQ_POLLIN, CStateMonitorBase::StateMessageRelay, this
    );
    
    // Call our initialization callback if it's not null:
    
    if (cb) {
        (*cb)(this);
    }
    
    
}
/**
 * destructor
 *    *  unregister the state publication socket.
 *    *  close both sockets.
 *    *  Destroy the ZMQ context (that should shtudown zmq)
 */
CStateMonitorBase::~CStateMonitorBase()
{
    m_eventLoop.unregister(*m_pStateSocket);
    
    m_pRequestSocket->close();
    m_pStateSocket->close();
    
    delete m_pRequestSocket;
    delete m_pStateSocket;
    
    delete m_pContext;               // Maps to zmq_term according to docs.
}
/*
 * Implementation of public methods:
 */

/**
 * requestTransition
 *    Ask the state machine to perform a transition on our behalf.
 *
 * @param transition - The transition requested.  Note that transitions are
 *                     not the same as a target state name, e.g. to go from
 *                     NOTREADY to READY, we do a START transition.
 * @return std::string - The reply message from the server.
 */
std::string
CStateMonitorBase::requestTransition(std::string transitionName)
{
    std::string msg = "TRANSITION:";
    msg            += transitionName;
    return sendRequestMessage(msg);
}
/**
 * sendRequestMessage
 *    Sends a message to the request port.
 *
 *  @param msg - the message to send.
 *  @return std::string the response message.
 */
std::string
CStateMonitorBase::sendRequestMessage(std::string msg)
{
    zmq::message_t trMessage(const_cast<char*>(msg.c_str()), msg.size(), 0);
    m_pRequestSocket->send(trMessage);
    
    return getMessage(*m_pRequestSocket);
}
/**
 * run
 *    Run the event loop.
 */
void
CStateMonitorBase::run()
{
    
    while(true) {
        m_eventLoop.pollForever(10*1000*1000);   // Wake up every 10ms
    }
}

/*
 * getters:
 */

CZMQEventLoop&
CStateMonitorBase::getEventLoop() {return m_eventLoop;}

zmq::context_t*
CStateMonitorBase::getContext()   {return m_pContext;}

std::string
CStateMonitorBase::getState()     {return m_currentState;}

int
CStateMonitorBase::getRunNumber() const {return m_runNumber;}
std::string
CStateMonitorBase::getTitle() const {return m_title;}

bool
CStateMonitorBase::getRecording() const {return m_recording;}

/*
 * Protected methods (virtual ones get overriden but these base class
 * members should stil be called fromt the override)
 */

/**
 * initialState
 *    This is called from StateMessage if:
 *    *  We got a STATE: message.
 *    *  We got a TRANSITION: message without getting a STATE: message first
 *       (that indicates a transition occured before we got the first periodic
 *       STATE publication)
 *       
 *    The following actions are performed:
 *    *   The m_currentState is updated to reflect the new state.
 *    *   The STATE:: subscription is cancelled.
 *
 * @param state - Contains the new state.
 */
void
CStateMonitorBase::initialState(std::string state)
{
    m_currentState = state;
    m_pStateSocket->setsockopt(ZMQ_UNSUBSCRIBE, "STATE:", 6);
}

/**
 * transition
 *   Called from State Message if it got a TRANSITION: message.
 *
 * @param transition - the state we've just transitioned to.
 */
void
CStateMonitorBase::transition(std::string transition)
{
    // If there is no prior state, treat this like an initialState
    // otherwise, just set the current state:
    
    if (m_currentState == "") {
        initialState(transition);
    } else {
        m_currentState = transition;
    }
}
/**
 * runNumMsg
 *    Called when a run number message has been received
 *
 *   @param body - the body of the message.
 */
void
CStateMonitorBase::runNumMsg(std::string body)
{
    m_runNumber = atoi(body.c_str());
}
/**
 * titleMsg
 *   Called when a title message has been received.
 *
 *   @param body - the body of the message.
 */
void
CStateMonitorBase::titleMsg(std::string body)
{
    m_title = body;
}
/**
 * recordMsg
 * Called whena new recording message is received.
 * The body of the message is either "True" or "False" depending
 * on the recording state.
 *
 * @param body - message body.
 */
void
CStateMonitorBase::recordMsg(std::string body)
{
    m_recording = (body == "True");
}
/**
 * Private utilities;
 */

/**
 * split
 *    Splits the message up into its constituent chunks.
 *    (the message type e.g. STATE | TRANSITION, and the message body
 *    which is the new or current state name).  Note that state names
 *    are upper cased to be consistent with how they are managed by the
 *    state manager.
 *
 *  @param[out] type -- The message type ("STATE" | "TRANSITION").
 *  @param[out] body -- The body of the message after the ':' delimeter.
 *  @param[in]  message -- The stringized message.
 *
 *  @note with thanks to stackover flow and Evan Teran at:
 *        http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
 */
void
CStateMonitorBase::split(std::string& type, std::string& body, std::string message)
{
    std::stringstream m(message);
    std::string       item;
    
    std::getline(m, item, ':');
    type = item;
    
    std::getline(m, item, ':');
    body = item;
    
}
/**
 * stateMessage
 *    Called when a message comes on on our subscription.
 *    *  Read the message
 *    *  Split it into the type and body.
 *    *  Invoke transition or initialState or bitch that the type is illegal.
 *   
 *  @param item  - Pointer to the poll item that fired the event.
 *                 the s_socket element is our C socket.
 */
void
CStateMonitorBase::StateMessage(zmq::pollitem_t* item)
{
    std::string  msg = getMessage(item->socket);
    std::string type;
    std::string state;
    
    split(type, state, msg);
    
    if(type == "STATE") {
        initialState(state);
    } else if (type == "TRANSITION") {
        transition(state);
    } else if (type == "RUN") {
        runNumMsg(state);
    } else if (type == "TITLE") {
        titleMsg(state);
    } else if (type == "RECORD") {
        recordMsg(state);
    } else {
        std::string failure("Invalid message type received: ");
        failure += type;
        throw failure;
    }
    
}
/**
 * StateMessageRelay
 *    Actual poller callback.. transition to object context and do the real work
 *    in  StateMessage
 *
 *  @param pEventLoop - pointer to the event loop.
 *  @param pItem      - Pointer to poll item that fired us off.
 *  @param param      - Actually a pointer to the object running the monitor.
 */
// static
void
CStateMonitorBase::StateMessageRelay(
    CZMQEventLoop* pEventLoop, zmq::pollitem_t* item, void* param
)
{
    CStateMonitorBase* pObject = reinterpret_cast<CStateMonitorBase*>(param);
    pObject->StateMessage(item);
}
/**
 * getMessage
 *    Get a message from a zmq socket and return it as an std::string.
 *    This only makes sense obviously for sockets that toss around
 *    textual messages.
 * @param socket  -  zmq socket to recv from.
 * @return std::string - the message string gotten from the socket.
 */
std::string
CStateMonitorBase::getMessage(void* socket)
{
    zmq_msg_t  message;
    zmq_msg_init(&message);
    
    zmq_recv(socket, &message, 0);
    
    size_t n = zmq_msg_size(&message);
    char msg[n+1];
    memset(msg, 0, n+1);
    memcpy(msg, zmq_msg_data(&message), n);
    
    
    zmq_msg_close(&message);
    
    return std::string(msg);
}

/*-----------------------------------------------------------------------------
 * Implementation of  CStateMonitor
 *
 */

/**
 * constructor
 *    All the work is done in the base class.
 *
 * @param transitionReqURI  -- The URI of the transition request port.
 * @param statePublisherURI -- The URI of the state publisher.
 * @param cb                -- Function called when everything is initialized.
 */
CStateMonitor::CStateMonitor(
    std::string transitionReqURI, std::string statePublisherURI,
    CStateMonitorBase::InitCallback cb
) : CStateMonitorBase(transitionReqURI, statePublisherURI, cb),
    m_titleCallback(0),
    m_runNumberCallback(0),
    m_titleCBData(0),
    m_runNumberCBData(0),
    m_recordCBData(0),
    m_recordingCallbackCalled(false)
{}
/**
 * Destructor is also handled by the base class.
 */
CStateMonitor::~CStateMonitor() {}

/**
 * Register
 *    Register a state handler.  The CallbackInfo struct is filled in.  It is
 *    then copied into the m_dispatch map indexed by the uppercased target state
 *    name.
 */
void
CStateMonitor::Register(std::string state, Callback cb, void* cbarg)
{
    state               = toUpper(state);                // To get case insensitivity.
    CallbackInfo cbInfo = {cb, cbarg};
    m_dispatch[state]   = cbInfo;
}
/**
 * unregister
 *   Remove a state handler.  The entry is just removed from the map.
 *   Note that it is a silent no-op to remove a state registration that does
 *   not exist.
 *
 *  @param state - name of the state to unregister a callback for.
 */
void
CStateMonitor::unregister(std::string state)
{
    state = toUpper(state);
    std::map<std::string, CallbackInfo>::iterator p = m_dispatch.find(state);
    if (p != m_dispatch.end()) {
        m_dispatch.erase(p);
    }
}
/**
 * setTitleCallback
 *    Set a callback that is invoked when the title changes.
 *  @param cb - Pointer to the callback function. This should be null to
 *              disable callbacks.
 *  @param cd - data passed without interpretation to the callbackk.
 */
 void
 CStateMonitor::setTitleCallback(CStateMonitor::TitleCallback cb, void* cd)
 {
    m_titleCallback = cb;
    m_titleCBData   = cd;
 }
 /**
  * setRunNumberCallback
  *    Set the callback that is invoked when the run number changes
  *   
  *  @param cb - Pointer to the callback function. This should be null to
  *              disable callbacks.
  *  @param cd - data passed without interpretation to the callback.
 */
void
CStateMonitor::setRunNumberCallback(CStateMonitor::RunNumberCallback cb, void* cd)
{
    m_runNumberCallback = cb;
    m_runNumberCBData   = cd;
}
/**
 * setRecordingCallback
 *    Set the callback and data that are invoked when the state of the
 *    recording flag.
 *
 *    @param cb - The callback
 *    @param cd - Data passed without interpretation to the callback.
 */
void
CStateMonitor::setRecordingCallback(RecordingCallback cb, void* cd)
{
    m_recordingCallback = cb;
    m_recordCBData   = cd;
}
  
 /*-------------------------------------------------------------------
 * protected method overrides.
 */
/**
 * initialState
 *    Called when the state is initially known (when we don't know a prior
 *    state).
 *    *  Invoke the base class method.l
 *    *  Invoke any handler for the state using an empty prior state string.
 *
 *  @param state - New state string.
 */
void
CStateMonitor::initialState(std::string state)
{
    CStateMonitorBase::initialState(state);
    dispatch("", state);
}
/**
 * transition
 *    Called when we do a transition.
 *    *  Save the prior state (it's overwritten by base class transition)
 *    *  Call the base class transition
 *    *  If the prior state is not empty dispatch any handler. Note that if the
 *        prior state is empty, the base class will eventually invoke
 *        initialState which will do the dispatch.
 *
 *  @param newState - New state we are transitioning into.
 */
void
CStateMonitor::transition(std::string newState)
{
    std::string oldState = getState();
    CStateMonitorBase::transition(newState);
    
    if (oldState != "") {
        dispatch(oldState, newState);
    }
}
/**
 * runNumMsg
 *   Called when a message for a run number changes has occured,
 *   *   Invoke the base class method.
 *   *   If there's a callback, invoke it - note the callback is a oneshot.
 *
 *  @param body - stringified new run number
 */
void
CStateMonitor::runNumMsg(std::string body)
{
    int oldRun = getRunNumber();
    CStateMonitorBase::runNumMsg(body);
    int newRun = getRunNumber();
    if (m_runNumberCallback && (oldRun != newRun)) {
        (*m_runNumberCallback)(this, newRun, m_runNumberCBData);
    }
}
/**
 * titleMsg
 *   Called when a new title message is received:
 *   *  Call the base class method
 *   *  If there's a callback, invoke it - note the callback is a oneshot
 *
 *  @param body - body of the message - the  new title.
 */
void
CStateMonitor::titleMsg(std::string body)
{
    std::string oldTitle = getTitle();
    CStateMonitorBase::titleMsg(body);
    std::string newTitle = getTitle();
    if (m_titleCallback && (oldTitle != newTitle)) {
        (*m_titleCallback)(this, newTitle, m_titleCBData);
    }
}
/**
 * recordMsg
 *    Called when a new recording message is received.
 *    *  Call the base class method.
 *    *  If the recording state changed, or if we've never called the callback,
 *       invoke it (if it exists).  This first time logic is needed because
 *       there's no 'evil' state for the valoue that we can use to distinguish
 *       the first notification.
 *  @param body - body of the message
 */
void
CStateMonitor::recordMsg(std::string body)
{
    bool priorState = getRecording();
    CStateMonitorBase::recordMsg(body);
    bool currentState = getRecording();
    
    if (m_recordingCallback &&
        ((currentState != priorState) || !m_recordingCallbackCalled)
    ) {
        (*m_recordingCallback)(this, currentState, m_recordCBData);
        m_recordingCallbackCalled = true;
    }
}

/*----------------------------------------------------------------------------
 * Implement private utilities.
 */

/**
 * dispatch
 *    Dispatches a state handler given the prior and current states.
 *
 * @param prior   - Previous state name.
 * @param current - Current state name.
 * @note the prior and current states are passed to the state handler upcased.
 */
void
CStateMonitor::dispatch(std::string prior, std::string current)
{
    prior   = toUpper(prior);
    current = toUpper(current);
    
    std::map<std::string, CallbackInfo>::iterator p = m_dispatch.find(current);
    if (p != m_dispatch.end()) {
        CallbackInfo cinfo = p->second;
        (*cinfo.s_callback)(this, prior, current, cinfo.s_parameter);
    }
    
}
/**
 * toUpper
 *     return the uppercased version of an std::string.  This uses
 *     std::transform as per the recipe posted at
 *     http://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case
 *     by Pierre and Johanes Schaub.  The idea is that tranform applies a function
 *     to each item in a range of a container defined by begin/end iterators.
 *     The transformation is actually in place but since the parameter is
 *     passed by value that does not matter.
 *
 *  @param in - The input string.
 *  @return std::string - the transformed string.
 */
std::string
CStateMonitor::toUpper(std::string in)
{
    std::transform(in.begin(), in.end(), in.begin(), ::toupper);
    return in;                 // Which has been transformed above.
}