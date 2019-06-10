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

/** @file:  CRingItemDispatcher.cpp
 *  @brief: Implement the CRingItemDispatcher class.
 */
#include "CRingItemDispatcher.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemTransport.h"
#include "CReceiver.h"

/**
 * constructor
 *    - Create the receiver using the factory to create its transport.
 *    - The sink is passed in to us and relayed to the CDispatcher constructor.
 *
 * @param uri - URI of the ring data source,.
 * @paran pSink - Pointer to the dispatch sink.
 */
CRingItemDispatcher::CRingItemDispatcher(const char* uri, CSender* pSink) :
    CDispatcher(new CReceiver(
        *CRingItemTransportFactory::createTransport(uri, CRingBuffer::consumer)),
        pSink
    ) {}
    
