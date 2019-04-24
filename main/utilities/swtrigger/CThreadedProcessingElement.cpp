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

/** @file:  CThreadedProcessingElement.cpp
 *  @brief: Implement the threaded processing element.
 */
#include "CThreadedProcessingElement.h"
#include "CProcessingElement.h"


/**
 * constructor
 *    Retain a pointer to the processing element.
 *    The processing element is assumed to have been dynamically
 *    allocated and will be deleted when/if we are destroyed.
 *
 *  @param processor - pointer to the encapsulated processor.
 */
CThreadedProcessingElement::CThreadedProcessingElement(
    CProcessingElement* processor
) : m_pProcessor(processor)
{}

/**
 * destructor
 *    -  Very important the caller knows the thread has exited before calling
 *       this.
 *    -  The processor is deleted.
 */
CThreadedProcessingElement::~CThreadedProcessingElement()
{
    delete m_pProcessor;
}
/**
 * run
 *   Thread entry point - just pass control to the processor.
 */
void
CThreadedProcessingElement::run()
{
    (*m_pProcessor)();
    
}