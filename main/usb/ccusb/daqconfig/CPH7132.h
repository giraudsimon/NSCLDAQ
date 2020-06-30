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
#ifndef CPH7132_H
#define CPH7132_H
#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>


// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;

/*!
  CPH7132 supports reading out the Phillips 7132 32 channel scaler module.

  options:
\verbatim
name  type default  contains
-slot int  0        CAMAC slot module is inserted in.
*/
class CPH7132 : public CReadoutHardware
{

  // Implemented canonicals.
public:
  CPH7132();
  CPH7132(const CPH7132& rhs);
  virtual ~CPH7132();
  CPH7132& operator=(const CPH7132& rhs);

  // Unsupported canonicals.

private:
  int operator==(const CPH7132& rhs) const;
  int operator!=(const CPH7132& rhs) const;

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;
  
};
  


#endif
