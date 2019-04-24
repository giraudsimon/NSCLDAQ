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

/** @file:  CRingItemDispatcher.h
 *  @brief: Dispatche rin gitems gotten from some source to some sink.
 *  
 */
#ifndef CRINGITEMDISPATCHER_H
#define CRINGITEMDISPATCHER_H
#include "CDispatcher.h"

class CSender;

/**
 * @class CRingItemDispatcher
 *     A class that dispatches ring items to some sink.
 *     Normally, but not necessarily, the sink is a fanout such as
 *     a CZMQRouter sink.
 *     The ring source transport is gotten from a CRingItemTransportFactory.
 */
class CRingItemDispatcher : public CDispatcher
{
public:
    CRingItemDispatcher(const char* uri, CSender* pSink);
    virtual ~CRingItemDispatcher() {}
};
#endif