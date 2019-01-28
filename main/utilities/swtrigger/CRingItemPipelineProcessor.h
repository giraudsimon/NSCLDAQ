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

/** @file:  CRingItemPipelineProcess.h
 *  @brief: Takes one input ring item and prodduces one or more output items.
 *  
 */
#ifndef CRINGITEMPIPELINEPROCESSOR_H
#define CRINGITEMPIPELINEPROCESSOR_H
#include <list>

class CRingItem;

/**
 * @interface CRingTemplatePipelineProcessor
 *    This class is an abstract base class for processor elements that
 *    process a ring item input into one or more ring item outputs.
 *    See CRingItemProcessor for more information about how this is used.
 */

class CRingTemplatePipelineProcessor
{
public:
    CRingTemplatePipelineProcessor() {}
    virtual ~CRingTemplatePipelineProcessor() {}
    
    virtual std::list<CRingItem*> operator()(CRingItem*) = 0;
}

#endif