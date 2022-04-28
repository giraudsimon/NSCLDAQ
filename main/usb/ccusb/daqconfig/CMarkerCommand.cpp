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
#include "CMarkerCommand.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CConfiguration.h"
#include <CMarker.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

using std::string;
using std::vector;


//////////////////////////////////////////////////////////////////////////////
///////////////////// Implemented Canonicals /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



/*!

   Construct the command and register it (base constructor does this
   by default.
   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of ADCs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CMarkerCommand::CMarkerCommand(CTCLInterpreter&   interp,
			       CConfiguration&    config,
			       std::string        commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{}

/*!
   Destructor:
*/
CMarkerCommand::~CMarkerCommand()
{
}
//////////////////////////////////////////////////////////////////////
//////////////// Command processing //////////////////////////////////
/////////////////////////////////////////////////////////////////////

/*
  The command entry just:
  - Ensures there is at least 3 parameters, the command, subcommand and
    module name.
  - Dispatches to the appropriate subcommand processor... or
  - Returns an error if the subcommand keyword is not recognized.
  Parameters:
    CTCLInterpreter& interp        - Interpreter running the command.
    std::vector<CTCLObject>& objv  - Command line words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.
  Side effects:
     The interpreter's result will be set in a manner that depends on 
     success/failure and the subcommand's operation.

*/
int
CMarkerCommand::operator()(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 3) {
    Usage("Insufficient command parameters", objv);
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
       adc config [adc create adc1 0x80000000] ....
*/
int
CMarkerCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // Need to have exactly 4 elements, command 'create' name base.

  if (objv.size() != 4) {
    Usage("Not enough parameters for create subcommand", objv);
    return TCL_ERROR;
  }

  // Get the command elements and validate them:

  string name    = objv[2];
  string sValue   = objv[3];

  errno = 0;
  uint32_t value  = strtoul(sValue.c_str(), NULL, 0);
  if ((value == 0) && (errno != 0)) {
    Usage("Invalid value for marker value.", objv);
    return TCL_ERROR;
  }
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    Usage("Duplicate module creation attempted", objv);
    return TCL_ERROR;
  }
  // This is a unique module so we can create it:

  CMarker* pMarker = new CMarker;
  pModule    = new CReadoutModule(name, *pMarker);
  pModule->configure("-value", sValue);

  m_Config.addAdc(pModule);

  m_Config.setResult(name);
  return TCL_OK;
  
}

/*
    Configure an adc module.
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
CMarkerCommand::config(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if ( (objv.size() < 5) || ((objv.size() & 1) == 0)) {
    Usage("Incorrect number of command parameters for config", objv);
    return TCL_ERROR;
  }
  /* Get the module name and use it to locate the module or report an error. */

  string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    Usage("Marker module does not exist", objv);
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
CMarkerCommand::cget(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  if (objv.size() != 3) {
    Usage("Invalid command parameter count for cget", objv);
    return TCL_ERROR;
  }
  string           name    = objv[2];
  CReadoutModule *pModule = m_Config.findAdc(name);
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
////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Utility function(s) ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void
CMarkerCommand::Usage(std::string msg, std::vector<CTCLObject>& objv)
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
  result += "    marker create name value\n";
  result += "    marker config name config-params...\n";
  result += "    marker cget name";

  m_Config.setResult(result);
}
