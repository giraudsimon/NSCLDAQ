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
 #ifdef SWIG
 %module CCCUSBReadoutList
 %{
 #include "CCCUSBReadoutList.h"
 %}

 #endif
 #ifndef __CCUSBREADOUTLIST_H
 #define __CCUSBREADOUTLIST_H

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
   CCCUSBReadoutList(const std::vector<uint16_t>& list) : m_list(list) {}
   CCCUSBReadoutList(const CCCUSBReadoutList& rhs);
   CCCUSBReadoutList& operator=(const CCCUSBReadoutList& rhs);

   // Operations on the list as a whole.
 public:

   std::vector<uint16_t> get()     const;
   size_t                size()    const;
   void                  clear();
   void                  append(const CCCUSBReadoutList& rdolist);

   // Adding elements to the list:

 public:
   // Single shot operations:

   virtual void addWrite16(int n, int a, int f, uint16_t data);
   virtual void addWrite24(int n, int a, int f, uint32_t data);

//   void addWrite16(int n, int a, int f, int data) { /* swig */
//     addWrite16(n,a,f, (uint16_t)data);
//   }
//   void addWrite24(int n, int a, int f, int data) { /* swig */
//     addWrite24(n,a,f, (uint32_t)data);
//   }

   virtual void addRead16(int n, int a, int f, bool lamWait=false);
   virtual void addRead24(int n, int a, int f, bool lamWait=false);

   virtual void addControl(int n, int a, int f);


   // Block transfer operations:

   virtual void addQStop(int n, int a, int f, uint16_t max, bool lamWait = false);
   virtual void addQStop24(int n, int a, int f, uint16_t max, bool lamWait = false);
   void addQStop(int n, int a, int f, int max, bool lamWait = false) { /* swig */
     addQStop(n,a,f, (uint16_t)max, lamWait);
   }
   void addQStop24(int n, int a , int f, int max, bool lamWait = false) { /* swig */
     addQStop24(n,a,f, (uint16_t)max, lamWait);
   }


   virtual void addQScan(int n, int a, int f, uint16_t max, bool lamWait = false);
   void addQScan(int n, int a, int f, int max, bool lamWait = false) { /* swig */
     addQScan(n,a,f,(uint16_t)max, lamWait);
   }

   virtual void addRepeat(int n, int a, int f,uint16_t count, bool lamWait=false);
   void addRepeat(int n, int a, int f, int count, bool lamWait=false) {
     addRepeat(n,a,f, (uint16_t) count, lamWait);
   }

   // Other:

   virtual void addMarker(uint16_t value);    // Add literal value to event.
   void addMarker(int value) {	     /* swig */
     addMarker((uint16_t)value);
   }

   virtual void addAddressPatternRead16(int n, int a, int f, bool lamWait=false);
//   void addAddressPatternRead24(int n, int a, int f, bool lamWait=false);

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

inline std::vector<uint16_t>* vecuint16_create(int size)
{
  return new std::vector<uint16_t>(size);
}

inline void vecuint16_pushback(std::vector<uint16_t>* vec, uint16_t val)
{
  vec->push_back(val); 
}

inline void vecuint16_set(std::vector<uint16_t>* vec, int index, uint16_t val)
{
  if (index >= vec->size()) {
    vec->resize(index+1);
  }

  (*vec)[index] = val;
}

inline std::vector<uint16_t>& vecuint16_ptr2ref(std::vector<uint16_t>* vec)
{
  return *vec;
}


#endif
