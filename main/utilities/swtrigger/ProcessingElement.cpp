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

/** @file:  ProcessingElement.cpp
 *  @brief: Implement (primarly the run method) of the ProcessingElement ABC.
 */
#include "ProcessingElement.h"
#include <stdint.h>
#include <string.h>
#include <stdexcept>

/**
 * constructor
 *    We're a thread so just construct the base class.
 *    @param name - name to assign to the thread.
 */
ProcessingElement::ProcessingElement(const char* name) :
    Thread(std::string(name)), m_running(false)
{}

/**
 * Destruction is a bit tricky.
 * If this is the same thread as the ProcessingElement; if we're running
 * shutdown the source and sink and we're done.  If we're  in a different
 * thread, if we're not running, no big deal,  If we are, we need to send
 * an exit request to the thread, join and then exit.
 * Derived classes have to do the hard work because we can't get virtual calls
 *  in a destructor.
 */
ProcessingElement::~ProcessingElement()
{

}

/**
 * run
 *    This is the thread entry point.
 */
void
ProcessingElement::run()
{
    m_running = true;
    connectSource();
    connectSink();

    
    //  The main program loop:  Note that while EXIT_REQUEST
    //  explicitly sets m_running false so that the loop will exit,
    //  m_running is protected allowing any of the handlers invoked to
    //  turn it off forcing the  program to exit.
    
    while (m_running) {
        MessageType::Message msg = getNextWorkItem();
        uint32_t msgType = msg.s_messageType;
        if (msgType == MessageType::PROCESS_ITEM) {
            processWorkItem(msg);
        } else if (msgType == MessageType::REGISTRATION) {
            onRegister(msg);
        } else if (msgType == MessageType::UNREGISTRATION) {
            onUnregister(msg);
        } else if (msgType == MessageType::EXIT_REQUEST) {
            onExitRequested(msg);
            m_running = false;
        } else if (msgType == MessageType::END_ITEM) {
            onEndItem(msg);
        } else if (msgType == MessageType::DATA_REQ) {
            onDataRequest(msg);
        } else {
            onOtherMessageType(msg);
        }
    }
    
    disconnectSource();
    disconnectSink();
}
/**
 * terminateAndJoin
 *   Must only be called from other threads.  If the
 *   thread is running it's asked to stop and joined.
 *   @todo - locking on m_running to close timing holes but this is an
 *           edge case typically at program exit so not urgent.
 */
void ProcessingElement::terminateAndJoin()
{
    if (m_running) {
        void* c = connectAsSource();
        requestExit(c);
        closeSource(c);
        join();              // Run will disconnect and cleanup.
    }
}
////////////////////////////////////////////////////////////////////////
// Strategy handlers.  C++ actually allows pure virtual methods to be
// implemented.  I'll not go into why.  See e.g.:
// https://blogs.msdn.microsoft.com/oldnewthing/20131011-00/?p=2953
// if you have interest in this.  In this case, we implement the
// pure virtuals just to allow documentation comments for them to be
// in the implementation rather than in the header.
//

/**
 * connectSource
 *    This will be called exactly once before the main processing
 *    loop is entered.  It is expected to do whatever is need to establish
 *    a connection with the data source.  Note that the term 'data source'
 *    is used rather loosely.  It could just as easily mean a file as
 *    some communications medium (e.g,. TCP socket, shared memory message queu
 *    or ZMQ socket
 */

void ProcessingElement::connectSource() {}

/**
 * connectSink
 *     This is called exactly once before main the main processing loop is
 *     entered.  It's intended to establish a connection with the sink of
 *     data.  Again the sink could be a communications object or it could
 *     be a file.
 */
void ProcessingElement::connectSink() {}

/**
 * disconnectSource
 *    Called just before thread exit to disconnect the data source.
 *    connected by connectSource().
 */
void ProcessingElement::disconnectSource() {}

/**
 * disconnectSink
 *    Called just before thread exit to disconnect the data sink connected
 *    by connectSink()
 */
void ProcessingElement::disconnectSink() {}

/**
 * getNextWorkItem
 * @return MessageType::Message
 *    Returns the next work item from the data source.  The type of
 *    the work item is used by the run() method to dispatch to the
 *    appropriate strategy handler (another pure virtual method).
 *    Note that this is expected to retrieve work items from the data
 *    source connected via connectSource().
 */
MessageType::Message ProcessingElement::getNextWorkItem() {}

