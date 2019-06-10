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
 * @file CDeadCommand.cpp
 * @brief Implementation of the CDeadCommand (deadsource command processor) class.
 */

#include "CDeadSourceCommand.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <TCLObject.h>
#include <Exception.h>

/*-----------------------------------------------------------------------------------
** Implementations of canonical methods.
*/

/**
 * constructor
 *
 *  Constructs the command using the base class constructor.
 *
 * @param interp  - Interpreter on which the command will be registered.
 * @param command - Command name string.
 *
 */
CDeadSourceCommand::CDeadSourceCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true)
{
  
}
/**
 * destructor
 * 
 * The base class unregisters us from the interpreter.
 */
CDeadSourceCommand::~CDeadSourceCommand() {}

/*------------------------------------------------------------------------------------
** Implementation of public methods:
*/

/**
 * operator()
 *
 *   Called to process the command:
 *   - There shouild be one parameter in addition to the command word.
 *   - The word should decode as a integer.
 *   - The source id is passed to CFragmentHandler::markSourceFailed
 *
 * @param interp - Interpreter that's executing the command.
 * @param objv   - Command words making up the object.
 *
 * @return int   - TCL_OK on success, TCL_ERROR on failure.
 */
int
CDeadSourceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  try {
    requireExactly(objv, 2);
    bindAll(interp, objv);

    std::string socketString = static_cast<std::string>(objv[1]);

    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->markSocketFailed(socketString);

  }
  catch(std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  } 
  catch (const char* msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (CException& e) {
    interp.setResult(e.ReasonText());
    return TCL_ERROR;
  }
  catch(...) {
    interp.setResult("Exception caught");
    return TCL_ERROR;
  }

  return TCL_OK;
}
