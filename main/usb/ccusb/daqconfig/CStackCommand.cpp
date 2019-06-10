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
#include "CStackCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <CStack.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

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
       The configuration of STACKs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CStackCommand::CStackCommand(CTCLInterpreter& interp,
			 CConfiguration&  config,
			 std::string      commandName) : 
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{
}
/*!
   Destructor is a no-op but chains to the base class which unregisters
   etc.
*/
CStackCommand::~CStackCommand()
{
}

////////////////////////////////////////////////////////////////////////
///////////////////// Command processing ///////////////////////////////
////////////////////////////////////////////////////////////////////////

/*
  The command entry just:
  - ensures there are at least 2 command words, the command and subcommand.
  - Dispatches to the approrpriate processor depending on the subcommand
  - reports errors in the evetn the subcommand is not recognized.

*/
int
CStackCommand::operator()(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 2) {
    Usage("Incorrect number of  command parameters", objv);
    return TCL_ERROR;
  }
  // Get the subcommand keyword and dispatch or error:

  string subcommand = objv[1];
  if (subcommand == string("create")) {
    return create(interp, objv);
  }
  else if (subcommand == string("config")) {
    return config(interp, objv);
  } 
  else if (subcommand == string("cget")) {
    return cget(interp, objv);
  }
  else {
    Usage("Invalid subcommand", objv);
    return TCL_ERROR;
  }
}
/*
   Process the create subcommand:
   - ensure we have enough values on the command line.
   - ensure we have a valid stack name, and base address.
   - ensure that there is no other stack with the same name.
   - Create the new stack module
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
       stack config [stack create stack1] ....
*/
int
CStackCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // Need to have exactly 3 elements, command 'create' name '

  if (objv.size() != 3) {
    Usage("Incorrect Number of Parameters", objv);
    return TCL_ERROR;
  }

  // Get the command elements and validate them:

  string name    = objv[2];

  errno = 0;

  CReadoutModule* pModule = m_Config.findStack(name);
  if (pModule) {
    Usage("Duplicate stack creation attempted", objv);
    return TCL_ERROR;
  }
  // This is a unique module so we can create it:

  CStack* pStack = new CStack;
  pModule    = new CReadoutModule(name, *pStack);


  m_Config.addStack(pModule);

  m_Config.setResult(name);
  return TCL_OK;
  
}
/*
    Configure an stack module.
    - Ensure that there are enough command line parameters.  These means
      at least 5 parameters in order to have at least one configuration
      keyword value pair... and that there are an odd number of params
      (to ensure that all keywords have values).
    - Ensure the module exists.
    - For each command keyword/value pair, configure the module.

   Parameters:
     CTCLInterpreter&    interp   - Interpreter that is executing this command.
     vector<CTCLObject>& objv     - Vector of command words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The interpreter result is set with an error message if the return value
     is TCL_ERROR, otherwise it is set with the module name.
     Note that all error messages will start with the text "ERROR:"
*/
int
CStackCommand::config(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if ( (objv.size() < 5) || ((objv.size() & 1) == 0)) {
    Usage("Incorrect number of command parameters for config", objv);
    return TCL_ERROR;
  }
  /* Get the module name and use it to locate the module or report an error. */

  string name = objv[2];
  CReadoutModule* pModule = m_Config.findStack(name);
  if (!pModule) {
    Usage("Stack module does not exist", objv);
    return TCL_ERROR;
  }
  /* Process the configuration... this is done inside a try/catch block
    as the configure can throw.
  */
  try {
    for (int i = 3; i < objv.size(); i += 2) {
      string key   = objv[i];
      string value = objv[i+1];
      pModule->configure(key, value);
    }
  }
  catch (string msg) {		// BUGBUG - This may partially configure object.
    Usage(msg, objv);
    return TCL_ERROR;
  }

  m_Config.setResult(name);
  return TCL_OK;
}
/*
   Get the configuration of a module and return it as a list of
   keyword/value pairs.
   - ensure we have enough command line parameters (exactly 3).
   - Ensure the module exists and get its pointer.
   - Fetch the module's configuration.
   - Map the configuration into a list of 2 element lists and set the
     result accordingly.

   Parameters:
     CTCLInterpreter&    interp   - Interpreter that is executing this command.
     vector<CTCLObject>& objv     - Vector of command words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The interpreter result is set.  If the command returned an error, 
     This is a string that begins with the text ERROR:  otherwise it is a 
     list of 2 element sublists where each sublist is a configuration keyword
     value pair...e.g. {-base 0x80000000} ...
*/
int
CStackCommand::cget(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    Usage("Invalid command parameter count for cget", objv);
    return TCL_ERROR;
  }
  string           name    = objv[2];
  CReadoutModule *pModule = m_Config.findStack(name);
  if (!pModule) {
    Usage("No such  module", objv);
    return TCL_ERROR;
  }
  XXUSB::CConfigurableObject::ConfigurationArray config = pModule->cget();

  Tcl_Obj* pResult = Tcl_NewListObj(0, NULL);

  for (int i =0; i < config.size(); i++) {
    Tcl_Obj* key   = Tcl_NewStringObj(config[i].first.c_str(), -1);
    Tcl_Obj* value = Tcl_NewStringObj(config[i].second.c_str(), -1);

    Tcl_Obj* sublist[2] = {key, value};
    Tcl_Obj* sl = Tcl_NewListObj(2, sublist);
    Tcl_ListObjAppendElement(interp.getInterpreter(), pResult, sl);
  }
  Tcl_SetObjResult(interp.getInterpreter(), pResult);
  return TCL_OK;

}
/*
  Return the configuration. This allows subclassed commands to function properly.
*/
CConfiguration* 
CStackCommand::getConfiguration()
{
  return &m_Config;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Utility function(s) ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void
CStackCommand::Usage(std::string msg, std::vector<CTCLObject>& objv)
{
  string result("ERROR: ");
  result += msg;
  result += "\n";
  for (int i = 0; i < objv.size(); i++) {
    result += string(objv[i]);
    result += ' ';
  }
  result += "\n";
  result += "Usage\n";
  result += "    stack create name\n";
  result += "    stack config name config-params...\n";
  result += "    stack cget name";

  m_Config.setResult(result);
}
