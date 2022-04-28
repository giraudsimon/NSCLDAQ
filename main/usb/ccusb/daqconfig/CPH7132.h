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
#ifndef __CPH7132_H
#define __CPH7132_H
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
