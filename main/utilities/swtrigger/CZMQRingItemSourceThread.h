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

/** @file:  CZMQRingItemSourceThread.h
 *  @brief: Encapsulates the CRingITemZMQSourceElement in a thread.
 */
#ifndef CZMQRINGITEMSOURCETHREAD_H
#include "CThreadedProcessingElement.h"

/**
 * @class CZMQRingItemSourceThread
 *    This encapsulates a CRingTiemZMQSourceElement in a thread.
 *    This class is used when composing a multicore (threaded)
 *    parallel computation that needs to accept ring items from a
 *    data source (online or offline) and distribute it to
 *    workers for processing.
 *
 *    This object can also be used in hybrid processing where a source of
 *    ring items is then sent via the ZMQ Router/Dealer pattern to
 *    processes across the network.  The Router URL will differentiate that.
 *    Use inproc: uris for threaded computation and tcp: uris for
 *    distributed computing.
 */
class CZMQRingItemSourceThread : public CThreadedProcessingElement
{
public:
    CZMQRingItemSourceThread(const char* ringUri, const char* routerUri);
    virtual ~CZMQRingItemSourceThread() {}
};


#endif