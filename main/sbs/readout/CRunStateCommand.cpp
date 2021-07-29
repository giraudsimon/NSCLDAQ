/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CRunStateCommand.cpp
 *  @brief:  Implement the 'runstate' command.
 *  
 */
#include "CRunStateCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include "RunState.h"
#include <stdexcept>
#include <string>

/**
 * constructor
 *   @param interp  - references the interpreter on which the command will be
 *                    registered.
 *   @param command - Name of the command "runstate" for compatibility with other
 *                    readout frameworks and the REST plugin.
 *   @param pState  - Pointer to the run state object.
 */
CRunStateCommand::CRunStateCommand(
    CTCLInterpreter& interp, const char* command, RunState* pState
) : CTCLObjectProcessor(interp, command, TCLPLUS::kfTRUE),
    m_State(*pState)
{}

/**
 * Destructor
 */
CRunStateCommand::~CRunStateCommand() {}

/**
 * operator()
 *    Processes the command, just digs the state of of m_State and
 *    sets the result to the textual equivalent -- see the header comments
 *    for possible values.
 *
 *  @param interp - references the interpreter that's running the command.
 *  @param objv   - vector of command words. (There can be only one).
 *  @return int   - TCL_OK on success, TCL_ERROR on failure (extra command words).
 */
int
CRunStateCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {                  // try try ... just a little bit harder.
        requireExactly(objv, 1, "runstate - incorrect number of command words");
        switch (m_State.m_state) {
        case RunState::inactive:
            interp.setResult("idle");
            break;
        case RunState::active:
            interp.setResult("active");
            break;
        case RunState::paused:
            interp.setResult("paused");
            break;
        default:
            throw std::domain_error("runstate command - invalid state value!");
        }
        
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("runstate - unexpected exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
}