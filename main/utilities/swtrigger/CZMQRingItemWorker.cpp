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

/** @file:  CZMQRingItemWorker.cpp
 *  @brief: Implement the CZMQRingItemWorker class.
 */
#include "CZMQRingItemWorker.h"
#include "CZMQDealerTransport.h"
#include "CSender.h"
#include "CProcessor.h"
/**
 * constructor
 *  @param routerUri - URI on which the router is listening.
 *  @param clientId  - Our client id for the Router/Dealer.
 *  @param sender    - References the actual sender.
 *  @param processor - pointer to the actual processing element.
 */
CZMQRingItemWorker::CZMQRingItemWorker(
    const char* routerUri, uint64_t clientId, CSender& sender,
    CProcessor* processor
) :
    CParallelWorker(
        *(new CZMQDealerTransport(routerUri, clientId)), sender
    ), m_pProcessor(processor)
{}

/**
 * process
 *    Process a message.  This is delegated to the
 *    processor.
 * @param pData - pointer to the message data.
 * @param nBytes - Size of the message data.
 */
void
CZMQRingItemWorker::process(void* pData, size_t nBytes)
{
    m_pProcessor->process(pData, nBytes, *getSink());
}
    
    

