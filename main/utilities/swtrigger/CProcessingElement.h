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

/** @file:  CProcessingElement.h
 *  @brief: Abstract base class for processing elements.
 */

#ifndef CPROCESSINGELEMENT_H
#define CPROCESSINGELEMENT_H
#include <stddef.h>

/**
 * @class CProcessingElement
 *   This is the abstract base class of a generic processing element.
 *   The element has a flow of control descdribed by its
 *   operator() and something that does processing described by
 *   process().   The operator() obtains data for process() to
 *   process, if processing results in any data process must arrange to
 *   send it somewhere.
 */
class CProcessingElement
{
public:
    virtual ~CProcessingElement();
    
    virtual void operator()()  = 0;
    virtual void process(void* pData, size_t nBytes) = 0;
};

#endif