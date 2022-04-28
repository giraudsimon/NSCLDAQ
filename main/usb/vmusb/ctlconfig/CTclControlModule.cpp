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
#include "CControlModule.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdint.h>

/**
 * constructor
 *   Just let the base class do it's stuff
 *
 * @param name - name of the module we are creating.
 */
CTclControlModule::CTclControlModule(CTCLInterpreter& interp) :
  CControlHardware(),
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
  configuration.addParameter("-ensemble", NULL, NULL, "");
  
  // call parent to store configuration as m_pConfig
  CControlHardware::onAttach(configuration);

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
 * @param vme - CVMUSB reference that can be used to perform needed VME operations.
 *
 * @throw std::string - if ther are errors...with an error message.
 *
 */
void
CTclControlModule::Initialize(CVMUSB& vme)
{
  std::string baseCommand = m_pConfig->cget("-ensemble");
  if (baseCommand == "") {
    throw std::string("Tcl drivers require an -ensemble option to provide the base command name");
  }

  // Build the command:

  Tcl_Interp*      interp  = m_interp.getInterpreter();

  CTCLObject       command;
  command.Bind(&m_interp);
  command += baseCommand;	// Base of ensemble
  command += "Initialize";      // Subcommand.

  // Turn vme into a marshalled pointer.

  std::string vmusbPointer = swigPointer(&vme, "CVMUSB");
  command += vmusbPointer;

  // Run the command:

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);

  if (status != TCL_OK) {
    throw std::string(Tcl_GetStringResult(interp));
  }
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
 * @param vme - Reference to a CVMUSB object on which vme operations can be done
 * @return std::string -errors from the script.
 */
std::string
CTclControlModule::Update(CVMUSB& vme)
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "Update";

  command += swigPointer(&vme, "CVMUSB");

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  std::string result = std::string(Tcl_GetStringResult(interp));
  if (status != TCL_OK) {
    std::string msg = "ERROR - ";
    msg += result;
    return msg;
  }
  return result;
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
CTclControlModule::Set(CVMUSB& vme, std::string parameter, std::string value)
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "Set";

  command += swigPointer(&vme, "CVMUSB");
  command += parameter;
  command += value;

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  std::string result = Tcl_GetStringResult(interp);

  if (status != TCL_OK) {
    std::string msg = "ERROR - ";
    msg += result;
    return msg;
  }
  return result;

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
CTclControlModule::Get(CVMUSB& vme, std::string parameter)
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "Get";

  command += swigPointer(&vme, "CVMUSB");
  command += parameter;

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  std::string result = Tcl_GetStringResult(interp);

  if (status != TCL_OK) {
    std::string msg = "ERROR - ";
    msg += result;
    return msg;
  }
  return result;

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
/**
 * addMonitorList
 *   Adds elements to the monitor list.
 *   * Marshall vmeList
 *   * Construct the command for the addMonitorList of -ensemble
 *   * Turn errors from the command into an std::string exception.
 * 
 * @param vmeList - reference to the VME operation list that monitors the devices.
 *
 * @throw std::string - Errors from the script.
 */
void
CTclControlModule::addMonitorList(CVMUSBReadoutList& vmeList)
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "addMonitorList";
  command += swigPointer(&vmeList, "CVMUSBReadoutList");

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  if (status != TCL_OK) {
    throw std::string(Tcl_GetStringResult(interp));
  }
}
/**
 * processMonitorList
 *    Called to process our part of the monitor list.
 *    The monitor list is marshalled as a list of bytes.
 *    The result of a successful completion is supposed to be the number of bytes
 *    consumed from this list.
 * 
 * @param pData - Pointer to the unprocessed part of the data.
 * @param remaining - Number of bytes remaining in the pData buffer.
 *
 * @return - pointer to the next unused chunk of pData.
 *
 * @throw std::string - result of command if status was not TCL_OK>
 */
void*
CTclControlModule::processMonitorList(void* pData, size_t remaining)
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "processMonitorList";
  
  CTCLObject* pD = marshallData(&m_interp, pData, remaining);
  command += *pD;

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  delete pD;
  if (status != TCL_OK) {
    throw std::string(Tcl_GetStringResult(interp));
  }

  int used;
  Tcl_GetIntFromObj(interp, Tcl_GetObjResult(interp), &used);

  uint8_t* p = reinterpret_cast<uint8_t*>(pData);
  p += used;

  return reinterpret_cast<void*>(p);

}
/**
 * getMonitoredData
 *    Returns the most recently gotten monitored data.  The idea with
 *    monitored data is that there  are two asynchronous polling processes.
 *    The first is the periodic execution of the list which invokes the driver's
 *    processMonitorList.  That triggers the driver to store away the values 
 *    from that list in some driver specific manner.
 *    The second is the client periodically asking for the monitored data.
 *    That trigers getMonitoredData which is supposed to return the results of
 *    the most recent pass through processMonitorList in a 'convenient' manner.
 *
 * @return std::string 
 */
std::string
CTclControlModule::getMonitoredData()
{
  Tcl_Interp*      interp  = m_interp.getInterpreter();
  CTCLObject       command;
  command.Bind(&m_interp);

  std::string baseCommand = m_pConfig->cget("-ensemble");
  command += baseCommand;
  command += "getMonitoredData";

  int status = Tcl_EvalObjEx(interp, command.getObject(), TCL_EVAL_GLOBAL);
  std::string msg;
  if (status != TCL_OK) {
    msg = "ERROR -  ";

  } else {
    msg = "OK ";
  }
  msg += Tcl_GetStringResult(interp);
  return msg;

}

/*-------------------------------------------------------------------------------
** Private utilities.
*/

/**
 * Generate a swig pointer from the C++ Pointer and its type.
 * This is of the form _address_p_typename
 * @param obj - pointer to the object.
 * @param type - Type name.
 *
 * @return std::string
 */
std::string
CTclControlModule::swigPointer(void* p, std::string  type)
{

  char result [10000];
  std::string hexified;		// Bigendian.

  uint8_t* s = reinterpret_cast<uint8_t*>(&p); // Point to the bytes of the pointer

  // Note that doing the byte reversal this way should be
  // 64 bit clean..and in fact should work for any sized ptr.

  static const char hex[17] = "0123456789abcdef";
  register const unsigned char *u = (unsigned char *) &p;
  register const unsigned char *eu =  u + sizeof(void*);
  for (; u != eu; ++u) {
    register unsigned char uu = *u;
    hexified += hex[(uu & 0xf0) >> 4];
    hexified += hex[uu & 0xf];
  }

  sprintf(result, "_%s_p_%s", hexified.c_str(), type.c_str());

  return std::string(result);


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

