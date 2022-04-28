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

#ifndef __CMASE_H
#define __CMASE_H

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

#ifndef __CXLM_H
#include "CXLM.h"		/* Base class definition. */
#endif

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;


/*!
  Provides support for the MASE XLM based readout system.  This class uses the CXLM base class
  to provide the basic services and configuration parameters it requires.
  We don't require any additional configuration parameters.  Note, however that it is
  recommended that the event stack have a 250usec delay between trigger and list start
  in order to allow the MASE hardware to complete data acquisition and transfer to the
  XLM SRAM B.
  structure of the SRAMB is
\verbatim
+-----------------------------------------+
|  data count (noninclusive               |
+-----------------------------------------+
|  ... data ...                           |
~                                         ~
+-----------------------------------------+
\endverbatim
   At end of read the readout must 'reset' the XLM by touching its register set.
*/

class CMASE : public XLM::CXLM
{
public:
  CMASE();
  CMASE(const CMASE& rhs);
  virtual ~CMASE();
  CMASE& operator=(const CMASE& rhs);

private:
  int operator==(const CMASE& rhs) const;
  int operator!=(const CMASE& rhs) const;

  // Standard readout interface.. note that onAttach is implemented by the base class
  // and that its implementation is perfectly fine for us.
  // 

public:
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

};

#endif
