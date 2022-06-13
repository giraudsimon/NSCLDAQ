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
#include "CPauseCommand.h"
#include <CReadoutMain.h>

#include "CRunControlPackage.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <StateException.h>
#include <tcl.h>

using namespace std;


/*!
   Constructor.. we need to register ourself as the pause command:
   @param interp   - Reference to the interpreter object on which this
                     command will be registered.

   @note - The command keyword is hard coded to 'pause'.
*/
CPauseCommand::CPauseCommand(CTCLInterpreter& interp) :
  CTCLPackagedObjectProcessor(interp, string("pause"))
{
  
}
/*!
   Destructor here just to support chaining.
*/
CPauseCommand::~CPauseCommand() {}

/*!
  The pause command does not take any parameters.
  It attempts to pause the run, catching any exceptions that fire
  and turning them into Tcl errors (ironically the Tcl object framework
  will turn those into CTCLException exceptions.


   @param interp  - Reference to the intepreter that's running the command.
   @param objv    - Vector of Tcl objects that are the command parameters.
   @return int
   @return TCL_OK     - The command succeeded and the run is now active.
   @return TCL_ERROR  - The command failed and the run state has not changed.

*/
int
CPauseCommand::operator()(CTCLInterpreter&    interp,
			  vector<CTCLObject>& objv)
{
  // There should be no command words other than the 'begin' keyword:
  bool error = false;
  string result;
  CReadoutMain*        pMain       = CReadoutMain::getInstance();
  try {
    requireExactly(objv, 1, "Too many command line parameters:\n");
    
    // Get the package and cast it to a CRunControlPackage:
  
    
    CTCLObjectPackage*   pPack       = getPackage();
    CRunControlPackage&  pRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);
  
    // Attempt the begin.  We will catch the common types of exceptions in addition 
    // to the CStateException:
    //
    
    
    
    pMain->logStateChangeRequest("Pausing a run");
    pRunControl.pause();
    pMain->logStateChangeStatus("Run successfully paused");
  }
  catch (CStateException& e) {
    error   = true;
    result  = "Run was not in the proper state to pause: \n";
    result += e.ReasonText();
  }
  catch (string message) {
    error   = true;
    result  = "String exception caught attempting to pause the run: \n";
    result += message;
  }
  catch (const char* message) {
    error   = true;
    result  = "char* exception caught attempting to pause the run: \n";
    result += message;
  }
  catch (...) {
    error = true;
    result = "Some unanticipated exception was caught while attempting to pause the run";
  }
  if (error) {
     pMain->logStateChangeStatus("Run failed to pause");
  }
  interp.setResult(result);

  return error ? TCL_ERROR : TCL_OK;

}

/*
  Provide the command usage:
*/
string
CPauseCommand::usage()
{
  string result;
  
  result  = "Usage\n";
  result += "   pause\n";
  result += " Attempts to temporarily pause the run\n";
  return result;
}
