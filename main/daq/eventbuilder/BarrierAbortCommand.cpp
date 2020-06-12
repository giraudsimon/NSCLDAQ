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

/** @file:  BarrierAbortCommand.cpp
 *  @brief: Implement CBarrierAbortCommand
 */
#include "BarrierAbortCommand.h"
#include "CFragmentHandler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <TCLException.h>
#include <exception>

/**
 * constructor
 *   @param interp - interpreter on wich the command gets registered.
 *   @param cmd    - name of the command we'll use.
 */
CBarrierAbortCommand::CBarrierAbortCommand(
    CTCLInterpreter& interp, const char* cmd
) :
    CTCLObjectProcessor(interp, cmd, true)
    {}
    
/**
 * operator()
 *    Called when the command is used.
 */
int
CBarrierAbortCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        requireExactly(objv, 1, "Command does not require any parameters");
        
    
        CFragmentHandler::getInstance()->abortBarrierProcessing();
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e)
    {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type caught");
        return TCL_ERROR;
    }
    
    return TCL_OK;
    
}