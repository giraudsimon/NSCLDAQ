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

#ifndef __CC1205_H
#define __CC1205_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

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

// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;


/*!
  Support for the CAEN C1205 ADC.  This module provides the capability to
  give data in more than one range (sort of like the VME V1785 ADC).
  Specifically three ranges are supported and set by providing individual range
  threshold values.
  
  Options are:

\verbatim

Name          Type           Initial         Meaning
-slot         int            0            Slot module is installed in.
-id           uint8_t        0            Id in marker word and in module Id field.
-rangemode    enum          auto          one of all, auto, sparse (bits 9-10 of cr).
-usepedestals bool          true          Enable pedestal subtraction.
-hires        bool          true          Enable 12 bit resolution.
-thresholds   int[16]       16*0          Thresholds for each of the 16 channels.
-lopedestals  int[16]       16*0          Thresholds that define the low range.
-midpedestals int[16]       16*0          Thresholds that define the midrange.
-hipedestals  int[16]       16*0          Thresholds taht define the high range.


\endverbatim

I believe the thresholds are used as follows.  Any conversion between the
low pedestal value and the mid pedestals are low range between the low and high mid 
range and above the hi pedestals, are high range values.

\note the -lo/-mid/-hipedestal values are in the range -8190-8191 
(pg 10 o1f the manual).

*/
  
  
class CC1205 : public CReadoutHardware
{
  // Clas canonicals
public:
  CC1205();
  CC1205(const CC1205& rhs);
  virtual ~CC1205();
  CC1205& operator=(const CC1205& rhs);

  // Disallowed canonicals:

  int operator==(const CC1205& rhs) const;
  int operator!=(const CC1205& rhs) const;

  // CReadoutHardware interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

  // Utilities:

  virtual uint32_t configurationRegister(); //  Figure out config reg.
  virtual uint32_t rangeMode();		    //  int value of -rangemode.
};
  

#endif
