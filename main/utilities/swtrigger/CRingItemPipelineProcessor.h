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

/** @file:  CRingItemPipelineProcess.h
 *  @brief: Takes one input ring item and prodduces one or more output items.
 *  
 */
#ifndef CRINGITEMPIPELINEPROCESSOR_H
#define CRINGITEMPIPELINEPROCESSOR_H
#include <ProcessingElement.h>
#include <list>

class CRingItemDataSource;
class CRingItemDataSink;
class CRingItemProcessor;
/**
 * @class CRingItemPipelineProcessor
 *   This is an base class for processing element that take ring items
 *   as input and deliver modified ring items as output.  No
 *   assumptions are made about conservation of ring items. Processing
 *   can remove or add new ring items as well as transform ring items or
 *   event pass them through with no modification.
 *   Three helpers are used:
 *   -   CRingItemDataSource is associated with fetching ring items from the
 *                       previous stage of the pipeline as well as
 *                       providing, as needed mechanisms for that
 *                       previous stage to form a connection with us.
 *   -   CRingItemDataSink - disposes of ring items to the next stage of the
 *                       pipeline.  It also provides mechanisms for
 *                       sink processes to connect to us, or may be written
 *                       to actively connect to the next stage of the pipeline.
 *   - CRingItemProcessor - accepts a single ring item and, in response emits a
 *                    (possibly empty) list of output ring items.
 *  @note that the assumption is that ring items produced by CRingSource
 *        are dynamically allocated, as are ring items produced by
 *        CRingProcessor.  Thus, any ring items not directly passed through
 *        must  be deleted by the CRingProcessor and, CRingDataSink will
 *        delete any ring items it is given to pass to consumer(s).
 *  @note A null CRingDataSource, uses a null pointer to a ring item to flag
 *        the end of data on its underlying source of ring items.
 *        
 */

class CRingItemPipelineProcessor : public ProcessingElement
{
private:
    CRingItemDataSource* m_pSource;
    CRingItemDataSink*  m_pSink;
    CRingItemProcessor* m_pProcessor;
public:
    CRingItemPipelineProcessor(
        const char* name,
        CRingItemDataSource* src, CRingItemDataSink* sink,
        CRingItemProcessor* prc
    );
    virtual ~CRingItemPipelineProcessor();
protected:    
    virtual void connectSource();
    virtual void connectSink();
    virtual void disconnectSource();
    virtual void disconnectSink();
    virtual MessageType::Message getNextWorkItem();
    virtual void sendWorkItemToSink(MessageType::Message& workItem);
        // Processing:
    
    virtual void onRegister(MessageType::Message& reg);
    virtual void onUnregister(MessageType::Message& reg);
    virtual void processWorkItem(MessageType::Message& item);
    virtual void onOtherMessageType(MessageType::Message& item);
    virtual void onEndItem(MessageType::Message& endItem);
    virtual void onExitRequested(MessageType::Message& item);
    
    virtual void onDataRequest(MessageType::Message& item);      // Override for explicit pull protocols.

    virtual void*  connectAsSource();
    virtual void   closeSource(void* c);
    virtual void*  connectAsSink();
    virtual void   closeSink(void* c);    
};

#endif