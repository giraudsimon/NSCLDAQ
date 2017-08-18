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
#include "CLRS2249Command.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <CLRS2249.h>
#include <Exception.h>

using namespace std;

//   Implementation of the CLRS2249Command class.

/**************************************************************************/
/*                 Implementations of canonical methods                   */
/**************************************************************************/

/*!
  Construct the command and register it with the interpreter.
  Registration is done by the base class constructor.

   \param interp : CTCLInterpreter&
       Tcl interpreter on which the command will be registered.
   \param config : CConfiguration& config
       The configuration of STACKs that will be manipulated by this command.
   \param commandName std::string
       Name of the command to register.
*/
CLRS2249Command::CLRS2249Command(CTCLInterpreter& interp,
			     CConfiguration& config,
			     std::string     commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{
}

/*!
   The base class destructor will unregister us from the interpreter:
*/
CLRS2249Command::~CLRS2249Command()
{
}

/*************************************************************************/
/*                           Command Processing                          */
/*************************************************************************/

/*************************************************************************/
/*!
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

  \note  The interpreter's result will be set to one of the following
         depending on the return value:
	 - Return TCL_OK    : the result will be dependent on the actual processor.
	 - Return TCL_ERROR : the result will be a human readable error message.

*/
int
CLRS2249Command::operator()(CTCLInterpreter& interp,
			  std::vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

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

/****************************************************************************/
/*
   Process the create subcommand:
   - Ensure that we have enough values on the command line.  We need
     the command, the subcommand (already determined to be "create"), the
     name of the scaler and at least one configuration pair 
     is set) for a total of 5 parameters minimum, and an odd number to ensure
     the configuration parameters are paired.
   - Ensure that the first configuration pair is a -slot configuration so that
     the module is gaurenteed to live somewhere in the crate.
   - Ensure the module does not yet exist.
   - Create a new SCALER module.
   - Add it to the configuration.
   - Process the remainder of the command line as configuration pairs.

   Parameters:
     CTCLInterpreter&    interp   - Interpreter that is executing this command.
     vector<CTCLObject>& objv     - Vector of command words.
  Returns:
    int: 
       TCL_OK      - Command was successful.
       TCL_ERROR   - Command failed.

  Side Effects:
     If the return value is TCL_OK, the name of the module is returned.
     If the return value is TCL_ERROR, an error message describing the problem is
     returned.  Note that the configuration will catch all exceptions  and
     turn them into error strings and TCL_ERROR returns.
*/
int
CLRS2249Command::create(CTCLInterpreter& interp,
		       std::vector<CTCLObject>& objv)
{
  // Need to make sure the word count of the command is valid.

  if ((objv.size() < 5) || ((objv.size() & 1) == 0)) {
    Usage("Invalid number of command parameters, must be at least 5 and odd",
	  objv);
    return TCL_ERROR;
  }
  // Ensure the first configuration parameter is "-slot"
  string conf1 = objv[3];
  if (conf1 != "-slot") {
    Usage("First configuration parameter must be for the -slot", objv);
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

  CLRS2249* pScaler   = new CLRS2249;
  pModule        = new CReadoutModule(name, *pScaler); // Also attaches pScaler to configuration.

  int status     = configure(interp,
			     pModule,
			     objv); 
  if (status == TCL_OK) {
    m_Config.addAdc(pModule);
    m_Config.setResult(name);
  }
  else {
    delete pModule;
    delete pScaler;
  }
  return status;
}

/****************************************************************************/
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
CLRS2249Command::config(CTCLInterpreter& interp,
		      std::vector<CTCLObject>& objv)
{
  // Ensure the parameter counts are valid:

  if ((objv.size() < 5) || ((objv.size() & 1) == 0)) {
    Usage("Incorrect number of command parameters for config", objv);
    return TCL_ERROR;
  }

  // Get the moduel name and locate it.. it's an error for the module to not exist.

  string          name     = objv[2];
  CReadoutModule* pModule  = m_Config.findAdc(name);
  if(!pModule) {
    Usage("lrs2249 module does not exist", objv);
    return TCL_ERROR;
  }
  // and configure:

  m_Config.setResult(name);	// This gets overwritten in case of error.
  return configure(interp, pModule, objv);

}
/***************************************************************************/
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
CLRS2249Command::cget(CTCLInterpreter& interp,
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
/*!
   Return the confguration.
*/
CConfiguration*
CLRS2249Command::getConfiguration()
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
CLRS2249Command::Usage(std::string msg, std::vector<CTCLObject> objv)
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
  result += "    lrs2249 create name base-address\n";
  result += "    lrs2249 config name config-params...\n";
  result += "    lrs2249 cget name";
  
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
CLRS2249Command:: configure(CTCLInterpreter&         interp,
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
CLRS2249Command::configMessage(std::string base,
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
  
