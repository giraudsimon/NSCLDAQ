/*
   This software is Copyright by the Board of Trustees of Michigan
   State University (c) Copyright 2005.

   You may use this software under the terms of the GNU public license
   (GPL).  The terms of this license are described at:

http://www.gnu.org/licenses/gpl.txt

Author:
Ron Fox
NSCL
Michigan State University
East Lansing, MI 48824-1321
*/

#ifndef __LOGGINGREADOUTLIST_H
#define __LOGGINGREADOUTLIST_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#include "CCCUSBReadoutList.h"

class CLoggingReadoutList : public CCCUSBReadoutList
{
  private:
    std::vector<std::string> m_log;

    // Just really need a copy constructor and an assignment

  public:
    CLoggingReadoutList() : CCCUSBReadoutList(), m_log() {}
    CLoggingReadoutList(const CLoggingReadoutList& rhs);
    CLoggingReadoutList& operator=(const CLoggingReadoutList& rhs);

    // Operations on the list as a whole.
  public:

    std::vector<std::string> getLog() const { return m_log;}
    void clearLog() { m_log.clear();}

    void append(const CLoggingReadoutList& list);

    // Adding elements to the list:

  public:
    // Single shot operations:

    virtual void addWrite16(int n, int a, int f, uint16_t data);
    virtual void addWrite24(int n, int a, int f, uint32_t data);


    virtual void addRead16(int n, int a, int f, bool lamWait=false);
    virtual void addRead24(int n, int a, int f, bool lamWait=false);

    virtual void addControl(int n, int a, int f);


    // Block transfer operations:

    virtual void addQStop(int n, int a, int f, uint16_t max, bool lamWait = false);
    virtual void addQStop24(int n, int a, int f, uint16_t max, bool lamWait = false);

    virtual void addQScan(int n, int a, int f, uint16_t max, bool lamWait = false);

    virtual void addRepeat(int n, int a, int f,uint16_t count, bool lamWait=false);

    // Other:

    virtual void addMarker(uint16_t value);    // Add literal value to event.

    virtual void addAddressPatternRead16(int n, int a, int f, bool lamWait=false);

};

#endif
