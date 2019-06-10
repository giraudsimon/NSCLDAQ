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
#ifndef __TCLUTIL_H
#define __TCLUTIL_H

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

class CTCLInterpreter;
class CTCLObject;

/*! \file tclUtil.h
      This is just a few Tcl utilities.  They are unbound functions
      that are bundled into the tclUtil namespace.
*/
namespace tclUtil {

  void setResult(CTCLInterpreter& interp, std::string msg);
  void Usage(CTCLInterpreter& interp, std::string msg, 
	     std::vector<CTCLObject>& objv, std::string usage);

};
#endif
