/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CImmediateListCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CVMUSBModule.h"
#include "CVMUSB.h"
#include <usb.h>
#include "Exception.h"
#include <errno.h>

/**
 * @file CImmediateListCommand.cpp
 *
 * Implementation of the CImmdediateList class which provides the
 * vmusb::immdediateList command.  Note that before creating an instance
 * of this object with the default command name, the vmusb namespace must have been
 * created.  By default, the command name is vmusb::immediateList, and the VM-USB
 * used is the first one enumerated see the serial parameter, however.
 * 
 * 
 *
 */


/*--------------------------------------------------------------------
** Canonicals:
*/

/**
 * CImmediateListCommand
 *
 *   Constructs the command object.  In addition a CVMUSB object is created if possible.
 * 
 * @param interp - Reference to the encapsulated interpreter on which the
 *                 command is registered.
 * @param command - The command name under which the object is registered.
 * @param serial  - If this is not a null pointer, it points to the serial 'number' string
 *                  of the desired VM-USB.  If it is NULL, the first VM-USB found will
 *                  be created.
 *
 * @throw std::string - If the VM-USB object cannot be created because there are no
 *                      interfaces or there are no controllers that match the serial
 *                      string provided, the string "No matching VM-USB controllers found"
 */
CImmediateListCommand::CImmediateListCommand(CTCLInterpreter& interp, std::string command, 
					     const char* serial) :
  CTCLObjectProcessor(interp, command, true),
  m_pController(0),
  m_pExecutor(0)
{
  // The controller and executor are created here so that an exception will 
  // get them destroyed appropriately:

  try {
    m_pController = createController(serial);
    m_pExecutor   = new CVMUSBModule("VMUSB");
  
  }
  catch (...) {
    destroyMembers();
    throw;			// Re-throw the error.
  }
}
/**
 * ~CImmediateList
 *
 * Destructor just needs to destroy both the contrroller and executor objects:
 */
CImmediateListCommand::~CImmediateListCommand()
{
  destroyMembers();
}

/*------------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *    Subcommand dispatch.
 *    - All words get bound.
 *    - Ensure there is a subcommand.
 *    - Execute it.
 *
 * @param interp - The encapsulated interpreter that is running the command.
 * @param objv   - Reference to the vector of encapulated Tcl_Obj*'sthat make up the
 *                 command words.
 * @return int
 * @retval TCL_OK - correct completion, with the whatever the subcommand set as the result.
 * @retval TCL_EROR - if there's an error with the reason as the result.
 */
int
CImmediateListCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // Bind the command words to an interp:

  for (int i=0; i < objv.size(); i++) {
    objv[i].Bind(interp);
  }

  // Check that we actually have a subcommand:

  if(objv.size() < 2) {
    interp.setResult("Too few command parameters - missing subcommand\n");
    return TCL_ERROR;
  }
  // Get the subcommand and dispatch it:



  try {

    std::string subcommand = objv[1];
    if ((subcommand != "reconnect")  && !m_pController) {
      interp.setResult("Not connected - issue reconnect first\n");
      return TCL_ERROR;
    }

    if (subcommand == "bulkread") { // Leave this first for best DAQ performance.
      return bulkRead(interp, objv);
 
    }  else if (subcommand == "immediatelist") {

      return immediateList(interp, objv);
    }  else if (subcommand == "writeactionregister") {
      return writeActionRegister(interp, objv);
      
    } else if (subcommand == "serialno") {
      return getSerialNum(interp, objv);
      
    } else if (subcommand == "load") {
      return loadList(interp, objv);
    } else if (subcommand == "reconnect") {
	delete m_pController;
	m_pController = 0;
        CVMUSB* pNewController = createController(NULL); // throws on error
	m_pController = pNewController;
	return TCL_OK;
	
    }  else {
      interp.setResult("Invalid subcommand\n");
      return TCL_ERROR;
    }
  }
  catch (std::string msg) {
    msg += "\n";
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (CException& e) {
    std::string msg = e.ReasonText();
    msg += "\n";
    interp.setResult(msg);
    return TCL_ERROR;
  }
}


/*-----------------------------------------------------------------------------
** Command executors.
*/
/** bulkRead
 *
 * execute the bulkread subcommand.  This takes the following parameters in turn:
 * - size - number of bytes to try to read.
 * - timeout - miliseconds of timeout on the read.
 *
 * @param interp - The encapsualted interpreter.
 * @param objv   - Vector of encapsulated Tcl_Obj*'s that make up the command words.
 * 
 * @return int  
 * @retval TCL_OK - Success, The result is a line of ASCII of the form:
 *                  "n\n"
 *                  This is followed by n bytes of binary data (byte array).
 * @retval TCL_ERROR - Failure.  The result is of the form "errno value\n"
 */
