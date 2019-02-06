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

/** @file:  CRingItemPipelineProcessor.cpp
 *  @brief: Implements the methods of CRingItemPipelineProcessor.
 *
 */
#include "CRingItemPipelineProcessor.h"
#include "CRingItemDataSource.h"
#include "CRingItemProcessor.h"
#include "CRingItemDataSink.h"

/**
 * @note - Much of the logic of this class is deciding how to delegate
 *        to the methods in its data source, processor and data sink.
 */


/**
 * constructor
 *    Saves the source, sink and processor.  Ownership is not transferred
 *    to this object.  Disposal of the objects is the responsibility
 *    of the caller or happens at end of program.
 *    @param name - name of the thread.
 *    @param src  - pointer to the source object that will be used.
 *    @param sink - Pointer to the sink object that will be used.
 *    @param prc  - Pointer to the processor that will be used.
 *    
 */
CRingItemPipelineProcessor::CRingItemPipelineProcessor(
    const char* name,
    CRingItemDataSource* src, CRingItemDataSink* sink, CRingItemProcessor* prc
) : ProcessingElement(name),
    m_pSource(src), m_pSink(sink), m_pProcessor(prc)
{}
/**
 * destructor
 *    Currently empty as ownership of the source and sink are not transferred.
 */

CRingItemPipelineProcessor::~CRingItemPipelineProcessor() {}

/**
 * connectSource
 *    This is supposd to connect the data source it whatever it gets data
 *    from.  There are three possible cases concrete CRingDataSource classes need
 *    to consider when being written:
 *    -  The source is intended to be a client of some sort.  In this case,
 *       the source should connect to its server.  This is called in the thread
 *        of the processing element.
 *    -  The source is intended to be a servr of some sort.  In this case,
 *       the source should set up and advertise its service, this is done in
 *       the thread of the processor.  Other threads, then call
 *       our connectAsSource (in their thread) which then calls connectSource
 *       in their thread to perform the connection to the server.
 *    - The source connects to some non message passing source of ring items
 *      (e.g. an event file or a ringbuffer).  In that case, this should do
 *      what's needed to connect to that source.
 */
void
CRingItemPipelineProcessor::connectSource()
{
    m_pSource->connect();
}
/**
 * disconnectSource
 *    This is called in the thread of the processing element just before
 *    it exits.  In the three cases desribed in connectSource (in order)
 *    -   The source should close it's connection with its server.
 *    -   The source should stop advertising its service for new connections.
 *        if the application protocol managed by this object supports doing so,
 *        the clients should be told to drop their connections as well.
 *    -   The source should close its connection with whatever source of
 *        data there might be (e.g. close the file, destroy the CRingBuffer or
 *        CRingDataSource or whatever).
 */
void
CRingItemPipelineProcessor::disconnectSource()
{
    m_pSource->disconnect();
}
/**
 * connectSink
 *    This is supposed to connect the data sink with whatever is getting
 *    its ring items.  The same three cases apply as for data sources with the
 *    same suggested actions.
 */
void
CRingItemPipelineProcessor::connectSink()
{
    m_pSink->connect();
}
/**
 * disconnectSink
 *    Disconnects the sink from whatever its getting its ring items.
 */
void
CRingItemPipelineProcessor::disconnectSink()
{
    m_pSink->disconnect();
}
/**
 * getNextWorkItem
 *    The next work item is gotten from the data source.   In this case;
 *    getNextItem is called in the data source.  The expected return
 *    is a message with a message type understood by the ProcessingElement's
 *    main loop.  Any payload that message may have is normally
 *    a pointer to a dynamically allocated ring item.
 *
 * @return MessageType::Message - the work item the sourcre returned.
 */
MessageType::Message
CRingItemPipelineProcessor::getNextWorkItem()
{
    // Poll the sink for data and return that first -- could be a data
    // pull request.
    
    if (m_pSink->haveMessage()) {
        return m_pSink->getMessage();
    }
    return m_pSource->getNextItem();
}
/**
 * sendWorkItemToSink
 *    The various processing methods will produce a message that consists
 *    of a type and a list of pointers to dynamically allocated
 *    ring items.  These generally will be passed to us and we ask
 *    the sink to pass it on to a physical sink or process with which it's
 *    communicating.
 *    Sending the message transfers storage ownership over to the
 *    sender. 
 *
 *  @param msg - the message to send.
 *  @note after the messgae is sent, the dynamically allocated ring item(s)
 *        are assumed to have been deleted by the sink...or eventually by
 *        the entity with which the sink is communicating if this is a
 *        shared data/threaded system.
 */
