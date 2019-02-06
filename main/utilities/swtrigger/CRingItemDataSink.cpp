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

/** @file:  CRingItemDataSink.cpp
 *  @brief: Provide linkables for cons and des of CRingItemDataSink ABC
 */
#include "CRingItemDataSink.h"

CRingItemDataSink::CRingItemDataSink() {}
CRingItemDataSink::~CRingItemDataSink() {}

// Default is no message (not a pull so can't receive from sink peer).

MessageType::Message
CRingItemDataSink::getMessage()
{
    return CRingItemDataSink::requestData(nullptr); // Already does what we want.
}

// Default is for non pull sinks - -return an ignore message:

MessageType::Message
CRingItemDataSink::requestData(void* c)
{
    MessageType::Message result;
    result.s_messageType = MessageType::IGNORE;
    return result;    
}
