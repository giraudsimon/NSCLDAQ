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
 */
#include "CRunStateCommand.h"
#include "CRunState.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <string>
/**
 * constructor
 *    @param interp  - References the interpreter on which the command is registered.
 *    @param command - name of command registered
 */
CRunStateCommand::CRunStateCommand(CTCLInterpreter& interp, const char* command) :
    CTCLObjectProcessor(interp, command, TCLPLUS::kfTRUE)
{}

/**
 * destructor
 */
CRunStateCommand::~CRunStateCommand() {}

/**
 * operator()
 *     Called to execute the command.
 *
 *  @param interp - reference to the interpreter executing the command.
 *  @param objv   - The command line words.
 *  @return int   - TCL_OK - success, TCL_ERROR - failure.
 */
int
CRunStateCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        switch (CRunState::getInstance()->getState()) {
        case CRunState::Idle:
            interp.setResult("idle");
            break;
        case CRunState::Active:
            interp.setResult("active");
            break;
        case CRunState::Paused:
            interp.setResult("paused");
            break;
        default:
            throw std::domain_error("runstate command -invalid state returned");
        }
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e ) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("runstate - unanticipated exception caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
    
}