int
CImmediateListCommand::bulkRead(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if (objv.size() != 4) {
    throw std::string("bulkread - incorrect number of command parameters");
  }

  int size      =  objv[2];
  int timeoutMs = objv[3];

  uint8_t buffer[size];
  size_t  bytesTransferred;

  // Try the bulk read:

  int status = m_pController->usbRead(buffer, size, &bytesTransferred, timeoutMs);

  // Handle error case:

  int errorReason;
  if (status < 0) {
    errorReason = errno;
 

    CTCLObject result;
    result.Bind(interp);
    result = errorReason;
    std::string reasonText = static_cast<std::string>(result);
    reasonText += "\n";
    interp.setResult(reasonText);
    return TCL_ERROR;

  }
  // Success
  
  // We're going to need to do this as byte array objs I think.

  char firstLine[1000];
  sprintf(firstLine, "%d\n", bytesTransferred);
  Tcl_Obj* response = Tcl_NewByteArrayObj((const unsigned char*)firstLine, strlen(firstLine));
  Tcl_Obj* data     = Tcl_NewByteArrayObj((const unsigned char*)buffer, bytesTransferred);
  Tcl_AppendObjToObj(response, data);

  interp.setResult(response);

 
  return TCL_OK;
}


/**
 *  immediateList
 *
 *  Execute the command:
 *  - there must be a total of four command words, the command, subcommand
 *    the return data size and the list.
 *  - makeRequestList is used to generate the single Tcl List expected
 *    by the executor.
 *  - The executor is called to run the list.
 * 
 * @param interp - The encapsulated interpreter that is running the command.
 * @param objv   - Reference to the vector of encapulated Tcl_Obj*'sthat make up the
 *                 command words.
 * @return int
 * @retval TCL_OK - correct completion, with the  data bytes read as the list result.
 * @retval TCL_EROR - if there's an error with the reason as the result.
 */
int
CImmediateListCommand::immediateList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // Validate the command length and bind the parameters to the interpreter

  if (objv.size() != 4) {
    interp.setResult("Incorrect number of command line words\n");
    return TCL_ERROR;
  }

  // Create the request list and execute it:

  CTCLObject* pRequest = makeRequestList(interp, 
					 static_cast<int>(objv[2]), 
					 objv[3]);

  // std::stringexceptions are one form of error:

  std::string result;
  try {
    
   result = m_pExecutor->ImmediateList(*m_pController, 
				       static_cast<std::string>(*pRequest));
   delete pRequest;
  }
  catch (std::string msg) {
    delete pRequest;
    msg += "\n";
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (...) {
    delete pRequest;
    interp.setResult(std::string("vmusb immediate list execution threw an unanticpated exception\n"));
    return TCL_ERROR;
  }

  // The result is of the form "Status - stuff"
  // if the status is OK stuff is a Tcl list.
  // if the status is ERROR the stuff is a string.
  //

  CTCLObject resultObj;
  resultObj.Bind(interp);
  resultObj = result;
  CTCLObject status = resultObj.lindex(0);
  status.Bind(interp);
  std::string statusString = static_cast<std::string>(status);


  // Report failure:

  if (statusString != "OK") {
    result += "\n";
    interp.setResult(result);
    return TCL_ERROR;
  }
  // Success:

  std::vector<CTCLObject> outputList;
  outputList = resultObj.getListElements();
  std::string resultString = static_cast<std::string>(outputList[2]);
  resultString += "\n";
  interp.setResult(resultString);
  return TCL_OK;

}
/**
 * writeActionRegister
 *
 *   Write the action register.  The action register value will be the
 *   command word just following the subcommand.
 *
 * @param interp - The encapsulated interpreter that is running the command.
 * @param objv   - Reference to the vector of encapulated Tcl_Obj*'sthat make up the
 *                 command words.
 * @return int
 * @retval TCL_OK - correct completion, no result is returned.
 * @retval TCL_EROR - if there's an error with the reason as the result.
 */
