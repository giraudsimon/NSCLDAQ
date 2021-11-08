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
#include "CHINPCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <CXLM.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
using namespace std;

using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////
/////////////////// Canonicals that are implemented ////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Construct the command and register it (base constructor does this
   by default.
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of HINP XLMs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CHINPCommand::CHINPCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) : 
  CDeviceCommand(interp, commandName.c_str(), config),
  m_Config(config)
{
}
/*!
   Destructor is a no-op but chains to the base class which unregisters
   etc.
*/
CHINPCommand::~CHINPCommand()
{
}

////////////////////////////////////////////////////////////////////////
///////////////////// Command processing ///////////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Process the create subcommand:
   - ensure we have enough values on the command line.
   - ensure we have a valid adc name, and base address.
   - ensure that there is no other adc with the same name.
   - Create the new adc module
   - Add it to the configuration.
   Parameters:
     CTCLInterpreter&    interp   - Interpreter that is executing this command.
     vector<CTCLObject>& objv     - Vector of command words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The result for the interpreter is set as follows:
     - On error this is an error message of the form ERROR: message
     - On success, this is the name of the module. allowing e.g.
       hinp config [hinp create hinp1 0x80000000] ....
*/
int
CHINPCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if (objv.size()) {
    Usage(interp, "NSCLDAQ does NOT support software for CHINP. Please contact whom it may concern.", objv);
    return TCL_ERROR;
  }
	return TCL_OK;
}
/*
  Return the configuration. This allows subclassed commands to function properly.
*/
CConfiguration* 
CHINPCommand::getConfiguration()
{
  return &m_Config;
}