/**
 * sendWorkItemToSink
 *    Processing elements have data come, get processed which
 *    results in data being passed to the next stage of the
 *    logical pipeline.  Parallelism is present in the pipelining
 *    as well as in the fanout/fanin if the data flow as concocted
 *    by the actual application.  sendWorkItemToSink is the method
 *    that should be called to send a work item to the next stage of
 *    the processing pipeline connected to this object by
 *    connectSink().  Note that it's also very possible that
 *    this could just write something to an output file.
 *
 *    @param workitem - the work item to forward on to the sink.
 */
void ProcessingElement::sendWorkItemToSink(MessageType::Message& workItem)
{}

////////////////////////////////////////////////////////////////////////
// Given that a work item has been retrieved, these methods are
// called by run() depending on the type of item.  They are expected
// to process the item and may (or may not) call sendWorkItemToSink
// if a work item is ready to be sent off.
//


/**
 * onRegister
 *    Called when a client registration message has been sent.
 *    Some applications  may require that a processing element
 *    know who it is communicating with.  Those processing elements
 *    can send registration messages which, in turn, are processed by
 *    this method in the processing thread.
 *
 *  @param reg   Registration message.  It's type will be
 *               MessageType::REGISTRATION
 */
void ProcessingElement::onRegister(MessageType::Message& reg)  {}


/**
 * onUnregister
 *    Similarly this processses unregistration requests from clients.
 *
 * @param reg - the unregistration message.  It's message type will be
 *              MessageType::UNREGISTRATION
 */
void ProcessingElement::onUnregister(MessageType::Message& reg)  {}

/**
 * processWorkItem
 *    This is the method that's called for a data work item.
 *    Normally this is where the computationally intensive work
 *    is done.
 *  @param item - the work item to process.
 */
void ProcessingElement::processWorkItem(MessageType::Message& item) {}

/**
 * onOtherMessageType
 *    The set of message types in MessageTypes.h is a minimal set.
 *    Other unanticipated message types may be needed.  If a message
 *    type is encounted that does not match one of the ones
 *    defined in MessageTypes.h, this method is called to process
 *    it.
 *
 *  @param item - the message.
 */
void ProcessingElement::onOtherMessageType(MessageType::Message& item) {}

/**
 * onEndItem
 *    Called when no more work items are expected.  If the thread is
 *    registered with other objects it should normally unregister at this
 *    time.   This method often sets m_running false to indicate the
 *    thread should exit.
 * @param msg - the end item messgae.
 */
void ProcessingElement::onEndItem(MessageType::Message& endItem) {}

/*
 * onExitRequested
 *    Called when the thread is requested to exit.  The user's code
 *    should _at least_ set m_running to false so that the thread exits.
 *    One use case for this is if our destructor is invoked from outside
 *    the processing thread while the thread is running -- in that case
 *    the thread has to be asked to exit and then joined prior to
 *    allowing the destructor to complete (which can yank the rug
 *    out from underneath the code).  Note that derived classes
 *    with non-trivial destructors should invoke terminateAndJoin before
 *    destroying any data structs for the same reasons we do.
 *    
 *  @param msg  - exit request message.
 */
void ProcessingElement::onExitRequested(MessageType::Message& item)
{}

/**
 * onDataRequest
 *    Some protocols require explicit pulls for data, for example, in
 *    the ZMQ ROUTER/DEALER communication pattern, rate based
 *    data distribution is done by the sink requesting data from
 *    the source which is then routed directly to it.
 *    In that model, clients call our requestItem to send a request
 *    for data.  Once that's received, onDataRequest is called to
 *    supply the next available work item.
 *
 *     We supply a nulll method so that non explicit pull protocols
 *     don't need to override this.
 *
 *  @param item - message requesting data.
 */
void ProcessingElement::onDataRequest(MessageType::Message& item)
{}

////////////////////////////////////////////////////////////////////////
// These methods are intended to be called by
// threads other than the processing element. The encapsulate
// communication with the thread by other threads.

/**
 * connectAsSource
 *     Returns some opaque handle that represents an active connection
 *     to this processing element as a source of data (messages).
 *  @return void* - a full opaque connection handle.  This can be used in
 *     calls to the communication primitives below, though some of these
 *     may  only be able to accept source or sink connection objects.
 *     (application dependent).
 */
void* ProcessingElement::connectAsSource() {}

/**
 * closeSource
 *    Close a connection as a data source to the thread.
 * @param c opaque connection object.
 */
void ProcessingElement::closeSource(void* c) {}

/**
 * connectAsSink
 *   opens a connection to this object as a data sink.
 * @return void* opaque handle that represents the connection.
 */
void* ProcessingElement::connectAsSink() {}

/**
 * closeSink
 *    Closes a sink connection
 *  void* c - sink connection handle.
 */
void ProcessingElement::closeSink(void* c) {}


