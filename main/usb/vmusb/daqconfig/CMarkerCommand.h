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

#ifndef __CMARKERCOMMAND_H
#define __CMARKERCOMMAND_H


#include "CDeviceCommand.h"


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;

/*!
   This class creates and configure marker stack entries.  A marker stack
   entry adds a literal 16 bit word to the output buffer for an event.

   The command supports the usual syntaxes for module generating commands:

\verbatim
  marker create name value
  marker config name -value new_value
  marker cget   name

\endverbatim

  As you can see, the only configuration option supported is

  - -value   Sets a new value for the marker.

  Requiring the value on the creation command line is how we ensure that
the value is mandatory
*/

class CMarkerCommand : public CDeviceCommand
{
private:
  CConfiguration& m_Config;

public:
  CMarkerCommand(CTCLInterpreter&     interp,
		 CConfiguration&      config,
		 std::string          commandName = std::string("marker"));
  virtual ~CMarkerCommand();

private:
  CMarkerCommand(const CMarkerCommand& rhs);
  CMarkerCommand& operator=(const CMarkerCommand& rhs);
  int operator==(const CMarkerCommand& rhs) const;
  int operator!=(const CMarkerCommand& rhs) const;
public:


  // Command entry point:



private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
  void Usage(CTCLInterpreter& interp, const char* msg, std::vector<CTCLObject>& objv);
};

#endif
