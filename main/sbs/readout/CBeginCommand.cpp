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
#include "CBeginCommand.h"
#include "CRunControlPackage.h"
#include "RunState.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLVariable.h>
#include <StateException.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <tcl.h>
#include <CReadoutMain.h>

using namespace std;

/*!
   Constructor.. we need to register ourself as the begin command:
   @param interp   - Reference to the interpreter object on which this
                     command will be registered.

   @note - The command keyword is hard coded to 'begin'.
*/
CBeginCommand::CBeginCommand(CTCLInterpreter& interp) :
  CTCLPackagedObjectProcessor(interp, string("begin"))
{
  
}
/*!
   Destructor here just to support chaining.
*/
CBeginCommand::~CBeginCommand() {}

/*!
   The begin command does not take any parameters.
   It attempts to start the run, catching any exceptions that
   might occur during the process and turning them into Tcl errors.

   @param interp  - Reference to the intepreter that's running the command.
   @param objv    - Vector of Tcl objects that are the command parameters.
   @return int
   @return TCL_OK     - The command succeeded and the run is now active.
   @return TCL_ERROR  - The command failed and the run state has not changed.

*/
int
CBeginCommand::operator()(CTCLInterpreter&    interp,
			  vector<CTCLObject>& objv)
{
  bool error = false;
  string result;
  CReadoutMain* pMain = CReadoutMain::getInstance();
  // There should be no command words other than the 'begin' keyword:
  try {
    requireExactly(objv, 1, "Too many command line parameters:\n");
  
    // Get the package and cast it to a CRunControlPackage:
  
    CTCLObjectPackage*   pPack       = getPackage();
    CRunControlPackage&  pRunControl = reinterpret_cast<CRunControlPackage&>(*pPack);
  
    // Attempt the begin.  We will catch the common types of exceptions in addition 
    // to the CStateException:
    //
   
  
    
    pMain->logStateChangeRequest("Beginning run");
     
    // Set values for the title and run number if these Tcl variables are set:

    RunState* pState =  RunState::getInstance();

    CTCLVariable run(&interp, string("run"), TCLPLUS::kfFALSE);
    CTCLVariable title(&interp, string("title"), TCLPLUS::kfFALSE);

    const char* runValue = run.Get();
    const char* titleValue = title.Get();

    if (runValue && (pState->m_state == RunState::inactive)) {
      uint32_t newValue;
      char*    endptr;
      newValue = strtoul(runValue, &endptr, 0);
      if (runValue != endptr) {
	pState->m_runNumber = newValue;
      }

    }
    pMain->logProgress("Run  number set");
    if (titleValue && (pState->m_state == RunState::inactive)) {
      delete []pState->m_pTitle;
      pState->m_pTitle = new char[strlen(titleValue)+1];
      strcpy(pState->m_pTitle, titleValue);
      
    }
    pMain->logProgress("Run title set");
    pRunControl.begin();
    pMain->logStateChangeStatus("Run successfully begun");
  }
  catch (CStateException& e) {
    error   = true;
    result  = "Run was not in the proper state to begin: \n";
    result += e.ReasonText();
  }
  catch (string message) {
    error   = true;
    result  = "String exception caught attempting to start the run: \n";
    result += message;
  }
  catch (const char* message) {
    error   = true;
    result  = "char* exception caught attempting to start the run: \n";
    result += message;
  }
  catch (...) {
    error = true;
    result = "Some unanticipated exception was caught while attempting to start the run";
  }

  if (error) {
      std::string msg = "Begin run failed ";
      msg += result;
      pMain->logStateChangeStatus(msg.c_str());
  }
  interp.setResult(result);

  return error ? TCL_ERROR : TCL_OK;

}
/*!
   Provides the usage string for the command
*/
string
CBeginCommand::usage()
{
  string result = "Usage\n";
  result       += "   begin\n";
  result       += " Attempts to start a new data taking run";
  return result;
}