int
CImmediateListCommand::writeActionRegister(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // Validate the number of commands:

  try {
    if (objv.size() != 3) {
      throw std::string("Insufficient parameters, missing action register value");
    }
    int value = objv[2];	// convert to int or throw if it can't.

    std::string result = m_pExecutor->writeActionRegister(*m_pController, 
							  static_cast<uint16_t>(value)); 
    result += "\n";
    interp.setResult(result);
  }
  catch (CException & e) {
    std::string msg = e.ReasonText();
    msg += "\n";
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (std::string msg) {
    msg += "\n";
    interp.setResult(msg);
    return TCL_ERROR;
  }
  return TCL_OK;

}
/**
 * getSerialNum
 *
 *   Fetch the device serial number.
 *
 * @param interp - The encapsulated interpreter that is running the command.
 * @param objv   - Reference to the vector of encapulated Tcl_Obj*'sthat make up the
 *                 command words.
 * @return int
 * @retval TCL_OK - correct completion,result is the serial number string.
 * @retval TCL_EROR - if there's an error with the reason as the result.
 */
int 
CImmediateListCommand::getSerialNum(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if (objv.size() != 2) {
    throw std::string("Incorrect number of command line parameters for serialno");
  }
  std::string serialString = m_pController->serialNo();
  serialString += "\n";
  interp.setResult(serialString);
  
  return TCL_OK;
}

/**
 * Load a list.
 *  This requires a list number, a list offset and the list values themselves.
 *
 * @param interp - Encapsulated interpreter object.
 * @param objv   - Vector of Tcl_Obj*'s which are the command words.
 *
 * @return int
 * @retval TCL_OK - load succeeded.  no command return value.
 * @retval TCL_ERROR - load failed, return value is an error message.
 */
int
CImmediateListCommand::loadList(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if (objv.size() != 5) {
    interp.setResult("Incorrect nubmer of command line parameters in loadList\n");
    return TCL_ERROR;
  }

  int listNum = objv[2];
  int offset  = objv[3];
  CTCLObject* pList = makeRequestList(interp, 0, objv[4]);

  std::string statusMsg = m_pExecutor->loadList(*m_pController, listNum, offset,
						static_cast<std::string>(*pList));
  delete pList;

  // analyze the return.  If this starts with "OK" all is good.

  if (statusMsg.substr(0, 2) == "OK") {
    interp.setResult(std::string("\n"));
    return TCL_OK;
  }

  std::string result = statusMsg.substr(6);
  result += "\n";
  interp.setResult(result); // all the stuff after ERROR -
  return TCL_ERROR;
  
    
}

/*---------------------------------------------------------------------
** Private methods (utilities).
*/

/**
 * makeRequestList
 *
 *  The executor we have expects a single TclList as a parameter.  The first.
 *  list element is the read buffer size and the second is the
 *  list of VME operations.
 *  This method builds that list:
 *
 * @param interp - TCL interpreter.
 * @param readBytes - Size of the read list.
 * @param list      - VME list.
 *
 * @return CTCLObject*
 * @retval pointer to a CTCLObject which was dynamically created and
 *          contains the marshalled list.  The caller must delete this object
 *          to avoid memory leaks.
 *
 */
CTCLObject*
CImmediateListCommand::makeRequestList(CTCLInterpreter& interp, int readBytes, CTCLObject& list)
{
  CTCLObject* pResult = new CTCLObject;
  pResult->Bind(interp);
  CTCLObject& rObj(*pResult);	// easier to use:

  rObj = readBytes;
  rObj += list;

  return pResult;
}

/**
 * createController
 *
 * Create a VM-USB controller.  If a serial number is specified, the controller
 * created must match that serial number otherwise, the first enumerated one is used.
 *
 * @param serial - If NULL, the first enumerated one is used, otherwise, the
 *                 controller matching the requested serial is used.
 *
 * @throw std::string - if the controller can't be found.
 * @note A usb_claim_interface is done on the controller.
 */
CVMUSB*
CImmediateListCommand::createController(const char*  serial)
{
  // first enumerate the available controllers:

  std::vector<struct usb_device*> devs = CVMUSB::enumerate();

  if (devs.size() == 0) {
    throw std::string("No VM-USB controllers are attached to this system");
  }

  // Figure out which one we want:

  struct usb_device* pController(0);
  if(!serial) {
    pController = devs[0];
  } else {
    for (int i = 0; i < devs.size(); i++) {
      if (CVMUSB::serialNo(devs[i]) == serial) {
	pController = devs[i];
	break;
      }
    }
  }
  if (!pController) {
    throw std::string("No matching VM-USB found");
  }

  try {
    return new CVMUSB(pController);
  }
  catch (const char* pMessage) {
    throw std::string(pMessage);
  }
  
}
/**
 * Factored out code to destroy our dynamically allocated stuff
 */
void
CImmediateListCommand::destroyMembers()
{
  delete m_pController;
  delete m_pExecutor;
}
