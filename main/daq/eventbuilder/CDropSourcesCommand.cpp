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
 * @file CDropSourcesCommand.cpp
 * @brief Implement the class that implements the dropSources command.
 */

#include "CDropSourcesCommand.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>

/*-----------------------------------------------------------------------------
** Canonicals:
*/ 

/**
 * Construction is done by the base class.
 *
 * @param interp - The interpreter on which the command is registered.
 * @param cmd    - Name of the command string.
 */
CDropSourceCommand::CDropSourceCommand(CTCLIntepreter& interp, std::string cmd) :
  CTCLObjectProcessor(interp, cmd, true)
{}
/**
* destructor
*  
*  All done by the base class as well.
*/

CDropSourceCommand::~CDropSourceCommand() {}


/*-------------------------------------------------------------------------
 * Public interface:
 */

/**
 * operator()
 *
 *   Executes the EVB::dropSources command.  This preremptorily drops knowledge 
 *   of all data sources and any in-flight fragments.  It's intended only
 *   for last resort error recovery use.
 *
 * @param interp - The interpreter that is executing the command.
 * @param objv   - Vector of command words as encapsulated Tcl_Obj's.
 *
 * @return int 
 * @retval TCL_OK    - success.
 * @retval TCL_ERROR - failure.
 */
int
CDropSourcesCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  try {
    
    // There are no other command line parameters:
    
    requireExactly(objv, 1);
    
    // Get the fragment handle object and request the function

    CFragmentHandler* pHandler = CFragmentHandler::getInstance();
    pHandler->clearQueues();

  }
  catch(std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  return TCL_OK;
}
