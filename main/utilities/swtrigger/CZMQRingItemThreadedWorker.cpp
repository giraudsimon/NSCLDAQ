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

/** @file:  CZMQRingItemThreadedWorker.cpp
 *  @brief: Implement the CZMQRingItemThreadedWorker class.
 */

#include "CZMQRingItemThreadedWorker.h"
#include "CProcessor.h"
#include "CZMQRingItemWorker.h"

/**
 * constructor
 *   @param routerUri - URI of ZMQ router we get data from.
 *   @param clientId  - Unique client id.
 *   @param processor - processing object that does the work on ring items.
 */
CZMQRingItemThreadedWorker::CZMQRingItemThreadedWorker(
    const char* routerUri, uint64_t clientId, CSender& sender,
    CProcessor* processor
) :
    CThreadedProcessingElement(
        new CZMQRingItemWorker(routerUri, clientId, sender, processor)
    )
{}