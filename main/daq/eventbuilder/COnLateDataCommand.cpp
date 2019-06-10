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
 * @file COnLateDataCommand.cpp
 * @brief Implementation of the onDataLate command processor.
 */

#include "COnLateDataCommand.h"
#include "CLateScriptObserver.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>


/*-----------------------------------------------------------------------------
**
* Canonicals:
*/

/**
 * constructor:
 *
 *   - Register the command (base constructor)
 *   - null out the dispatcher object indicating there are no scripts.
 *
 * @param interp - The encapsulated interpreter object.
 * @param command - The command name.
 *
 */
COnLateDataCommand::COnLateDataCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true),
  m_pDispatcher(0)
{}
/**
 * destructor:
 *
 *  The base class destructor unregisters the command.  Delete any m_pDispatcher
 *  this is zeroed when deleted so an unconditional delete is ok
 */
COnLateDataCommand::~COnLateDataCommand()
{
  delete m_pDispatcher;
}
/*----------------------------------------------------------------------------
** Public method implementations.
*/

/**
 * operator()
 *
 *  Command execution:
 *   - There must be at most two command words.
 *   - The second word, if present is the new script, unless it's empty in which
 *     case there is no new script.
 * 
 * @param interp - Encapsulated interpreter that is running the command.
 * @param objv   - vector of encapsualted Tcl_Obj* that make up the command words.
 *
 * @return int
 * @return TCL_OK - Successful completion.  The result is the prior command script.
 *                  This is an empty string if there was no prior string (so push/pop
 *                  stacks are simple to implement).
 * @return TCL_ERROR - Failure (only really possible if there are too many paramters).
 */
int
COnLateDataCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // Validate parameter count:

  try {
    requireAtMost(objv, 2);
    bindAll(interp, objv);

    // The result is always the current command string:

    std::string result = "";	// Though it may be empty:
    if (m_pDispatcher) {
      result = m_pDispatcher->getScript();
    }
    interp.setResult(result);

    // If the argument count is 2, the current observer is deleted and possibly
    // replaced:

    if (objv.size() == 2) {
      delete m_pDispatcher;
      m_pDispatcher = 0;
      std::string newScript = static_cast<std::string>(objv[1]);
      if (newScript != "") {
	m_pDispatcher = new CLateScriptObserver(interp, newScript);
      }
    }
  }
  catch  (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }

  return TCL_OK;
}