/**
 * sendMessageToThread
 *    Sends a message to the processing thread.  How this is done
 *    depends on the underlying communication scheme.
 *  @param c        - either a source or sink connection (see above).
 *                    Any restrictions on which this must be have to be enforced
 *                    by the caller.
 *  @param message  - The message to send.
 */
void ProcessingElement::sendMessageToThread(void* c, MessageType::Message& item)
{}

/**
 * receiveMessage
 *    This method receives the next message from a client.
 * @return MessageType::Message - the message received.
 */
MessageType::Message
ProcessingElement::receiveMessage() {}

/**
 * registerClient
 * Sends a registration message to the thread.  On receipt of that
 * message, eventually, onRegister will be called in the thread.
 * 
 * @param  c        - Connection to the processing element.  This code
 *                    does not restrict the connection type.  If you want to
 *                    override, check and if ok call us else throw e.g.
 * @param  identity - Client identity - you could use the thread name
 *            if you assign unique names.
 *
 *  The default implementation creates a message that consist of the following
 *  parts:
 * 
 *     +-------------------------+
 *     | Identity string         |
 *     +-------------------------+
 *
 *   Then uses sendMessageToThread to send the message.
 */
void ProcessingElement::registerClient(void* c, std::string identity)
{
    sendMessageWithIdentityToThread(c, MessageType::REGISTRATION, identity);
}
/**
 * unregisterClient
 *    sends an unregister message to the thread.  On receipt of that
 *    message, the thread will call onUnregister passing the message we
 *    construct.  The message will have the type UNREGISTRATION and a single
 *    message part consisting of the identity string itself.
 * @param c          - Source or sink connection.  We don't restrict the type
 *                     as we don't know enough to do so.
 * @param identity   - Identity of the processing element being unregistered.
 */
void ProcessingElement::unregisterClient(void* c, std::string identity)
{
    sendMessageWithIdentityToThread(c, MessageType::UNREGISTRATION, identity);
}
/**
 * queueWorkItem
 *   This sends a process item to the thread.  When received this is passed
 *   to the processWorkItem method.
 *
 * @param c    - Connection - this should normally be  a data source
 *               connection, however we don't know how to distinguish so the
 *               caller should implement the check and then delegate.
 * @param item - pairs of sizes and item pointers.  These will be
 *               copied to the s_dataParts chunk of the message.
 */
void
ProcessingElement::queueWorkItem(
    void* c, std::list<std::pair<uint32_t, void*> >& item
)
{
    MessageType::Message msg;
    msg.s_messageType = MessageType::PROCESS_ITEM;
    msg.s_dataParts = item;
    
    sendMessageToThread(c, msg); 
}
/**
 * requestItem
 *    For pull protocols, this is used by a sink thread to request its next
 *    work item.  When the message is received it is passed to
 *    onDataRequest which is expected (at some point) to satisfy the
 *    request.
 * @param c  - represens a connection.  Only sink connections should be able
 *             to call this but we don't know how to enforce that.
 * @param identity - identity of the requestor.
 */
void
ProcessingElement::requestItem(void* c, std::string identity)
{
    sendMessageWithIdentityToThread(c, MessageType::DATA_REQ, identity);
}
/**
 * noMoreData
 *   sends an empty messsage with the type: END_ITEM indicating the
 *   data source sending this message has no more data to send the
 *   thread.
 *
 *   @param c - connection object.  Note that only data source connections should
 *              be able to call this.
 */
void ProcessingElement::noMoreData(void* c)
{
    MessageType::Message m;
    m.s_messageType = MessageType::END_ITEM;
    sendMessageToThread(c, m);
}
/**
 * requestExit
 *    sends a EXIT_REQUEST message to the thread.
 *
 *  @param c - connection object either sources or sinks should be able to request
 *            an exit.
 */
void ProcessingElement::requestExit(void* c)
{
    MessageType::Message m;
    m.s_messageType = MessageType::EXIT_REQUEST;
    sendMessageToThread(c, m);
}
////////////////////////////////////////////////////////////////////
// Private utilities

/**
 * sendMessageWithIdentityToThread
 *    Several message types consist of a type and a segment
 *    with a string identity.  This method constructs and sends
 *    such messages
 *
 * @param c    - connection.
 * @param type - type of message.
 * @param identity - string to send.
 */
void
ProcessingElement::sendMessageWithIdentityToThread(
    void* c, uint32_t type, std::string identity
)
{
    MessageType::Message msg;
    msg.s_messageType = type;
    
    char identityString[identity.size() +1];
    strcpy(identityString, identity.c_str());
    msg.s_dataParts.push_back({identity.size() + 1, identityString});
    
    sendMessageToThread(c, msg);    
}