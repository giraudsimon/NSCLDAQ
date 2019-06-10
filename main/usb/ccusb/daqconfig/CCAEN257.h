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

#ifndef __CCAEN257_H
#define __CCAEN257_H

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
   This class supports the CAEN 257 scaler module.  Configuration options are:
\verbatim
Name          Type          Default      Meaning
-slot         integer       0 (illegal)  Slot in which the module was installed.
-id           integer       0            Id (see below).
-insertid     bool          false        If true, the -id value precedes the
                                         data for this module.  This is supplied
                                         in the unlikely event the module is used
                                         for event readout rather than as a periodic 
                                         scaler.
-readinhibit  bool          true         Inhibit CAMAC while reading.

\endverbatim

*/
class CCAEN257 : public CReadoutHardware
{
  // Class canonicals:
public:
  CCAEN257();
  CCAEN257(const CCAEN257& rhs);
  virtual ~CCAEN257();
  CCAEN257& operator=(const CCAEN257& rhs);

  // Disallowed canonicals... comparison makes no sense for CAMAC modules:

private:
  int operator==(const CCAEN257& rhs) const;
  int operator!=(const CCAEN257& rhs) const;

  // This modules implements the CReadoutHardware INterface:
public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

};

#endif
