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


#include <stdint.h>
#include <CLoggingDataSink.h>
#include <CRingItemFactory.h>
#include <CRingItem.h>
#include <DataFormat.h>

void CLoggingDataSink::putItem(const CRingItem& item)
{
  m_log.push_back(item); 
}

void CLoggingDataSink::put(const void* pData, size_t nBytes)
{
  const uint8_t* iter = reinterpret_cast<const uint8_t*>(pData);
  const uint8_t* end   = iter + nBytes;

  while (iter != end) {
    const RingItemHeader* header = reinterpret_cast<const RingItemHeader*>(iter);
    
    CRingItem* item = CRingItemFactory::createRingItem(iter);
    m_log.push_back(*item);
    delete item;

    iter +=
      itemSize(reinterpret_cast<const RingItem*>(header));;
  }
}
