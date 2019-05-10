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

/** @file:  CDispatcher.cpp
 *  @brief: Implement the dispatcher methods.
 */
#include "CDispatcher.h"
#include "CSender.h"
#include "CReceiver.h"


/**
 * constructor
 *    Just save the sender receiver.
 *
 *  @param pSource - receiver through which we get our work items.
 *  @param pSink   - Sender to which we send others our work items.
 */
CDispatcher::CDispatcher(CReceiver* pSource, CSender* pSink) :
    m_pSink(pSink), m_pSource(pSource)
{}

/**
 * destructor
 *    As indicated in the class commments, the source and sink are
 *    assumed to need deleting.
 */
CDispatcher::~CDispatcher()
{
    delete m_pSink;
    delete m_pSource;
}

/**
 * receiveWorkItem
 *    Get the next work item into storage that must be free(3)'d.
 */
void
CDispatcher::receiveWorkItem(void** pData, size_t& size)
{
    return m_pSource->getMessage(pData, size);
}
/**
 * sendWorkItem
 *    Sends a segmented work item to whomever cares about it.
 *
 * @param items - array of item descriptors.
 * @param nItems  - Number of items.
 */
void
CDispatcher::sendWorkItem(iovec* items, int nItems)
{
    m_pSink->sendMessage(items, nItems);
}
/**
 * sendWorkItem
 *    Send a work item that's a single idivisible thing.
 *
 *  @param pData - pointer to the data to send.
 *  @param nBytes - number of bytes of data.
 */
void
CDispatcher::sendWorkItem(void* pItem, size_t nBytes)
{
    m_pSink->sendMessage(pItem, nBytes);
}
/**
 *  end
 *  Tell the sink there's no more data.
 */
void
CDispatcher::end()
{
  m_pSink->end();
}
