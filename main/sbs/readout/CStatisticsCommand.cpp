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

/** @file:   CStatisticsCommand.cpp
 *  @brief:  Implement the statistics command.
 */
#include "CStatisticsCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <string>

/**
 * constructor
 *   @param interp - pointer to the interpreter.
 *   @param name   - name of the command to register.
 *   @param pExperiment - pointer to the experiment object.
 *
 */
CStatisticsCommand::CStatisticsCommand(
    CTCLInterpreter* pInterp, const char* name, CExperiment* pExperiment
) :
    CTCLObjectProcessor(*pInterp, name, TCLPLUS::kfTRUE),
    m_pExperiment(pExperiment)
{
    
}
/**
 * Destructor
 */
CStatisticsCommand::~CStatisticsCommand() {}

/**
 * operator()
 *    The command execution entry.  Fetch the statistics from the
 *    experiment and format them into the list described in
 *    the header.
 * @param interp - interpreter on which the command is run.
 * @param objv   - command line parameters - none.
 * @return int   - TCL_OK On success and TCL_ERROR if not.
 */
int
CStatisticsCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        requireExactly(objv, 1, "statistics - Incorrect number of command parameters");
        const CExperiment::Statistics& counters(m_pExperiment->getStatistics());
        
        CTCLObject result;
        result.Bind(interp);
        CTCLObject cumulative;
        cumulative.Bind(interp);
        CTCLObject perRun;
        perRun.Bind(interp);
        
        formatCounters(cumulative, counters.s_cumulative);
        formatCounters(perRun, counters.s_perRun);
        
        result += cumulative;
        result += perRun;
        interp.setResult(result);
        
    } catch (CException & e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string& m) {
        interp.setResult(m);
        return TCL_ERROR;
    }
    catch(...) {
        interp.setResult("CStatisticsCommand::operator() - unexpected exception type");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*-----------------------------------------------------------------------------
 * Private utilities.
 */

/**
 * formatCounter
 *    Given a reference to a counter returns the three element list
 *    that describes the counter values
 *  @param result  - The TCLObject to which the itmes will be lappended.
 *  @param counters - Reference to Counters object to format.
 */
void
CStatisticsCommand::formatCounters(
    CTCLObject& result, const CExperiment::Counters& counters
)
{
    result += (double)(counters.s_triggers);
    result += (double)(counters.s_acceptedTriggers);
    result += (double)(counters.s_bytes);
    
}
