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
#include "CPauseRun.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CRunState.h>
#include <CControlQueues.h>
#include <CTheApplication.h>
#include <tclUtil.h>

using std::string;
using std::vector;

static const string usage(
"Usage:\n\
   pause");

//////////////////////////////////////////////////////////////////
/////////////////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////

CPauseRun::CPauseRun(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "pause") 
{}
CPauseRun::~CPauseRun()
{}

//////////////////////////////////////////////////////////////////
//////////////////// Command processing //////////////////////////
//////////////////////////////////////////////////////////////////

/*!
   To pause a run requires that:
   - The following prerequisites be made:
     - The pause command must have no extra command line parameters.
     - The run state must be Active.
   - We request the pause through the CControlQueues.

   \param interp : CTCLInterpreter& 
      Reference to the interpreter that is running this command.
   \param objv   : vector<CTCLObject>& 
      Reference to the command words.

*/
int
CPauseRun::operator()(CTCLInterpreter& interp,
		      vector<CTCLObject>& objv)
{
  // Check the prereqs:

  if (objv.size() != 1) {
    tclUtil::Usage(interp, 
		   "Invalid parameter count",
		   objv,
		   usage);
    return TCL_ERROR;
  }
  // check the state:

  CTheApplication* pApp = CTheApplication::getInstance();
  pApp->logStateChangeRequest("Pausing run");
  CRunState* pState = CRunState::getInstance();
  if (pState->getState() != CRunState::Active) {
    tclUtil::Usage(interp,
		   "To pause, the run must be active",
		   objv,
		   usage);
    pApp->logStateChangeStatus("Run was not active - could not pause");
    return TCL_ERROR;
  }
  CControlQueues* pRequest = CControlQueues::getInstance();

  pApp->logProgress("Requesing acquisition thread pause run");
  pRequest->PauseRun();
  pApp->logStateChangeStatus("Run is paused");

  return TCL_OK;
}
