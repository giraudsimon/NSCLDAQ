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
#include "CEndCommand.h"
#include "CRunControlPackage.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <StateException.h>
#include <CReadoutMain.h>

#include <tcl.h>

using namespace std;


/*!
   Constructor.. we need to register ourself as the end command:
   @param interp   - Reference to the interpreter object on which this
                     command will be registered.

   @note - The command keyword is hard coded to 'end'.
*/
CEndCommand::CEndCommand(CTCLInterpreter& interp) :
  CTCLPackagedObjectProcessor(interp, string("end"))
{
  
}
/*!
   Destructor here just to support chaining.
*/
CEndCommand::~CEndCommand() {}

/*!
   The end command does not take any parameters.
   It attempts to start the run, catching any exceptions that
   might occur during the process and turning them into Tcl errors.

   @param interp  - Reference to the intepreter that's running the command.
   @param objv    - Vector of Tcl objects that are the command parameters.
   @return int
   @return TCL_OK     - The command succeeded and the run is now active.
   @return TCL_ERROR  - The command failed and the run state has not changed.

*/
int
CEndCommand::operator()(CTCLInterpreter&    interp,
			  vector<CTCLObject>& objv)
{
  // There should be no command words other than the 'end' keyword:

  try {
    requireExactly(objv, 1,"Too many command line parameters:\n"); 
    
    int status =0;
    string result;
    tie(status, result) = end();
  
    interp.setResult(result);
    return status;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
}

tuple<int, string> CEndCommand::end() 
{

  CReadoutMain* pMain = CReadoutMain::getInstance();
  pMain->logStateChangeRequest("Ending run");
  
  // Get the package and cast it to a CRunControlPackage:

  CTCLObjectPackage*   pPack       = getPackage();
  CRunControlPackage&  pRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);

  // Attempt the end.  We will catch the common types of exceptions in addition 
  // to the CStateException:
  //
  bool error = false;
  string result;
  try {
    pRunControl.end();
  }
  catch (CStateException& e) {
    error   = true;
    result  = "Run was not in the proper state to end: \n";
    result += e.ReasonText();
  }
  catch (string message) {
    error   = true;
    result  = "String exception caught attempting to end the run: \n";
    result += message;
  }
  catch (const char* message) {
    error   = true;
    result  = "char* exception caught attempting to end the run: \n";
    result += message;
  }
  catch (...) {
    error = true;
    result = "Some unanticipated exception was caught while attempting to end the run";
  }
  if (error) {
      pMain->logStateChangeStatus("End run failed.");        
  } else {
      pMain->logStateChangeStatus("Run successfully ended");
  }
  return make_tuple((error ? TCL_ERROR : TCL_OK), result);
}

/*!
   Provides the usage string for the command
*/
string
CEndCommand::usage()
{
  string result = "Usage\n";
  result       += "   end\n";
  result       += " Attempts to permanently end a data taking run";
  return result;
}
