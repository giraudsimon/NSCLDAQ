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

/** @file:  CStatisticsCommand.cpp
 *  @brief:  Implement the 'statistics' command object.
 */
#include "CStatisticsCommand.h"
#include "CTheApplication.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "Exception.h"
#include <stdexcept>
#include <string>

/**
 * constructor
 *    @param interp - interpreter on which the command is regsitered.
 *    @param command - string that activates the command.
 */
CStatisticsCommand::CStatisticsCommand(
    CTCLInterpreter& interp, const char* command
) :
    CTCLObjectProcessor(interp, command, TCLPLUS::kfTRUE)
{}

/**
 * destructor
 */
CStatisticsCommand::~CStatisticsCommand()
{}

/**
 * operator()
 *     executes the command. See the header for the successful command
 *     result.
 *     
 *  @param interp - interpreter that's running the command.
 *  @param objv   - command words.
 *  @return int   - TCL_OK if successful, TCL_ERROR if not.
 */
int
CStatisticsCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        requireExactly(objv, 1, "statistics - incorrect number of command parameters");
        CTheApplication* pApp = CTheApplication::getInstance();
        COutputThread*   pRouter = pApp->getOutputThread();
        const COutputThread::Statistics& stats = pRouter->getStatistics();
        
        // Now we can format the output list:
        
        CTCLObject result;
        result.Bind(interp);
        CTCLObject cumulative;
        cumulative.Bind(interp);
        CTCLObject perRun;
        perRun.Bind(interp);
        
        formatCounters(cumulative, stats.s_cumulative);
        formatCounters(perRun, stats.s_perRun);
        result += cumulative;
        result += perRun;
        interp.setResult(result);
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::exception e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unanticipated exception type returned");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*-----------------------------------------------------------------------------
 *  private utilities.
 */

/**
 *  Format  the three element counters list.
 *   @param result -output result.
 *   @param counters - the counters objedt to format.
 */
void
CStatisticsCommand::formatCounters(
    CTCLObject& result, const COutputThread::Counters& counters
)
{
    result += (double)(counters.s_triggers);
    result += (double)(counters.s_acceptedTriggers);
    result += (double)(counters.s_bytes);

}