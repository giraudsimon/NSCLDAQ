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
 * @file CFlushCommand.cpp
 * @brief Implement the class that executes the  EVB::flush  - flush event queues.
 */

#include "CFlushCommand.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>


/*--------------------------------------------------------------------------------
* Canonical methods:
*/

/**
 * constructor
 *
 * Create the command processor.  All the work is done by the base class
 * constructor.
 * 
 * @param interp - Interpreter on which the command is regsitered.
 * @param commandName - Name of command.
 */
CFlushCommand::CFlushCommand(CTCLInterpreter& interp, std::string commandName) :
  CTCLObjectProcessor(interp, commandName, true)
{}
/**
 * destructor
 *
 *  Destroys the command.  This is reaully done by the base class destructor.
 */
CFlushCommand::~CFlushCommand() {}

/*----------------------------------------------------------------------------------
 * public methods.
 */

/**
 * operator()
 *
 * Invoked to execute the command
 * - ensure there are now parameters.
 * - Get the event ordering singleton
 * - Invoke its flush method.
 * 
 * @param interp - the interpreter.
 * @param objv   - The command line parameters [0] is the command itself.
 *
 * @return int
 * @return TCL_ERROR - Error processing the command.
 * @return TCL_OK    - Successful command processing.
 */
int 
CFlushCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{

  try {
    requireExactly(objv, 1);
    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->flush();
    return TCL_OK;
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
}
