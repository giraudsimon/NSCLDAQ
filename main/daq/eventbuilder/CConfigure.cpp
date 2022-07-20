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
/**
 * @file CConfigure.cpp
 * @brief Implement class for evb configuration management.
 */

#include "CConfigure.h"
#include "CFragmentHandler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>


/*---------------------------------------------------------------------------------
** Canonicals

/**
 * Constructor:
 *
 * Construct the command processor
 *
 * @param interp - the interpreter on which the command processor will be registered.
 * @param commandName - the command word.
 *
 */
CConfigure::CConfigure(CTCLInterpreter& interp, std::string commandName) :
  CTCLObjectProcessor(interp, commandName, true)
{}

/**
 * Destructor:
 *
 *  Called when the command object is being deleted.
 */
CConfigure::~CConfigure()
{
}

/*------------------------------------------------------------------------------
 *
 * Command processor.
 */

/**
 * operator()
 *
 *  Called in response to the commansd.  Our job is to 
 *  pull apart the structure of the command and dispatch to the 
 *  Set or Get method depending on the ensemble command actually issued.
 *
 * @param interp - The interpreter that is executing the command.
 * @param objv   - Vector of references to encapsulated parameter objects.
 *
 * @return int - TCL_OK on success or TCL_ERROR on failure.
 */
int
CConfigure::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // We need at leats three parameters...command, subcommand, and parameter name.

  try {
    requireAtLeast(objv, 3, " Invalid request length");
    bindAll(interp, objv);

    std::string subcommand = objv[1];
    std::string name       = objv[2];
    
    if (subcommand == "set") {
      requireExactly(objv, 4);

      return Set(interp, name, objv[3]);

    } else if (subcommand == "get") {
      requireExactly(objv, 3);
      return Get(interp, name);

    } else {
      std::string errorMessage = "Illegal subcommand: ";
      errorMessage += subcommand;
      throw errorMessage;
    }
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (const char* pMsg) {
    interp.setResult(pMsg);
    return TCL_ERROR;
  }
  catch (CException& e) {
    interp.setResult(e.ReasonText());
    return TCL_ERROR;
  }
  catch (...) {
    interp.setResult("Unaniticipated error type in CConfigure::operator");
    return TCL_ERROR;
  }

  return TCL_OK;
}
/*--------------------------------------------------------------------------------
** Private utilities.
*/


/**
 * Set
 *
 * Set a configuration value.
 *
 * @param interp - the intepreter that is running this command.
 * @param name  -  Name of the configuration value to set.
 * @param value -  Proposed value of the configuration parameter.
 *
 * @return int - TCL_OK on success else throws an exception that is caught by
 *               the caller, turned ito an interpreter result and TCL_ERROR return.
 */
int
CConfigure::Set(CTCLInterpreter& interp, std::string name, CTCLObject& value)
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();

  // Dispatch accoring to the name:

  if (name == "window") {

    // For window the value must be a number > 0:

    int window = value;
    if (window <= 0) {
      std::string errorMsg = "Build time window must be > 0 was ";
      errorMsg += static_cast<std::string>(value);

      throw errorMsg;
    }
    pHandler->setBuildWindow(static_cast<time_t>(window));
    
  } else if (name == "XoffThreshold") {
    
    int size = value;
    pHandler->setXoffThreshold(size);
    
  } else if (name == "XonThreshold") {
    
    int size = value;
    pHandler->setXonThreshold(size);
    
  } else if (name == "perQXoffThreshold") {

    int size = value;
    pHandler->setPerQXoffThreshold(size);

  } else if (name == "perQXonThreshold") {

    int size = value;
    pHandler->setPerQXonThreshold(size);

  
  } else {
    std::string errorMsg = "Illegal configuration parametr name: ";
    errorMsg  += name;
    throw errorMsg;
  }
  return TCL_OK;
}
/**
 * Get
 *
 *  Get a configuration value.
 *
 * @param interp - interpreter running the command.
 * @param name   - Name of the configuration parameter to get.
 *
 * @return int - TCL_OK on success, throws an exception on error.
 *
 * @note the actual configuration parameter value is set as the interpreter result.
 */
int
CConfigure::Get(CTCLInterpreter& interp, std::string name)
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();

  // dispatch according to the parameter name:

  if (name =="window") {
    time_t value = pHandler->getBuildWindow();
    CTCLObject oValue;
    oValue.Bind(interp);
    oValue = static_cast<int>(value);
    interp.setResult(oValue);
  } else {
    std::string errorMsg = "Illegal configuration parameter: ";
    errorMsg += name;
    throw name;
  }

  return TCL_OK;
}
