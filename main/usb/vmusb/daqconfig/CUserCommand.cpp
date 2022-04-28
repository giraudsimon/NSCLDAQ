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
#include "CUserCommand.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CReadoutHardware.h>
#include <Exception.h>
#include <Globals.h>

using namespace std;

/**************************************************************
 * Canonical methods 
 **************************************************************/

/**
 * Construct an instance of the command.
 * 
 * @param interp - The interpreter the command is registered on.
 * @param config - The configuration the command manipulates.
 * @param commandName - The name of the command 
 * @param pTemplate - A template device driver which will be cloned
 *                    to produce specific device driver instances.
 */
CUserCommand::CUserCommand(CTCLInterpreter& interp,
			   CConfiguration& config,
			   std::string     commandName, CReadoutHardware* pTemplate) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config),
  m_pTemplate(pTemplate)
{
  
}
/**
 * For now deletion is a no-op.
 */
CUserCommand::~CUserCommand() {}

/*********************************************************************
 * Command Processing
 ********************************************************************/

/**
 * Gains control when the registered command is invoked.  Based on the
 * Subcommand dispatches to one of create, config, cget or indicates
 * an error with an error return in the Tcl interpreter.
 * 
 * @param interp - Tcl Interpreter that is executing the command.
 * @param objv   - vector of wrapped Tcl_Objv objects that are the command words.
 *
 * @return int
 * @return TCL_OK - successful command. See create, config, cget for information
 *                  about what, if anything, the result might be. 
 * @return TCL_ERROR - Failed command.  The command result is a human readable error
 *                     message.
 */
