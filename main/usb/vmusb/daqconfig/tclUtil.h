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
#ifndef TCLUTIL_H
#define TCLUTIL_H


#include <string>
#include <vector>

class CTCLInterpreter;
class CTCLObject;
class CConfiguration;
class CReadoutModule;
namespace tclUtil
{
  void  setResult(CTCLInterpreter& interp, std::string msg);
  void Usage(
      CTCLInterpreter& interp, std::string  msg, std::vector<CTCLObject>&  objv, std::string  usage
  );
  std::string getTclTraceback(CTCLInterpreter& interp);
  std::string swigPointer(void* p, std::string  type);
  CReadoutModule* getModule(
     CConfiguration& config,
     CTCLInterpreter& interp, std::vector<CTCLObject>& objv, bool predicate
  );
  void listConfig(CTCLInterpreter& interp, CReadoutModule* pModule);
  std::string newName(
     CTCLInterpreter& interp, CConfiguration* pConfig, std::vector<CTCLObject>& objv
  );
}

#endif
