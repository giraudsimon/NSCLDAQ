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
#include "CUpdateCommand.h"

#include "CCtlConfiguration.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include "CControlModule.h"
#include <CRunState.h>
#include <CControlQueues.h>
#include <CVMUSB.h>
#include <CMutex.h>
#include <tcl.h>

using namespace std;

/*!
  Construct the command.
*/
CUpdateCommand::CUpdateCommand(CTCLInterpreter&   interp,
    			                     CCtlConfiguration& config,
                        			 CVMUSB&            vme) :
  CTCLObjectProcessor(interp, "Update"),
  m_config(config),
  m_Vme(vme)
{}
CUpdateCommand::~CUpdateCommand()
{}

/*
   Execute the Update command.  See the class comments for syntax.
*/
int
CUpdateCommand::operator()(CTCLInterpreter& interp,
			vector<CTCLObject>& objv)
{
  // Need 2 words on the command line:

  if (objv.size() != 2) {
    interp.setResult(
	      "ERROR Update: Incorrect number of command parameters : need Update name");
    return TCL_ERROR;
  }

  // Get the pieces of the command:


  string name  = objv[1];
  
  // Locate the object:

  CControlModule* pModule = m_config.findModule(name);
  if (!pModule) {
    string msg("ERROR Update: unable to find module ");
    msg += name;
    interp.setResult( msg);
    return TCL_ERROR;
  }

  CriticalSection lock(CVMUSB::getGlobalMutex());

  // If we are in the middle of a run, we need to halt data collection
  // before using the vmusb
  bool mustRelease(false);
  if (CRunState::getInstance()->getState() == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }

  string result =   pModule->Update(m_Vme);
  interp.setResult( result);

  // if we need to, put the vmusb back into acquisition mode
  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }

  return TCL_OK;
}
