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

/** @file:  CRingItemZMQSourceElement.cpp
 *  @brief: Implement the ring item ZMQ router.
 */
#include "CRingItemZMQSourceElement.h"
#include "CRingItemTransportFactory.h"
#include "CRingItemTransport.h"
#include "CZMQRouterTransport.h"
#include "CReceiver.h"

#include <CRingBuffer.h>


/**
 * constructor
 *    Create the receiver (data source) using a transport produced by  the
 *    CRingItemTrasnportFactory and hand a CZMQRouter as the transport.
 *
 *   @param ringUri - specifies the ring data source.
 *   @param routerUri - Specifies the URI of the ZMQ router.
 */
CRingItemZMQSourceElement::CRingItemZMQSourceElement(
    const char* ringUri, const char* routerUri
) :
    CDataSourceElement(*(new CReceiver(
        *CRingItemTransportFactory::createTransport(
            ringUri, CRingBuffer::consumer
        ))),
        *(new CZMQRouterTransport(routerUri))
    )
{}