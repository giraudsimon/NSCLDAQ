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

/** @file:  CZMQRingItemSourceThread.cpp
 *  @brief: Encapsulate a CRingItemZMQProcessingElement in a thread.
 */

#include "CZMQRingItemSourceThread.h"
#include "CRingItemZMQSourceElement.h"

/**
 * constructor
 *    Wrap the processing element in our thread.
 *    
 *    @param ringUri - the URI from which ring items are taken.
 *    @param routerUri - the URI to which ring items are routed.
 */
CZMQRingItemSourceThread::CZMQRingItemSourceThread(
    const char* ringUri, const char* routerUri
) :
    CThreadedProcessingElement(
        new CRingItemZMQSourceElement(ringUri, routerUri)
    )
{}