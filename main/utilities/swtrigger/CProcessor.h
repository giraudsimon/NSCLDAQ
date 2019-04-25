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

/** @file:  CProcessor.h
 *  @brief: Extracts processing from e.g. CZMQRingItemWorker classes.
 */
#ifndef CPROCESSOR_H
#define CPROCESSOR_H
#include <stddef.h>

class CSender;

/**
 * @class CProcessor
 *    This class can be used by ProcessingElements that want to delegate
 *    processing to a communications neutral object.  It is an abstract
 *    base class.  Users should subclass it with a concrete implementation
 *    and hand it to e.g. the constructor of CZMQRingItemWorker.
 *    The process method will then be called by the process
 *    method of the encapsulating worker.  The goal is that users
 *    of the processing framework just have to write CProcessors and
 *    wrap them in the appropriate processing elements which get data
 *    for them and provide them with a message sending object
 *    to allow them to send data to the next element of the processing
 *    fabric.
 */
class CProcessor
{
public:
    virtual ~CProcessor() {}           // To support destructor chaining.
    
    virtual void process(void* pData, size_t nBytes, CSender& sender) = 0;
};

#endif