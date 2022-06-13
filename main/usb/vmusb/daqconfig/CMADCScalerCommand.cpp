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
#include <CMADCScalerCommand.h>
#include <CMADCScaler.h>

#include <TCLInterpreter.h>
#include <CConfiguration.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>


using std::string;
using std::vector;

/////////////////////////////////////////////////////////////////////
// Constructors and other implemented canonicals:

/*!
  Construct the madc command and register it.
  \param interp - The TCL interpreter on which the command is registered.
  \param config - Reference tothe configuration (database of known modules/stacks etc).
  \param commandName - Name of the command to register, defaults to "madc"
*/
CMADCScalerCommand::CMADCScalerCommand(CTCLInterpreter& interp,
			   CConfiguration&  config,
			   std::string      commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_Config(config)
{}

/*!
   destructor:
*/
CMADCScalerCommand::~CMADCScalerCommand()
{}

////////////////////////////////////////////////////////////////////////////////
// Command processing.

/*!  

  Called when the command is encountered in a script.
  - Ensures there are at least 3 arguments, the command, subcommand and
    name of an item.
  - Dispatches to the appropriate sub-comand processor or
  - returns an error if the subcommand keyword is not recognized.

  \param CTCLInterpreter& interp   - Interpreter running the command.
  \param std::vector<CTCLObject>& objv - Command line words.

  \return int
  \retval TCL_OK  - Everything worked.
  \retval TCL_ERROR - Command failed.

  

*/

int
CMADCScalerCommand::operator()(CTCLInterpreter& interp,
			 vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 3) {
    Usage(string("Insufficient command parameters"), objv);
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
/*!
   Create a module.
*/
int
CMADCScalerCommand::create(CTCLInterpreter& interp, vector<CTCLObject>& objv)
{
  // We already know we have at least three elements...

  string name = objv[2];

  // ensure the name is unique:

  CReadoutModule* pModule = m_Config.findAdc(name);
  if(pModule) {
    Usage("Duplicate moduel creation attempted", objv);
    return TCL_ERROR;
  }

  // Tentatively create the module, and bind it to a configuration.

  CMADCScaler* pAdc   = new CMADCScaler;
  pModule         = new CReadoutModule(name, *pAdc);

  // If there are additional parameters, attempt to configure the module
  

  int status = TCL_OK;		// If no config, then configuration worked ;-)

  if (objv.size() > 3) {
    status = configure(interp, objv, pModule);
  }

  // If the configuration failed, back off; otherwise enter the module in the configuration 
  // database.

  if (status != TCL_OK) {
    delete pModule;
    delete pAdc;
  }
  else {
    m_Config.addAdc(pModule);
    m_Config.setResult(name);
  }

  return status;
}
/*!
  Configure a module.
*/
int
CMADCScalerCommand::config(CTCLInterpreter& interp,
		     vector<CTCLObject>& objv)
{
  if (objv.size() < 4) {
    Usage("Insufficient parameters to do a configuration", objv);
    return TCL_ERROR;
  }


  // Locate the module:

  string name = objv[2];
  CReadoutModule* pModule = m_Config.findAdc(name);
  if (!pModule) {
    Usage("Module does not exist", objv);
    return TCL_ERROR;
  }
  // Configure it according to the remaining parameters.

  return configure(interp, objv, pModule);
}
/*!
  Get the configuration of a moduile
*/
int
CMADCScalerCommand::cget(CTCLInterpreter& interp,
		   vector<CTCLObject>& objv)
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
  Return the correct usage for the command.
*/
void
CMADCScalerCommand::Usage(string msg, std::vector<CTCLObject>& objv)
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
  result += "   madcscaler create name ?config?\n";
  result += "   madcscaler config name config...\n";
  result += "   madcscaler cget name";

  m_Config.setResult(result);

}
/*
   Configure the module.
*/
int
CMADCScalerCommand::configure(CTCLInterpreter& interp,
			std::vector<CTCLObject>& objv,
			CReadoutModule* pModule,
			int startAt)
{
  // See if there are an even number of keyword value pairs:

  int nWords        = objv.size();
  int nConfigItems  = nWords - startAt;
  if(nConfigItems % 2) {
    Usage("For each configuration keyword there must be a value too", objv);
    return TCL_ERROR;
  }
  try {
    while(startAt < nWords) {
      string  key = objv[startAt];
      string  val = objv[startAt+1];
      pModule->configure(key, val);
      startAt += 2;
    }
  }
  catch (string msg) {		// a config option failed.
    Usage(msg, objv);
    return TCL_ERROR;
  }
  return TCL_OK;

}
