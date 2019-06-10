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


#ifndef CLOGGINGDATASINK_H
#define CLOGGINGDATASINK_H

#include <vector>
#include <CDataSink.h>

class CRingItem;

class CLoggingDataSink : public CDataSink
{
  private:
    std::vector<CRingItem> m_log;

  public:
    virtual void putItem(const CRingItem& item);
    virtual void put(const void* pData, size_t nBytes);

    std::vector<CRingItem> getLog() { return m_log; }
};

#endif
