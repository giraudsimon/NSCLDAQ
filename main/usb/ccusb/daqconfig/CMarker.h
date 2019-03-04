/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef __CMARKER_H
#define __CMARKER_H

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
  The CMarker class is a module that inserts a marker word into the output
  data stream.  The marker word is a 16 bit datum. You can insert a 32 bit
  marker by inserting two of these with the low order 16 bits first.

  Configuration parameter is:

\verbatim
Parameter      Value type              value meaning
-value         integer [0-0xffff]       The marker word to insert. This must be provided.
                                        (well it actually defaults to 0).


*/

class CMarker : public CReadoutHardware
{
private:
  CReadoutModule*    m_pConfiguration;
public:
  // Class canonicals:

  CMarker();
  CMarker(const CMarker& CMarker);
  virtual ~CMarker();
  CMarker& operator=(const CMarker& rhs);

private:
  int operator==(const CMarker& rhs);
  int operator!=(const CMarker& rhs);


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

private:
  unsigned int getIntegerParameter(std::string name);


};


#endif
