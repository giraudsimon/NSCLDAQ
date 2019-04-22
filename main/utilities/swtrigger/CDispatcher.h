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

/** @file:  CDispatcher.h
 *  @brief: Get data from somewhere and send it on to its next destination.
 */
#ifndef CDISPATCHER_H
#define CDISPATCHER_H

#include <stddef.h>
#include <sys/uio.h>

class CSender;
class CReceiver;

/**
 * @class CDispatcher
 *    This class is normally included in a worker thread class to do the
 *    messaging. A typical worker gets data (a work item) from some data source
 *    (CReceiver) on their associated transports does something to the data
 *    and then sends output data to some sink (CSender).
 *
 *    @note The sender and receiver objects are assumed to have been
 *          dynamically created by the client.  Thus on destruction theyu
 *          are deleted.
 */
class CDispatcher {
private:
    CSender*   m_pSink;
    CReceiver* m_pSource;
public:
    CDispatcher(CReceiver* pSource, CSender* pSink);
    virtual ~CDispatcher();
    
    void receiveWorkItem(void** pData, size_t& size);
    void sendWorkItem(iovec* items, int nItems);
    void sendWorkItem(void* pItem, size_t nBytes);
};

#endif