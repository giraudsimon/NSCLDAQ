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
#ifndef __C3820_H
#define __C3820_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/*!
   The C3820 is a scaler module that will be read out during scaler events.
   We will run the module in 32 bit latch mode.  In our case, the latch
   impetus will be provided from the VME by the readout code.
   The only configuration parameter is
\verbatim
    Parameter Value Type       Value meaning
    -base     integer          Module base address.
\endverbatim
*/
class C3820 : public CReadoutHardware
{
private:
  CReadoutModule*   m_pConfiguration;
public:
  C3820();
  C3820(const C3820& rhs);
  virtual ~C3820();
  C3820& operator=(const C3820& rhs);
private:
  int operator==(const C3820& rhs) const;
  int operator!=(const C3820& rhs) const;

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;
private:
  uint32_t getBase() const;
};


#endif
