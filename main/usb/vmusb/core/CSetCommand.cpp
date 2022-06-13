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
#include "CSetCommand.h"
#include <CCtlConfiguration.h>
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CControlModule.h>
#include <CRunState.h>
#include <CControlQueues.h>
#include <CVMUSB.h>
#include <CMutex.h>
#include <tcl.h>

using namespace std;

/*!
  Construct the command.
*/
CSetCommand::CSetCommand(CTCLInterpreter&   interp,
                  			 CCtlConfiguration& config,
                  			 CVMUSB&            vme) :
  CTCLObjectProcessor(interp, "Set"),
  m_config(config),
  m_Vme(vme)
{}
CSetCommand::~CSetCommand()
{}



/*
   Execute the set command.  Set the class comments for syntax.
*/
int
CSetCommand::operator()(CTCLInterpreter& interp,
			vector<CTCLObject>& objv)
{
  // Must be 4 words in the command:

  if (objv.size() != 4) {
    interp.setResult("ERROR Set: Incorrect number of command words. Need: Set name point value");
    return TCL_ERROR;
  }
  // Pull out the values.. All values are strings:

  string name = objv[1];
  string point= objv[2];
  string value= objv[3];

  // Need to find the module:

  CControlModule* pModule = m_config.findModule(name);
  if (!pModule) {
    string msg("ERROR Set: Control module: ");
    msg += name;
    msg += " cannot be found";
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

  string result;
  try {
  // Now try the command returning any string error that is thrown:
    result = pModule->Set(m_Vme, point.c_str(), value.c_str());

  } catch (std::string msg) {
      result = msg;
  } catch (std::exception& exc) {
      result = exc.what();
  } catch (...) {
      result = "ERROR - caught unknown exception";
  }
  interp.setResult(result);
  // if we need to, put the vmusb back into acquisition mode
  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }

  return TCL_OK;
  
}
