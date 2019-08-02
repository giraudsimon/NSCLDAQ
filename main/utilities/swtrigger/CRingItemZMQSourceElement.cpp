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
#include "CSender.h"

#include <CRingBuffer.h>
#include <DataFormat.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <array>
#include <fragment.h>

/**
 * constructor
 *    Create the receiver (data source) using a transport produced by  the
 *    CRingItemTrasnportFactory and hand a CZMQRouter as the transport.
 *
 *
 *   @param ringUri - specifies the ring data source.
 *   @param routerUri - Specifies the URI of the ZMQ router.
 *   @param chunkSize - Number of ring items that are sent in each message.
 *
 *   @note each ring item is sent preceded by a 64 bit timestamp.  Where possible,
 *   this timestamp is taken from the body header.  Otherwise a synthetic timestamp
 *   is generated that preserves ordering.
 */
CRingItemZMQSourceElement::CRingItemZMQSourceElement(
    const char* ringUri, const char* routerUri,
    size_t chunkSize
) :
    CRingItemBlockSourceElement(
        rintUri, *(new CZMQRouterTransport(routerUri)),
        chunkSize
    )
{}
