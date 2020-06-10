/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CTclControlModule.h"
#include "CCtlConfiguration.h"
#include "CControlModule.h"
#include "CCCUSB.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Globals.h>
#include <tclUtil.h>
#include <stdint.h>

/**
 * constructor
 *   Just let the base class do it's stuff
 *
 * @param name - name of the module we are creating.
 */
CTclControlModule::CTclControlModule(CTCLInterpreter& interp) 
  :
  CControlHardware() ,
  m_interp(interp)
{
}


/**
 * onAttach
 *   Register the string configuration parameter -ensemble
 *   which will be the base name of the command ensemble we
 *   are wrapping.
 *
 * @param config - reference to our configuration.
 */
void
CTclControlModule::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
  configuration.addParameter("-ensemble", NULL, NULL, "");

}
/**
 * Initialize
 *
 *   Invoked after the configuration script has been processed.
 *   *  The -ensemble option better not be empty.
 *   *  Marshall the VMUSB object as a swig object.
 *   *  Invoke the Initialize subcommand of the ensemble passing it the
 *      marshalled VMUSB object
 *   *  Error returns from the script get turned into a string exception
 *      for the Interpreter result.
 *
 * @param vme - CCCUSB reference that can be used to perform needed VME operations.
 *
 * @throw std::string - if ther are errors...with an error message.
 *
 */
void
CTclControlModule::Initialize(CCCUSB& vme)
{
  CTCLObject command;
  makeCommand(command, vme, "Initialize");
  executeCommand(command);
  
}

/**
 * Update
 *   Perform the update operation as requested by the client.
 *   Since Initialize has been called we can assume -ensemble exists.
 *   * Fetch the command from -ensemble
 *   * Append Update
 *   * Append a marshalled vmusb pointer
 *   * Execute the command turning failures into exceptions.
 *
 *
 * @param vme - Reference to a CCCUSB object on which vme operations can be done
 * @return std::string -errors from the script.
 */
std::string
CTclControlModule::Update(CCCUSB& vme)
{
  CTCLObject command;
  makeCommand(command, vme, "Update");
  
  return executeCommand(command);
  

}
/**
 * Set
 *   Perform a Set operation  as directed by the client.
 *   The Set method of the -ensemble  is called with the paramters:
 *   * Marshalled VMUSB controller.
 *   * parameter name
 *   * value
 *   Any error from the script is turned into an ERROR return.
 *   Success returns "OK"
 * 
 * @param vme       - VME controller object.
 * @param parameter - name of the parameter to set.
 * @param value     - new parameter value.
 *
 * @return std::string
 * @retval "ERROR -  msg" - script did not return TCL_OK msg is the result.
 * @retval other - The string returned from script.
 */
std::string
CTclControlModule::Set(CCCUSB& vme, std::string parameter, std::string value)
{
  CTCLObject command;
  makeCommand(command, vme, "Set");
  
  command += parameter;
  command += value;

  return executeCommand(command);

}
/**
 * Get
 *   Perform a Get operation as directed by the client.
 *   The Get method of the -ensemble is called with the following parameters:
 *   *  Marshalled VME controller.
 *   *  Parameter name.
 *   Any script error is turned into an ERROR return.
 *
 * @param vme - VME controller object 
 * @param parameter - Name of the parameter to retrieve.
 *
 * @return std::string
 * @retval "ERROR - msg " - the script returned other than TCL_OK and msg as the result.
 * @retval other - the result from the script on TCL_OK return status.
 */
std::string
CTclControlModule::Get(CCCUSB& vme, std::string parameter)
{
  CTCLObject command;
  makeCommand(command, vme, "Get");
  
  command += parameter;

  return executeCommand(command);
}
/**
 * clone
 *    We have no information of our own, let the base class do this.
 */
CControlHardware*
CTclControlModule::clone() const
{
  return (new CTclControlModule(*this));
}

/*-------------------------------------------------------------------------------
** Private utilities.
*/

/**
 * makeCommand
 *    All commands are of the form:
 *    ensemble-name subcommand controller-object
 *
 *    There may or may not be additioanl command words
 *    depending on the subcommand.  This method
 *    creates a CTCLObject that contains that part of the
 *    command.
 * @param[out] command - object into which the command is generated.
 * @param  usb    - the usb controller.
 * @param subcommand   - Subcommand string.
 * @note command is a reference and will have been bound
 *               to the interpreter on exit.
*/
void
CTclControlModule::makeCommand(
  CTCLObject& command, CCCUSB& usb, const char* subcommand
)
{
  std::string baseCommand = m_pConfig->cget("-ensemble");
  if (baseCommand == "") {
    throw std::string("Tcl drivers require an -ensemble option to provide the base command name");
  }

  // Build the command:
  
  CTCLInterpreter* pInterp = &m_interp;

  command.Bind(pInterp);
  command += baseCommand;	// Base of ensemble
  command += subcommand;

  // Turn vme into a marshalled pointer.

  std::string vmusbPointer = tclUtil::swigPointer(&usb, "CCCUSB");
  command += vmusbPointer;  
}
/**
 * executeCommand
 *    Run a generated command object and return the result
 *    or throw an exception if there's a failure.
 * @param command - the generated commando bject.
 * @return std::string - commnand result.
 */
std::string
CTclControlModule::executeCommand(CTCLObject& command)
{
  Tcl_Interp* pInterp = m_interp.getInterpreter();
  int status = Tcl_EvalObjEx(pInterp, command.getObject(), TCL_EVAL_GLOBAL);
  std::string result = Tcl_GetStringResult(pInterp);

  if (status != TCL_OK) {
    std::string msg = "ERROR - Executing  command";
    msg += std::string("command");
    msg += ": ";
    msg += result;
    msg += ": ";
    msg += XXUSB::getTclTraceback(pInterp);
    return msg;
  }
  return result;
}

/**
 * marshallData
 *   Marshall a block of byte data into a Tcl List in a 
 *   CTCLObject that is new-ed into existencde.
 *
 * @param pInterp - Pointer to an object wrapped interpreter.
 * @param pData   - Pointer to the block of data.
 * @param nBytes  - Number of bytes in pData.
 *
 * @return CTCLObject* - Pointer to a dynamically allocated 
 *                       wrapped object containing the bytes.
 */
CTCLObject*
CTclControlModule::marshallData(CTCLInterpreter* pInterp, void* pData, size_t nBytes)
{
  CTCLObject* pResult = new CTCLObject;
  pResult->Bind(pInterp);

  uint8_t* p = reinterpret_cast<uint8_t*>(pData);
  for (int i =0; i < nBytes; i++) {
    int item = static_cast<int>(*p++);
    (*pResult) += item;
  }
  return pResult;
}

