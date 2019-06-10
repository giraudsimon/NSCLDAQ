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

/** @file:  CDataSinkElement.h
 *  @brief: Forwards data from a sender to a receiver.
 */

#ifndef CDATASINKELEMENT_H
#define CDATASINKELEMENT_H

#include "CProcessingElement.h"
#include <stddef.h>

class CSender;
class CReceiver;

/**
 * @class CDatasinkElement
 *    This element is usually used towards the end of data processing.
 *    Data from some receiver (usually application internal) is just
 *    forwarded to some sender (usually using a transport to get it
 *    out of the application e.g. to file or a ringbuffer if online).
 */
class CDataSinkElement : public CProcessingElement
{
private:
    CReceiver* m_pSource;
    CSender*   m_pSink;
public:
    CDataSinkElement(CReceiver& src, CSender& sink);
    virtual ~CDataSinkElement();
    
    virtual void operator()();
    virtual void process(void* pData, size_t nBytes);
protected:    
    CSender* getSink() { return m_pSink;}   // For derived classses.
};


#endif