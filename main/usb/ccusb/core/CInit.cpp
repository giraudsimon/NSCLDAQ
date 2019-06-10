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
#include "CInit.h"
#include <TCLInterpreter.h>
#include <TCLInterpreterObject.h>
#include <TCLVariable.h>
#include <Globals.h>
#include "tclUtil.h"
#include <CAcquisitionThread.h>
#include <CRunState.h>
#include <CConfiguration.h>
#include <iostream>
#include <TclServer.h>

using std::vector;
using std::string;
using std::cerr;
using std::endl;

static const string usage(
"Usage:\n\
   init");

/////////////////////////////////////////////////////////////////////////
//////////////////////////////// Canonicals /////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*!
  Construct the begin command 
  \param interp : CTCLInterpreter&
     Interpreter on which this command will be registered.
*/
CInit::CInit(CTCLInterpreter& interp) :
  CTCLObjectProcessor(interp, "init")
{}
/*!
   Destructor does nothing important.
*/
CInit::~CInit()
{}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Command execution /////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*!
   Process the begin run.
   - Ensure that the preconditions for starting the run are met these are:
     - The begin command has no additional command line parameters.
     - The current run state is Idle
   - Create the Adc/Scaler configuration.
     - Configure it from the file stored in Globals::configurationFilename.
     - Store that configuration in Globals::configuration
   - Start up the readout thread to take data.

   \param interp : CTCLInterpreter&
        Interpreter that is exeuting this command.
   \param objv : std::vector<CTCLObject>&
        Reference to an object vector that contains the command parameters.
*/
int
CInit::operator()(CTCLInterpreter& interp,
		      vector<CTCLObject>& objv)
{
  // Make sured all precoditions are met.

  if (objv.size() != 1) {
    tclUtil::Usage(interp, 
		   "Incorrect number of command parameters",
		   objv,
		   usage);
    return TCL_ERROR;
  }

  CRunState* pState = CRunState::getInstance();
  if (pState->getState() != CRunState::Idle) {
    tclUtil::Usage(interp,
		   "Invalid run state. Must be idle",
		   objv,
		   usage);
    return TCL_ERROR;
  }

  // Initialize the Modules loaded into the tclserver 
  // via the controlconfig.tcl script 
  Globals::pTclServer->initModules();

  tclUtil::setResult(interp, string("Init - initialization procedures executed"));
  return TCL_OK;
}

