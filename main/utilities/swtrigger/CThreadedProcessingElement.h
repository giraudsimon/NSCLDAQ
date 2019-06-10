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

/** @file:  CThreadedProcessingElement.h
 *  @brief: Defines a thread that encapsulates a processing element.
 */
#ifndef CTHREADEDPROCESSINGELEMENT_H
#define CTHREADEDPROCESSINGELEMENT_H
#include <Thread.h>
class CProcessingElement;

/**
 * @class CThreadedProcessingElement
 *     This class is just a thread that encapsulates a CProcessing element.
 *     The run method of the thread runs the processing element.
 *     On exit the processing element, the thread too exits.
 */
class CThreadedProcessingElement : public Thread
{
private:
    CProcessingElement* m_pProcessor;
public:
    CThreadedProcessingElement(CProcessingElement* processor);
    virtual ~CThreadedProcessingElement();
    
    virtual void run();
};


#endif