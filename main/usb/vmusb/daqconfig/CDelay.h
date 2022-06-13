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


#ifndef __CDELAY_H
#define __CDELAY_H

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
  The CDelay class is a module that inserts a delay into the stack execution.
  The delay is provided in 200-ns units.
    
  Configuration parameter is:

\verbatim
Parameter      Value type              value meaning
-value         integer [0-255]         The number of . This must be provided.
                                        (well it actually defaults to 0).


*/

class CDelay : public CReadoutHardware
{
private:
  CReadoutModule*    m_pConfiguration;
public:
  // Class canonicals:

  CDelay();
  CDelay(const CDelay& CDelay);
  virtual ~CDelay();
  CDelay& operator=(const CDelay& rhs);

private:
  int operator==(const CDelay& rhs);
  int operator!=(const CDelay& rhs);


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

private:
  unsigned int getIntegerParameter(std::string name);


};


#endif
