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
#ifndef __CMADCSCALER_H
#define __CMADCSCALER_h


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
class CVMUSB;
class CVMUSBReadoutList;

/*!
  This module provides support for a 'scaler' that reads two of the time
  counters in an MADC32 module.  The counters read are the
  daq_time_lo/hi counter which provides the time the adc is dead, and the
  time_0/time_1 registers.  Initialization will zero these time counters.

  Using these values provides a dead-time information for the system.

Configuration parameters:

\verbatim
   -base     integer   - Base address of the module.
\endverbatim


*/
class CMADCScaler : public CReadoutHardware
{
private:
  CReadoutModule*     m_pConfiguration;
public:
  CMADCScaler();
  CMADCScaler(const CMADCScaler& rhs);
  virtual ~CMADCScaler();
  CMADCScaler& operator=(const CMADCScaler& rhs);
private:
  int operator==(CMADCScaler& rhs) const;
  int operator!=(CMADCScaler& rhs) const;

  // The interface for CReadoutHardware:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;


};

#endif
