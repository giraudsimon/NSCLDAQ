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
# @file   <filename>
# @brief  <brief description>
# @author <fox@nscl.msu.edu>
*/
#include "CSyncCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CMyEventSegment.h"
#include <stdexcept>
/**
 *  Constructor
 *     Base class registers the command.  We need to save the event
 *     processor pointer:
 */
CSyncCommand::CSyncCommand(CTCLInterpreter& interp, CMyEventSegment* pSeg) :
    CTCLObjectProcessor(interp, "ddas_sync", true),
    m_pSegment(pSeg)
{}

/**
 * destructor
 *    Chain to superclass for now.
 */
CSyncCommand::~CSyncCommand() {}


/**
 * operator()
 *    Gets control when the command is invoked.  We're just going to
 *    *   Esnure there are no more parameters.
 *    *   Invoke the event segment's synchronize method.
 *
 * @param interp - intepreter that is running this command.
 * @param objv   - words that make up the tcl command.
 * @return int TCL_OK, TCL_ERROR to give the result.
 *
 */
int
CSyncCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Exceptions map to TCL_ERROR returns with a string that describes the
    // exception.
    
    try {
        requireExactly(
            objv, 1, "ddas_sync command takes no parameters"
        );                               // can throw std::string
        
        m_pSegment->synchronize();      // can throw std::exception sublcass
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;
}