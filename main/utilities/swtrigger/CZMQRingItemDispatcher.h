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

/** @file:  CZMQRingItemDispatcher.h
 *  @brief: ring item dispatcher to ZMQrouter.
 */
#ifndef CZMQRINGITEMDISPATCHER_H
#define CZMQRINGITEMDISPATCHER_H

#include "CRingItemDispatcher.h"

class CZMQRingItemDispatcher : public CRingItemDispatcher
{
public:
    CZMQRingItemDispatcher(const char* ringUri, const char* routerUri);
    virtual ~CZMQRingItemDispatcher() {}
};

#endif