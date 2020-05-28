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

#ifndef CLRS2249_H
#define CLRS2249_H

#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>


// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;

/*!
  CLRS2249 is a device class for the  LRS2249 QDC module.
  Each event trigger results in all 12 channels of the module
  being read out.


  Options supported are:

\verbatim
Name            Type       Initial           Meaning
-slot           int         0      Slot module is installed in.
-id             int16       0      Marker identifying the module.


\endverbatim

*/
class  CLRS2249 : public CReadoutHardware
{

  // class canonicals:

public:
  CLRS2249();
  CLRS2249(const CLRS2249& rhs);
  virtual ~CLRS2249();
  CLRS2249& operator=(const CLRS2249& rhs);

private:
  int operator==(const CLRS2249& rhs) const;
  int operator!=(const CLRS2249& rhs) const;

  // CReadoutHardware interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;



};
#endif

