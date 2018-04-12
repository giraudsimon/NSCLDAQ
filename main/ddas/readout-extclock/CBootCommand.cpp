/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CBootCommand.cpp
# @brief  Implement ddasboot command.
# @author <fox@nscl.msu.edu>
*/

#include "CBootCommand.h"
#include "CMyEventSegment.h"
#include "RunState.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>

/**
 * constructor
 *   @param interp - Reference to the interpreter.
 *   @param pCmd   - Command string.
 *   @param pSeg   - Event segment to manipulate.
 */
CBootCommand::CBootCommand(
    CTCLInterpreter& interp, const char* pCmd, CMyEventSegment* pSeg
) :
    CTCLObjectProcessor(interp, pCmd),
    m_pSegment(pSeg)
{}

/**
 * Destructor:
 */
CBootCommand::~CBootCommand() {}

/**
 * operator()
 *    Gets control when the command is invoked.
 *    - Ensures there are no additional command parameters.
 *    - Invokes the segments's boot method.
 *
 *  @param interp - interpreter executing the command.
 *  @param objv   - command words.
 *  @return int -  Status of the command:
 *                 - TCL_OK - successful completion.
 *                 - TCL_ERROR - failure.  Human readable reason is in
 *                   the intepreter result.
 */
int
CBootCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireExactly(objv, 1, "ddasboot requires no parameters");
        
        if (RunState::getInstance()->m_state == RunState::inactive) {
            m_pSegment->boot();
        } else {
            throw std::runtime_error("Cannot boot system while a run is active or paused.");
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception & e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    
    return TCL_OK;
}
