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

#ifndef __CCUSBREADOUTLIST_H
#define __CCUSBREADOUTLIST_H


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif


/*!
  In general, the CC-USB will be used by building list of CAMAC operations (stacks in the
  manual's parlance).  These lists get downloaded into the CC-USB and are then
  executed in response to trigger conditions.   In this way the host computer is not 
  involved in event to event interactions with the CC-USB, which improves performance
  tremendously.

  This class provides functions that build up a list which can then be passed
  to CCCUSB::executeList for immediate execution or CCCUSB::loadList to be loaded as
  either event or scaler lists in autonomous data taking mode.

  \note There are bits and pieces of CC-USB functionality that are not yet suppported
        by this class.

*/
class CCCUSBReadoutList
{
 private:
  std::vector<uint16_t> m_list;	

  // Just really need a copy constructor and an assignment

public:
  CCCUSBReadoutList() {}
  CCCUSBReadoutList(const CCCUSBReadoutList& rhs);
  CCCUSBReadoutList& operator=(const CCCUSBReadoutList& rhs);

  // Operations on the list as a whole.
public:

  std::vector<uint16_t> get()     const;
  size_t                size()    const;
  void                  clear();

  // Adding elements to the list:
  
public:
  // Single shot operations:

  void addWrite16(int n, int a, int f, uint16_t data);
  void addWrite24(int n, int a, int f, uint32_t data);

  void addRead16(int n, int a, int f, bool lamWait=false);
  void addRead24(int n, int a, int f, bool lamWait=false);

  void addControl(int n, int a, int f);


  // Block transfer operations:

  void addQStop(int n, int a, int f, uint16_t max, bool lamWait = false);
  void addQScan(int n, int a, int f, uint16_t max, bool lamWait = false);
  void addRepeat(int n, int a, int f,uint16_t count, bool lamWait=false);

  // 
private:
  static std::string messageWithValue(const char* format, int value);
  static uint16_t NAF(int n, int a, int f, bool isLong=false);
  static bool validWrite(int n, int a, int f, std::string& msg);
  static bool validRead(int n, int a,  int f, std::string& msg);
  static bool validControl(int n, int a, int f, std::string& msg);
  static bool validSlotAndSubaddress(int n, int a, std::string& msg);
  static bool validSlot(int n);
  static bool validSubaddress(int a);


  
};

#endif