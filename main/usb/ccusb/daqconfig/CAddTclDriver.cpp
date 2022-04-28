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
#include "CAddTclDriver.h"
#include <CTclModule.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CReadoutModule.h>
#include "CConfiguration.h"

/**
 * Constrution  We need to register our command and save the configuration so that
 * new elements can be added to it.
 *
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of STACKs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CAddTclDriver::CAddTclDriver(CTCLInterpreter& interp,
			     CConfiguration& config,
			     std::string     commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{}

/**
 * Base class destructor does all the work:
 */
CAddTclDriver::~CAddTclDriver() {}

/**
 * Command processing. Ensure the command has the right number of parameter,
 * Construct a TclDriver and register it with the configuration.
 * We don't have any subcommands actually...unlike the other configuration support commands.
 *
  Command entry ensures there are at least a three  command words,
  the command itself, the subcommand and the name of a module being
  manipulated.  

  Based on the value of the subcommand, the appropriate processing 
  function is invoked.

  \param interp    - The interpreter that's running this command.
  \param objv      - Vector of command words.

  \return int
  \retval TCL_OK    - The command succeeded.
  \retval TCL_ERROR - The command failed in some way.

  \note The interpreter result will be empty on success and an error message on failure.
*/
int
CAddTclDriver::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // must have two parameter:

  if (objv.size() != 2) {
    m_Config.setResult("Usage:\n    addtcldriver base-command-name");
    return TCL_ERROR;
  }

  // Bind the name and get it as a string:

  objv[1].Bind(interp);
  std::string baseCommand = objv[1];

  // Ensure the base command is not an existing device name:

  CReadoutModule* pModule = m_Config.findAdc(baseCommand);
  if (pModule) {
    std::string msg = "Error registering ";
    msg       += baseCommand;
    msg       += " as a Tcl driver. There is already a module with that name";
    m_Config.setResult(msg);
    return TCL_ERROR;
  }

  // Create the wrapper object and register it with the configuratino.
  // note that we need a configuration object (since that's what's actually registered),
  // even though this has no intrinsic configuration (that's managed by the driver).
  //

  CTclModule* pDriver = new CTclModule(baseCommand, interp);
  pModule             = new CReadoutModule(baseCommand, *pDriver);
  m_Config.addAdc(pModule);

  return TCL_OK;

}
