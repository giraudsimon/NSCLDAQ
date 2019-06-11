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

/** @file: CEventFilterApp.h
 *  @brief: Event filtering application class.
 */
#ifndef CEVENTFILTERAPP_H
#define CEVENTFILTERAPP_H
#include <stdint.h>

class CReceiver;
class CSender;

struct gengetopt_args_info;

/**
 * @class CEventFilterApp
 *    The application class for the event filter.
 *    -  We require that the body  header of an event be at least a uint32_t
 *       larger than the default header.
 *    -  We assume the uint32_t after the stock header comes from the
 *       classifier and is what we filter on.
 *
 *    Filtering is as follows:
 *    -   A mask is anded with the classification.
 *    -   The resulting value is compared with a specific value and
 *       if equal the event is considered accepted.
 *    -  Accepted events go to the accepted sink.
 *    -  If a rejected sink was defined, events that were not accepted
 *       are sent there instead otherwise they get dropped on the floor.
 */
class CEventFilterApp
{
private:
    CReceiver*   m_pDataSource;
    CSender*     m_pAcceptedSink;
    CSender*     m_pRejectedSink;
    
    uint32_t     m_mask;
    uint32_t     m_value;
public:
    CEventFilterApp(gengetopt_args_info& args);
    virtual ~CEventFilterApp();
    
    void operator()();
private:
    bool isAccepted(uint32_t value);
    uint32_t getClassification(void* pEvent);
    uint32_t type(void* pEvent);
    void send(CSender* pSender, void* pData);
};


#endif