void
CRingItemPipelineProcessor::sendWorkItemToSink(MessageType::Message& msg)
{
    m_pSink->send(msg);   
}
/**
 * onRegister
 *    A bit of strangeness in how this is handled (as well as onUnregister
 *    below).   The communication patterns we support are typically
 *    -  pure pipeline (one data source one data sink).
 *    -  Fan in - multiple data sources one data sink.
 *    -  Fan out - one data source, multiple data sinks.
 *
 *   A fan in/fan out can be created using two pipeline elements, one that
 *   fans in and the other that fans out.    One to many, and many to one
 *   communication patterns often require the many side of the communcation to
 *   register/unregister with the one.  In this way; at end of processing, the
 *   one side knows when the many sides have exited (when all have unregistered).
 *
 *   This method does not know the communication pattern being used.  Therefore
 *   it passes the request for registration on to both the source and the
 *   Only the source or sink which needs registration will pay any attention
 *   to this message.  Others will ignore it.
 *
 *  @param msg - the registration messgge.
 */
void
CRingItemPipelineProcessor::onRegister(MessageType::Message& reg)
{
    m_pSource->Register(reg);
    m_pSink->Register(reg);
}
/**
 * onUnregister
 *    Similar to onRegister but the unRegister method will be called.
 *
 * @param msg - the unregistration message.
 */
void
CRingItemPipelineProcessor::onUnregister(MessageType::Message& msg)
{
    m_pSource->unRegister(msg);
    m_pSink->unRegister(msg);
}
/**
 * processWorkItem
 *   Takes a work item that's been received from the data sourcdd
 *   and passes it on to the processor.  The resulting
 *   message is then sent to the sink.
 *
 * @param msg - the message encapsulated ring item to process.
 * @return MessageType::Message - message encapsulated
 *           ring items to pass to the sinks.  Note that if this is a filter,
 *           and the processor returned an IGNORE type the sink
 *           won't ever be called.
 */
void
CRingItemPipelineProcessor::processWorkItem(MessageType::Message& msg)
{
    MessageType::Message result = (*m_pProcessor)(msg);
    if (result.s_messageType != MessageType::IGNORE) {   // This assumes ignores have no payload.
        m_pSink->send(result);
    }
}
/**
 * onOtherMessageType
 *    Called if the source produced a message with a non-normal type.
 *    note that IGNORE is also not considered normal.  If the message
 *    type is not IGNORE, we're just going to pass it on to the sink.
 *
 * @param msg - the message received.
 */
void
CRingItemPipelineProcessor::onOtherMessageType(MessageType::Message& msg)
{
    if (msg.s_messageType != MessageType::IGNORE) {
        m_pSink->send(msg);
    }
}
/**
 * onEndItem
 *    End is treated like a normal work item because the
 *    processing element and the sink may need to clear some buffers etc.
 *
 * @param msg - the end message.
 */
void
CRingItemPipelineProcessor::onEndItem(MessageType::Message& msg)
{
    processWorkItem(msg);
}
/**
 * onExitRequested
 *   Also treated like a normal message.
 *
 * @param msg - the exit request message.
 */
void
CRingItemPipelineProcessor::onExitRequested(MessageType::Message& msg)
{
    processWorkItem(msg);
}
/**
 * onDataRequest
 *    A consumer wants to pull data from the sink.  We pass this oin to
 *    the sink's onPullRequest.
 * @param msg - the pull request message.
 */
void
CRingItemPipelineProcessor::onDataRequest(MessageType::Message& msg)
{
    m_pSink->onPullRequest(msg);
}

/**
 * connectAsSource
 *    This is normally called from a different thread than this processor.
 *    It's called when the data source object advertises as a server.
 *    The method then returns an open connection object to that server.
 *
 *  @return void* - actually a pointer to whatever connection object is used.
 *                  for communication.
 */
void*
CRingItemPipelineProcessor::connectAsSource()
{
    return m_pSource->connectSource();
}
/**
 * closeSource
 *    When the source object is a server this is called by clients in their
 *    threads to close the connection to that server.
 *
 * @param c - the connection gotten from connectAsSource.
 */
void
CRingItemPipelineProcessor::closeSource(void* c)
{
    m_pSource->closeSource(c);
}
/**
 * connectAsSink
 *    Connect to the sink as a client.  This is called in a client's
 *    thread to indicate interest in data.
 * @return void*  Some opaque handle to the connection object.
 */
void*
CRingItemPipelineProcessor::connectAsSink()
{
    return  m_pSink->connectSink();
}
/**
 * closeSink
 *    Called by a sink of our data to close the connection to our sink.
 *
 *  @param c - the connection object from connectAsSink
 */
void
CRingItemPipelineProcessor::closeSink(void* c)
{
    m_pSink->closeSink(c);
}
