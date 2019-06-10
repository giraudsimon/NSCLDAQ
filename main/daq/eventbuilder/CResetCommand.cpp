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
 * @file CResetCommand.cpp
 * @brief implementation of the class that resets timestamps in the event builder.
 */


#include "CResetCommand.h"
#include "CFragmentHandler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>


/*----------------------------------------------------------------------------------------
** Canonicals:
*/

/**
 * constructor
 *
 * The base class constructor does all the hard work:
 *
 * @param interp - interpreter on which the command is registered.
 * @param cmd    - Name of the command.
 */
CResetCommand::CResetCommand(CTCLInterpreter& interp, std::string cmd) :
  CTCLObjectProcessor(interp, cmd, true)
{}
/**
 * destructor
 *
 * Base class destructor does all the work.
 */

CResetCommand::~CResetCommand() {}

/*-------------------------------------------------------------------------------------
** command processor.  There are no command parameters.
*/

/**
 * Execute the command.
 * 
 * @param interp - the interpreter executing the command.
 * @param objv   - Vector of command parameters.
 * 
 * @return int 
 * @retval TCL_OK - success.
 * @retval TCL_ERROR - failed.
 */
int
CResetCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{

  try {
    requireExactly(objv, 1);
    
    
    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->resetTimestamps();
  }
  catch(std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  return TCL_OK;

}

