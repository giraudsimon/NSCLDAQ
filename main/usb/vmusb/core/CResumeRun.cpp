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
#include "CResumeRun.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CTheApplication.h>

#include <CRunState.h>
#include <CControlQueues.h>
#include <tclUtil.h>
#include <CMonVar.h>

using std::string;
using std::vector;

static const string usage(
"Usage:\n\
   resume");
//////////////////////////////////////////////////////////////////
/////////////////////////////// cannonicals //////////////////////
//////////////////////////////////////////////////////////////////


CResumeRun::CResumeRun(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "resume")
{}

CResumeRun::~CResumeRun()
{}

/*!
   -To resume a run requies the followig prerequisites be made:
     - The command must contain only the resume keyword.
     - The run state must be paused.
   - The run is actually resumed via the control queues.

   \param interp : CTCLInterpreter& 
      Reference to the interpreter that is running this command.
   \param objv   : vector<CTCLObject>& 
      Reference to the command words.

*/
int
CResumeRun::operator()(CTCLInterpreter& interp,
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
  CTheApplication* pApp = CTheApplication::getInstance();
  pApp->logStateChangeRequest("Resuming run");
  CRunState* pState = CRunState::getInstance();
  if (pState->getState() != CRunState::Paused) {
    tclUtil::Usage(interp,
		   "Invalid run state, to resume must be paused",
		   objv, usage);
    pApp->logStateChangeStatus("Run is not paused cannot resume");
    return TCL_ERROR;
  }
  // resume the run:

  CControlQueues* pRequest = CControlQueues::getInstance();
  pApp->logProgress("Asking the acquisition thread to resume data taking.");
  pRequest->ResumeRun();
  pState->getVarMonitor()->Set();
  pApp->logStateChangeStatus("Data taking is resumed");
  

  return TCL_OK;
}
