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

/** @file:  CDataSourceElement.h
 *  @brief: Defines a data sourcez for a fanned out computation.
 *  
 */
#ifndef CDATASOURCEELEMENT_H
#define CDATASOURCEELEMENT_H

#include "CProcessingElement.h"

class CFanoutTransport;
class CReceiver;
class CSender;

/**
 * @class CDataSourceElement
 *   The purpose of this class is to take data from a sourcde and fan it
 *   out to fan it out to worker tasks.  The operator(), therefore, gets data
 *   from some Receiver and, without any actual processing, sends it to
 *   a Sender with a Fanout underlying transport.
 */
class CDataSourceElement : public CProcessingElement
{
private:
    CFanoutTransport* m_pSenderTransport;
    CReceiver*        m_pDataSource;
    CSender*          m_pFanout;
public:
    CDataSourceElement(CReceiver& source, CFanoutTransport& fanout);
    virtual ~CDataSourceElement();
    
    virtual void operator()();
    virtual void process(void* pData, size_t nBytes);
protected:
    CSender* getSender() {return m_pFanout;}
    CReceiver* getSource() { return m_pDataSource; }
};

#endif