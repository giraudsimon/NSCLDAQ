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
#include "CVMUSBCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <CVMUSBControl.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>


/*-----------------------------------------------------
 * canonicals that are implemented
 */

/**
 * CVMUSBCommand
 *
 *  Construct the command.
 *
 * @param interp - The TCL Interpreter on which the command
 *                 is registered.
 * @param config - The configuration which is stored.
 * @param command- The name of the command (defaults to
 *                 vmusb).
 */

CVMUSBCommand::CVMUSBCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) : 
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{
}
/**
 * destructor
 *
 */
CVMUSBCommand::~CVMUSBCommand()
{
}

/*-----------------------------------------------------
** Virtual method overrides, specifically
*  command processing.
*/

/**
 * The command entry point requires a minimum of 3
 * command words.  The first word is the command name
 * the second is a subcommand and the third the
 * object name.
 *
 *  In the end, we are just going to dispatch to the
 *  appropriate module handler.
 *
 *
 * @param interp - The interpreter processing this command.
 * @param objv   - Vector of encapsulated Tcl_Obj's that are
 *                 the command words.
 *
 * @return int - TCL_OK on success, TCL_ERROR with an
 *               error message if there's an error.
 */
int
CVMUSBCommand::operator()(CTCLInterpreter& interp,
			  std::vector<CTCLObject>& objv)
{
  try {
    requireAtLeast(objv, 3, "Insufficient command parameters");
    bindAll(interp, objv);

    std::string subCommand = objv[1];

    if (subCommand == "create") {
      return create(interp, objv);
    } else if (subCommand == "config") {
      return config(interp, objv);
    }
    else if (subCommand == "cget") {
      return cget(interp, objv);
    } else {
      throw std::string("invalid subcommand");
    }
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/*-----------------------------------------------
** Private sub command handlers
*/

/**
 * create
 *
 * Create a new module and its configuration and
 * enter it in the configuration database.
 * Any additional command words are assumed to be 
 * configuration parameters, so config is called if
 * objv.size() > 3
 *
 * @param interp - Interpreter running the command.
 * @param objv   - Command words.
 *
 * @return int TCl_OK on success.
 */
int
CVMUSBCommand::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  std::string name = objv[2];
  
  // prevent duplication:

  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    throw std::string("Duplicate module creation attempted");
  }
 
  CVMUSBControl* pNewModule = new CVMUSBControl;
  pModule = new CReadoutModule(name, *pNewModule);
  m_Config.addAdc(pModule);

  if (objv.size() > 3) return config(interp,objv);

  interp.setResult(name);
  return TCL_OK;
}
/**
 * config
 *  
 * Configure the module.  There must at least 5 parameters
 * and an odd number of parameters at that.  The
 * parameters following the name are configuration option
 * value pairs.
 *
 * @param interp - References the interpreter.
 * @param objv   - Vector of encapsulated Tcl_Obj's that make
 *                 up the command words.
 *
 * @return int - TCL_OK on success.
 */
int
CVMUSBCommand::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireAtLeast(objv, 5, "Insufficient command parameters");
  if ((objv.size() & 1) == 0) {
    throw std::string("Odd number of command words needed for config");
  }

  bindAll(interp, objv);
  std::string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    throw std::string("vmusb module does not exist");
  }

  // Config the module:

  for (int i = 3; i < objv.size(); i+=2) {
    std::string key   = objv[i];
    std::string value = objv[i+1];
    pModule->configure(key, value);
  }
  interp.setResult(name);
  return TCL_OK;

}
/**
 * cget
 * 
 * Return a modules's configuration.
 *
 * @param interp - interpreter
 * @param objv   - Vector of encapsulated command words
 *
 * @return int TCL_OK on normal completion.
 */
int
CVMUSBCommand::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // validate prameters and extract the name.

  requireExactly(objv, 3, "cget requires exactly three command words");
  bindAll(interp, objv);
  std::string name = objv[2];

  // Locate the module.

  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    throw std::string("No such module name");
  }
  // Get the configuration and continue.

  XXUSB::CConfigurableObject::ConfigurationArray config = pModule->cget();

  CTCLObject result;
  result.Bind(interp);

  for (int i = 0; i < config.size(); i++) {
    CTCLObject element;
    element.Bind(interp);
    
    std::string key = config[i].first;
    std::string value = config[i].second;

    element += key;
    element += value;

    result += element;
  }
  interp.setResult(result);

  return TCL_OK;
  
}
