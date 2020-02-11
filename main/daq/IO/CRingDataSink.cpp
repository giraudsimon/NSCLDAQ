/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include <CRingDataSink.h>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <CDataSinkException.h>


CRingDataSink::CRingDataSink(std::string ringName)
  : m_pRing(0),
    m_ringName(ringName)
{
  openRing();
}

CRingDataSink::~CRingDataSink()
{
  delete m_pRing;
  m_pRing=0;
}

/**
 * putItem
 *    Puts a ring item in the sink.
 * @param item - Reference to the item top ut.
 */
void CRingDataSink::putItem(const CRingItem& item)
{
  put(item.getItemPointer(), item.size());

}
/**
 * put
 *   Puts arbitrary data to the sink (ring).
 *
 *   @param pData - Pointer to the buffer holding the data.
 *   @param nBytes - Number of bytes to put.
 */
void CRingDataSink::put(const void* pData, size_t nBytes)
{
    
    // TODO:  This is the theoretical correct way to do this as
    //        there is a (very long) timeout on the put if none is
    //        specified (years)..should at some point check that all
    //        'reliable' puts are done like this-- retrying until success.
    //
    while(!m_pRing->put(pData, nBytes))
        ;
}


void CRingDataSink::openRing()
{
  // try to open the ring as a producer...
  // check if the ring exists... if it does just
  // try to attach to it as a producer. If a producer
  // exists, this will throw a CErrnoException of errno=EACCES
  if (CRingBuffer::isRing(m_ringName)) {
    m_pRing = new CRingBuffer(m_ringName,CRingBuffer::producer);
  } else {
    // If we are here, no ring exists already with the desired
    // name so we can create and produce
    m_pRing = CRingBuffer::createAndProduce(m_ringName);
  }
  if (m_pRing==0) {

    throw CDataSinkException("CRingDataSink::openRing()","Failed to open specified ring");
  }
}
