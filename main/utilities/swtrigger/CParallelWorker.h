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

/** @file:  CParallelWorker.h
 *  @brief: Abstract base class for a worker that is parallelized via fanout
 */
#ifndef CPARALLELWORKER_H
#define CPARALLELWORKER_H
#include "CProcessingElement.h"
#include <stdint.h>

class CReceiver;
class CFanoutClientTransport;
class CSender;

/**
 * @class CParallelWorker
 *    This is a base class for a worker processing element.  Workers
 *    are elements that receive data from a fanout data transport and
 *    perform some (presumably expensive) processing and then send some
 *    derived data on to some next element.
 *
 */
class CParallelWorker : public CProcessingElement
{
private:
    CFanoutClientTransport*  m_pReceiverTransport;
    CReceiver*               m_pDataSource;
    CSender*                 m_pDataSink;
public:
    CParallelWorker(CFanoutClientTransport& fanin, CSender& sink);
    CParallelWorker(
        CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId
    );
    virtual ~CParallelWorker();
    
    virtual void operator()();

protected:
    CSender* getSink();
   
};

#endif