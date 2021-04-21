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
#include <config.h>
#include "CInitCommand.h"
#include "CRunControlPackage.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <StateException.h>
#include <tcl.h>
#include <iostream>

using namespace std;


/*!
   Constructor.. we need to register ourself as the resume command:
   @param interp   - Reference to the interpreter object on which this
                     command will be registered.

   @note - The command keyword is hard coded to 'resume'.
*/
CInitCommand::CInitCommand(CTCLInterpreter& interp) :
  CTCLPackagedObjectProcessor(interp, string("init"))
{
  
}
/*!
   Destructor here just to support chaining.
*/
CInitCommand::~CInitCommand() {}

/*!
   The resume command does not take any parameters.
   It attempts to restart the run, catching any exceptions that
   might occur during the process and turning them into Tcl errors.

   @param interp  - Reference to the intepreter that's running the command.
   @param objv    - Vector of Tcl objects that are the command parameters.
   @return int
   @return TCL_OK     - The command succeeded and the run is now active.
   @return TCL_ERROR  - The command failed and the run state has not changed.

*/
int
CInitCommand::operator()(CTCLInterpreter&    interp,
			  vector<CTCLObject>& objv)
{
  // There should be no command words other than the 'resume' keyword:

  if (objv.size() > 1) {
    std::string result = "Too many command line parameters:\n";
    result            += usage();
    interp.setResult(result);
    return TCL_ERROR;
  }

  // Get the package and cast it to a CRunControlPackage:

  CTCLObjectPackage*   pPack       = getPackage();
  CRunControlPackage&  pRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);

  interp.setResult("Init not implemented");
  return TCL_OK;
  
}
/*!
   Provides the usage string for the command
*/
string
CInitCommand::usage()
{
  string result = "Usage\n";
  result       += "   init\n";
  result       += " Does nothing.";
  return result;
}
