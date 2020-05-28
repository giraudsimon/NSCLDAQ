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

#ifndef CLRS2228_H
#define CLRS2228_H

#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>


// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;

/*!
  CLRS2228 is a device class for the  LRS2228 QDC module.
  Each event trigger results in all 12 channels of the module
  being read out.


  Options supported are:

\verbatim
Name            Type       Initial           Meaning
-slot           int         0      Slot module is installed in.
-id             int16       0      Marker identifying the module.


\endverbatim

*/
class  CLRS2228 : public CReadoutHardware
{

  // class canonicals:

public:
  CLRS2228();
  CLRS2228(const CLRS2228& rhs);
  virtual ~CLRS2228();
  CLRS2228& operator=(const CLRS2228& rhs);

private:
  int operator==(const CLRS2228& rhs) const;
  int operator!=(const CLRS2228& rhs) const;

  // CReadoutHardware interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;



};
#endif

