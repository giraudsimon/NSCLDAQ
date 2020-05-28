/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CDeviceCommand.cpp
# @brief  Implement device command shared/common utilities.
# @author <fox@nscl.msu.edu>
*/

#include "CDeviceCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CReadoutModule.h>
#include <string>
#include <Exception.h>
#include "CConfiguration.h"
#include "tclUtil.h"


/**
 *  Constructor
 *
 * @param interp - References the interpreter on which the command is being
 *                 defined.
 * @param command - name of the command.
 */
CDeviceCommand::CDeviceCommand(
  CTCLInterpreter& interp, const char* command,
  CConfiguration& config
) :
  CTCLObjectProcessor(interp, command, true),
  m_Config(config)
{}

/**
 * operator()
 *    Dispatch to the appropriate sub-command handler based on the first
 *    command parameter.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - vector of command keywords.
 *  @return int   - TCL_OK if the command succeeded.  Result dependson subcmd.
 *  @retval TCL_ERROR - If the command failed in which case normally the result
 *                  is a human readable error message.
 */
int
CDeviceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // require at least 3 parameters.

  if (objv.size() < 3) {
    Usage(interp, "Insufficient command parameters", objv);
    return TCL_ERROR;
  }
  // Get the subcommand keyword and dispatch or error:

  std::string subcommand = objv[1];
  if (subcommand == std::string("create")) {
    return create(interp, objv);
  }
  else if (subcommand == std::string("config")) {
    return config(interp, objv);
  } 
  else if (subcommand == std::string("cget")) {
    return cget(interp, objv);
  }
  else {
    Usage(interp, "Invalid subcommand", objv);
    return TCL_ERROR;
  }  
}
/**
 * config
 *    Top level configuration processing
 *
 * @param interp - interpreter executing the command
 * @param objv   - The command words
 * @return int   - The usual TCL_OK/TCL_ERROR returns.
 * 
 */
int
CDeviceCommand::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  
  CReadoutModule* pModule = tclUtil::getModule(
    m_Config, interp, objv, (objv.size() < 5)
  );
  if (!pModule) return TCL_ERROR;
  
  return configure(interp, pModule, objv);
}
/**
 * cget
 *    Return a list that describes the configuration of an object.
 *
 *  @param interp  - interpreter that is executing  the command.
 *  @param objv    - the command line words.
 *  @return int    - usual TCL_OK/TCL_ERROR status.
 */
int
CDeviceCommand::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  
  CReadoutModule* pModule = tclUtil::getModule(
    m_Config, interp, objv, objv.size() != 3
  );
  if (!pModule) return TCL_ERROR;
  tclUtil::listConfig(interp, pModule);
  
  return TCL_OK;

}
/**
 * configure
 *    Handle configuration option processing at both create and config
 *    time.
 *
 *  @param interp  - interpreter that executes the device commands.
 *  @param pModule - Pointer to the readout module being configured.
 *  @param config  - keyword value pair list that configures the module.
 *  @param firstPair - Index in to the list of the first pair.  By defaulting
 *                     to 3, the default case is the config command.
 *                     by specifying a different index, create time
 *                     configuration is also supported.
 *  @return int    - TCL_OK if all the configuration options processed.
 *  @retval TCL_ERROR If there was a problem processing the config opts.
 *                    In this case, configMessage below is uased to set a
 *                    tentative value for the interpreter result.
 */
int
CDeviceCommand::configure(
    CTCLInterpreter& interp, CReadoutModule* pModule,
    std::vector<CTCLObject>& config, int firstPair
)
{
  std::string message = "Invalid configuration parameter pair ";

  // Ensure the number of remaining configuration parameters is even:
  
  if ((config.size() - firstPair) % 2) {
    interp.setResult("For every command option there must be a value");
    return TCL_ERROR;
  }
  
  std::string key; 
  std::string value;
  try {
    for (int i =firstPair; i < config.size(); i+= 2) {
      key   = (std::string)config[i];
      value = (std::string)config[i+1];
      pModule->configure(key, value);
    }
  }
  catch (CException& e) {

    Usage(interp, configMessage(message, key, value, std::string(e.ReasonText())),
	  config);
    return TCL_ERROR;
  }
  catch (std::string msg) {
    Usage(interp, configMessage(message, key, value, msg),
	  config);
    return TCL_ERROR;
  }
  catch (const char* msg) {
    Usage(interp, configMessage(message, key, value, std::string(msg)),
	  config);
    return TCL_ERROR;
  }
  catch (...) {
    Usage(interp, configMessage(message, key, value, std::string(" unexpected exception ")),
	  config);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/**
 *  configMessage
 *     Generates/returns a message for an invalid configuration parameter or
 *     value.
 *
 *   @param base - the first part of the message
 *   @param key  - Configuration keyword that sparked the message.
 *   @param value - Value of that configuration value.
 *   @param errorMessage - tail of the message.
 *   @return std::string
 */
std::string
CDeviceCommand::configMessage(
    std::string base, std::string key, std::string value,
    std::string errorMessage
)
{
  std::string message = base;
  message += " ";
  message += key;
  message += " ";
  message += value;
  message += " : ";
  message += errorMessage;
 
  return message;

}
/**
 * Usage
 *    Sets the interpreter result to a usage string.
 *
 *  @param interp - Intepreter whose result gets set.
 *  @param msg    - Base message.
 *  @param objv   - Command word vector.
 */
void
CDeviceCommand::Usage(
  CTCLInterpreter& interp, std::string msg, std::vector<CTCLObject> objv
)
{
  std::string cmdName = objv[0];
  std::string usage("Usage\n");
  usage += "    ";
  usage += cmdName;
  usage += "  create name value\n";
  usage += "    ";
  usage += cmdName;
  usage += " config name config-params...\n";
  usage += "    ";
  usage += cmdName;
  usage += " cget name";
  
  tclUtil::Usage(interp, msg, objv, usage); 
}
