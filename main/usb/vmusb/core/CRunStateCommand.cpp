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
using namespace std;

#include "CRunStateCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CCtlConfiguration.h>
#include <CRunState.h>

/*!
   Construct the command. 
   - The base class is created with the command string "runstate"
     creation is done with registration.
     \param interp : CTCLInterpreter&
       The interpreter on which this command will be registered.
    \param config : a reference to the CCtlConfiguration for returning results to.
*/
CRunStateCommand::CRunStateCommand(CTCLInterpreter&   interp,
			                             CCtlConfiguration& config) :
  CTCLObjectProcessor(interp, "runstate"),
  m_config(config)  
{

}
//! Destroy the module.. no op provided only as a chain to the base class destructor.
CRunStateCommand::~CRunStateCommand()
{}


/*!
   The command executor.  The command must have only 1 object elements:
   - the command name ("runstate"),

   \param interp : CTCLInterpreter& 
       Reference to the interpreter that is running this command.
    \param vector<TCLObject>& objv
       Reference to an array of objectified Tcl_Obj objects that contain the
       command line prarameters.
    \return int
    \retval - TCL_OK  - If eveything worked.
    \retval - TCL_ERROR - on failures.
*/
int
CRunStateCommand::operator()(CTCLInterpreter& interp,
			   vector<CTCLObject>& objv)
{
  // validate the parameter count.

  if (objv.size() != 1) {
    interp.setResult("'runstate' called with parameters when there should be none.");
    return TCL_ERROR;
  }

  // Determine what the runstate is.
  std::string strState = "unknown";
  CRunState* state = CRunState::getInstance();
  switch (state->getState()) {
    case CRunState::Idle :
      strState = "idle";
      break;
    case CRunState::Active :
      strState = "active";
      break;
    case CRunState::Paused :
      strState = "paused";
      break;
    case CRunState::Starting :
      strState = "starting";
      break;
    case CRunState::Stopping :
      strState = "stopping";
      break;
    default:
      strState = "unknown";
  }
  
  // Return the state
  interp.setResult(strState);
  
  return TCL_OK; 
  
}