int
CUserCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if (objv.size() < 3) {
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
/**
 * Creates a new instance of a driver.  This command has the form:
 *
 * \verbatim
 *    xxx create name ?options?
 * \endverbatim
 *
 *  Where options are a list of configuration option names followed by values as in e.g.:
 *
 * \verbatim
 *    xxx create name -slot 6 -anotheroption stuff ...
 * \endverbatim
 *
 * @param interp - Tcl Interpreter that is executing the command.
 * @param objv   - vector of wrapped Tcl_Objv objects that are the command words.
 *
 * @return int
 * @return TCL_OK - successful command. The command result is the name of the module
 *                  supporting constructs like set name [xxx create george...]
 * @return TCL_ERROR - Failed command.  The command result is a human readable error
 *                     message.
 */
int
CUserCommand::create(CTCLInterpreter& interp,
		       std::vector<CTCLObject>& objv)
{
  // Need to make sure the word count of the command is valid.

  if ((objv.size() < 3) || ((objv.size() & 1) == 0)) {
    Usage("Invalid number of command parameters, must be at least 3 and odd",
	  objv);
    return TCL_ERROR;
  }
 
  // Get the name of the module, ensure it will be unique:

  string name = objv[2];

  CReadoutModule* pModule = m_Config.findAdc(name);
  if (pModule) {
    Usage("Duplicate module creation attempted: ", objv);
    return TCL_ERROR;
  }
  // Since the module is unique, we can create it we won't register it until
  // the configuration is successful;

  CReadoutHardware* pNewModule   = m_pTemplate->clone();
  pModule        = new CReadoutModule(name, *pNewModule); // Also attaches pAdc to configuration.

  // If there are configuration parametesr, process them; Relies on the fact that
  // all commands have the same length prefix:  type subcommand module-name ...

  int status = TCL_OK;
  if (objv.size() > 3) {
    status     = configure(interp,
			   pModule,
			   objv); 
  }

  // Add the module if all is still ok.

  if (status == TCL_OK) {
    m_Config.addAdc(pModule);
    m_Config.setResult(name);
  }
  else {
    delete pModule;
    delete pNewModule;
  }
  return status;
}

/*
  Process the config subcommand.
  - Ensure there are enough command parameters.  We need at least 5, and
    we need an odd number of configuration parameters to ensure that
    the -switch/value pairs are balanced.
  - Ensure the module exists.
  - Let the configure utility take care of the rest of the work (factored code
    with the create method).

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

*/
int
CUserCommand::config(CTCLInterpreter& interp,
		      std::vector<CTCLObject>& objv)
{
  // Ensure the parameter counts are valid:

  if ((objv.size() < 5) || ((objv.size() & 1) == 0)) {
    Usage("Incorrect number of command parameters for config", objv);
    return TCL_ERROR;
  }

  // Get the module name and locate it.. it's an error for the module to not exist.

  string          name     = objv[2];
  CReadoutModule* pModule  = m_Config.findAdc(name);
  if(!pModule) {
    Usage("ad811 module does not exist", objv);
    return TCL_ERROR;
  }
  // and configure:

  m_Config.setResult(name);	// This gets overwritten in case of error.
  return configure(interp, pModule, objv);

}
/*
  Process the cget subcommand which returns the configuration of a module as
  a list of keyword/value pairs.
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
CUserCommand::cget(CTCLInterpreter& interp,
		    std::vector<CTCLObject>& objv)
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

/************************************************************************
 * Attribute selectors
 ***********************************************************************/

CConfiguration* 
CUserCommand::getConfiguration()
{
  return &m_Config;
}
/***********************************************************************/
/*  Produce an error message, set it in the interpreter result field.
    Parameters:
       msg  -  A string to put in the message.
       objv -  The command words which are also put in the error message to 
               help the user locate the problem.

*/
void
CUserCommand::Usage(std::string msg, std::vector<CTCLObject> objv)
{
  string result("ERROR: ");
  string cmdName = objv[0];

  result += msg;
  result += "\n";
  for (int i = 0; i < objv.size(); i++) {
    result += string(objv[i]);
    result += ' ';
  }
  result += "\n";
  result += "Usage\n";
  result += "    ";
  result += cmdName;
  result += "  create name ?config-params...? n\n";
  result += "    ";
  result += cmdName;
  result += " config name config-params...\n";
  result += "    ";
  result += cmdName;
  result += " cget name";
  
  m_Config.setResult(result);  
}
/*******************************************************************/
/*   Configures an object.  The caller is supposed to have
     validated that an even number of configuration parameters have
     been supplied.

     Parameters:
        interp    - The intepreter that is executing the caller.
	pModule   - Pointer to the module being configured.
	config    - The command doing the configuration.
	firstPair - Index into config of the first keyword/value pair.
                    defaults to 3 which is just right for the create/config
                    subcommands.
  Returns:
    TCL_OK    - The configuration succeeded.
    TCL_ERROR - The configuration failed...and the interpreter result says why.
*/
int
CUserCommand:: configure(CTCLInterpreter&         interp,
			  CReadoutModule*          pModule,
			  std::vector<CTCLObject>& config,
			  int                      firstPair)
{
  string message = "Invalid configuration parameter pair ";

  string key; 
  string value;
  try {
    for (int i =firstPair; i < config.size(); i+= 2) {
      key   = (string)config[i];
      value = (string)config[i+1];
      pModule->configure(key, value);
    }
  }
  catch (CException& e) {

    Usage(configMessage(message, key, value, string(e.ReasonText())),
	  config);
    return TCL_ERROR;
  }
  catch (string msg) {
    Usage(configMessage(message, key, value, msg),
	  config);
    return TCL_ERROR;
  }
  catch (const char* msg) {
    Usage(configMessage(message, key, value, string(msg)),
	  config);
    return TCL_ERROR;
  }
  catch (...) {
    Usage(configMessage(message, key, value, string(" unexpected exception ")),
	  config);
    return TCL_ERROR;
  }

  return TCL_OK;
}
/*************************************************************************/
/*
  Factors the generation of an error message for configuration errors
  out of the various exception handlers:
*/
string
CUserCommand::configMessage(std::string base,
			    std::string key,
			    std::string value,
			    std::string errorMessage)
{
  string message = base;
  message += key;
  message += " ";
  message += value;
  message += " : ";
  message += errorMessage;
 
  return message;

}
  
/**********************************************************************
 * Static member functions.
 **********************************************************************/

/**
 * Add a new driver command to the set recognized by the 
 * configuration subsystem.
 * 
 * @param command - command name.
 * @param pTemplateObject - CReadoutControlHardware pointer to an object that will
 *                          be cloned for the creation of each instance of the driver.
 */
void
CUserCommand::addDriver(std::string command, CReadoutHardware* pTemplateObject)
{
  CConfiguration* pConfig = Globals::pConfig;
  new CUserCommand(*(pConfig->getInterpreter()),
		   *pConfig, command, pTemplateObject);
}
