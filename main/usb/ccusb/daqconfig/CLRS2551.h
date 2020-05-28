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

#ifndef CLRS2551_H
#define CLRS2551_H


#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>


// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;

/*!
  CLRS2551 is a device class for the  LRS2551 scaler
  This is a very simple module.
  All 12 channnels are read out regardless of the occupancy as the 
  module does not support useful sparse readoutl.

  Options supported are:

\verbatim
Name            Type       Initial           Meaning
-slot           int         0      Slot module is installed in.
-id             int16       0      Marker identifying the module.
-insertid       bool       false   Insert the id (set true if event mode).
-cummulative    bool       false   If true, scalers are not reset on read.


\endverbatim

*/
class  CLRS2551 : public CReadoutHardware
{

  // class canonicals:

public:
  CLRS2551();
  CLRS2551(const CLRS2551& rhs);
  virtual ~CLRS2551();
  CLRS2551& operator=(const CLRS2551& rhs);

private:
  int operator==(const CLRS2551& rhs) const;
  int operator!=(const CLRS2551& rhs) const;

  // CReadoutHardware interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;



};
#endif

