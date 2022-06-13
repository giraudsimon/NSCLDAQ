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

/** @file:  CEventFilterApp.cpp
 *  @brief: Implement the CEventFilter class.
 */
#include "CEventFilterApp.h"

#include "CSender.h"
#include "CReceiver.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemTransport.h"
#include "CTransport.h"
#include <DataFormat.h>

#include "eventfilterflags.h"

#include <stdlib.h>
#include <stdexcept>

/**
 * constructor
 *    Use the parsed parameters to figure out how to intialize
 *    the data.
 * @param args - parsed command line arguments.
 */
CEventFilterApp::CEventFilterApp(gengetopt_args_info& args) :
    m_pDataSource(nullptr), m_pAcceptedSink(nullptr),
    m_pRejectedSink(nullptr), m_mask(0), m_value(0),
    m_sample(-1), m_rejectCount(0)
{
    std::string sourceUri = args.source_arg; // mandatory.,
    CRingItemTransport *pSourceXport =
        CRingItemTransportFactory::createTransport(
            sourceUri.c_str(), CRingBuffer::consumer
        );
    m_pDataSource = new CReceiver(*pSourceXport);
    
    std::string acceptedUri = args.accepted_sink_arg;
    CRingItemTransport* pASinkXport =
        CRingItemTransportFactory::createTransport(
            acceptedUri.c_str(), CRingBuffer::producer
        );
    m_pAcceptedSink = new CSender(*pASinkXport);
    
    if (args.rejected_sink_given) {
        CRingItemTransport* pRSinkXport =
            CRingItemTransportFactory::createTransport(
                args.rejected_sink_arg ,CRingBuffer::producer
            );
        m_pRejectedSink = new CSender(*pRSinkXport);
    }
    
    m_mask = args.mask_arg;
    m_value= args.value_arg;
    m_sample= args.sample_arg;
}
/**
 * destructor
 *    The only dynamic things are the senders and receivers.
 */
CEventFilterApp::~CEventFilterApp()
{
    delete m_pDataSource;
    delete m_pAcceptedSink;
    delete m_pRejectedSink;
}

/**
 * operator()
 *    The main flow of control...
 *    Each message from the receiver is a ring item.
 *    If it's not a physics item pass it on to all sinks.
 *    If it is a physics item then see if its accepted.
 *    If so send it to the accepted sinkd
 *    otherwise, if it's defined, send it to the rejected sink.
 */
void
CEventFilterApp::operator()()
{
    void*  pMsg;
    size_t nBytes;
    
    do {
        m_pDataSource->getMessage(&pMsg, nBytes);
        if (nBytes) {
            bool accepted(true);       // Non physics items need to go
            bool rejected(true);       // to both sinks.
            if (type(pMsg) == PHYSICS_EVENT) {
                uint32_t c = getClassification(pMsg);
                if (isAccepted(c)) {
                    accepted = true;
                    rejected = false;
                } else {
		    rejected = true;
		    m_rejectCount++;
		    if (m_sample > 0) {
		      if ((m_rejectCount % m_sample) == 0) {
			accepted = true; // downscaled acceptance.
		      }
		    } else {
		      accepted = false;
		    }
                }
            }
            if (accepted) {
                send(m_pAcceptedSink, pMsg);
            }
            if (rejected && m_pRejectedSink) {
                send(m_pRejectedSink, pMsg);
            }
            free(pMsg);
        }
    } while (nBytes > 0);
    
    // Send data end messages to the sinks.  That should make them
    // flush their buffers:
    
    m_pAcceptedSink->end();
    if (m_pRejectedSink) {
        m_pRejectedSink->end();
    }
}
/**
 * isAccepted
 *
 *    @return true if the parameter is an acceptable classification
 */
bool
CEventFilterApp::isAccepted(uint32_t value)
{
    return (value & m_mask) == m_value;
}
/**
 * getClassification
 *
 *    Given a ring item that's known to be a physics event;
 *    return the value the classifier gave it.
 *
 *  @param pEvent - actually a pRingItem.
 *  @return uint32_t - the classfication value.
 *  @throw std::invalid_argument if there's no body header or
 *           there'a s body header but it's not big enough to have
 *           a classification word.
 */
uint32_t
CEventFilterApp::getClassification(void* pEvent)
{
    pRingItem pItem = static_cast<pRingItem>(pEvent);
    
    // Be sure we have a body header:
    
    
    if (!hasBodyHeader(pItem)) {
        throw std::invalid_argument(
            "Physcics event has no body header"
        );
    }
    pBodyHeader pBh = static_cast<pBodyHeader>(bodyHeader(pItem));
    uint32_t bHeaderSize = pBh->s_size;
    
    // sizeof(BodyHeader) ok here because we want to know if the actual
    // body header is bigger than this...
    
    int extra = static_cast<int>(bHeaderSize) - sizeof(BodyHeader);
    if (extra < sizeof(uint32_t)) {
        throw std::invalid_argument(
            "Physics event body header does not have a classification."
        );
    }
    
    pBh++;                      // Points to the classification.
    uint32_t* pClassification = reinterpret_cast<uint32_t*>(pBh);
    return *pClassification;
}
/**
 * type
 *    @param pEvent - pointer to a ring item.
 *    @return uint32_t the ring item type.
 */
uint32_t
CEventFilterApp::type(void* pEvent)
{
    pRingItem ph = static_cast<pRingItem>(pEvent);
    return itemType(ph);
}
/**
 * send
 *    Send an item to some sink.
 *
 *  @param pSender - Pointer to a CSender encapsulating the transport
 *                   to use.
 *  @param pData   - Pointer to the data to send (ring item).
 */
void
CEventFilterApp::send(CSender* pSender, void* pData)
{
    pRingItem ph = static_cast<pRingItem>(pData);
    pSender->sendMessage(pData, itemSize(ph));
}
