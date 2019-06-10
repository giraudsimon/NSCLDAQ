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

/** @file:  CZMQRingItemThreadedWorker.h
 *  @brief: Encapsualte a CZMQRingItemWorker in an execution thread.
 */
#ifndef CZMQRINGITEMTHREADEDWORKER_H 
#define CZMQRINGITEMTHREADEDWORKER_H

#include "CThreadedProcessingElement.h"
class CProcessor;
class CSender;

/**
 *  A threaded worker that encapsulates a processing element
 *  that knows how to get data from a ZMQ Router (via a ZMQ dealer),
 *  do some processing on it and send it on to the next stage of the
 *  computation.
 *
 */
class CZMQRingItemThreadedWorker : public CThreadedProcessingElement
{
public:
    CZMQRingItemThreadedWorker(
        const char* routerUri, uint64_t clientId, CSender& sender,
        CProcessor* processor
    );
    virtual ~CZMQRingItemThreadedWorker() {}
};

#endif