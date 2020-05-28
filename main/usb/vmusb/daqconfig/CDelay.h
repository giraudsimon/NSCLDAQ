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


#ifndef CDELAY_H
#define CDELAY_H

#include "CReadoutHardware.h"
#include <stdint.h>
#include <string>
#include <vector>

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
