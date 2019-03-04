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
 * @file CBarrierTraceCommand.cpp
 * @brief Implement the CBarrierTraceCommand class; provides a barriertrace command.
 */

#include "CBarrierTraceCommand.h"

#include "CCompleteBarrierCallback.h"
#include "CPartialBarrierCallback.h"

#include <Exception.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>

/*-------------------------------------------------------------------------
** Canonical methdod implementation.
*/

/**
 * constructor
 *  
 *  The base class registers us withthe interpreter, We just need to null
 *  out the observers we maintain.
 *
 * @param interp - Reference to the interpreter on which this should be registered.
 * @param command - Name of the desired command string.
 */
CBarrierTraceCommand::CBarrierTraceCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true),
  m_pComplete(0),
  m_pPartial(0)
{}

/**
 * destructor
 *
 *   Get rid of and hence unregister all the active observer.
 */
CBarrierTraceCommand::~CBarrierTraceCommand () 
{
  delete m_pComplete;
  m_pComplete = 0;
}
/*----------------------------------------------------------------------------
** Public entries.
*/

/**
 * operator()
 * 
 * Command executor.  Just need to pull out the command words (must be either
 * 2 or 3 of them, select the correct observer and call the appropriate templated
 * type of newCallback.
 *
 * @param interp - the interpreter executing the command.
 * @param objv   - The command words as encapsulated Tcl_Obj*'s.
 *
 * @return int
 * @retval TCL_OK - Correct completion the result of the command is the prior script.
 * @retval TCL_ERROR - field completion, the result is the error text.
 */
int
CBarrierTraceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{

  try {
    // Parameter number validation and interpreter bindings:

    requireAtLeast(objv, 2);
    requireAtMost(objv, 3);
    bindAll(interp, objv);

    // Get the subcommand and the script (const char* 0 if not present):

    std::string tracetype = objv[1];
    const char* script (0);
    if (objv.size() == 3) {
      script = static_cast<std::string>(objv[2]).c_str();
    }

    // fan out based on the contents of tracetype:

    if (tracetype == "complete") {
      interp.setResult(newCallback(&m_pComplete, script));
    }
    else if (tracetype == "partial") {
      interp.setResult(newCallback(&m_pPartial, script));
    } else {
      throw std::string("Invalid barrier trace type");
    }

  }
  catch (std::string msg) {
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
  catch (...) {
    interp.setResult("Unanticipated exception from barriertrace");
    return TCL_ERROR;
  }


  return TCL_OK;

}
