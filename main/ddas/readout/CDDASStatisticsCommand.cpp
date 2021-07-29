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

/** @file:  CDDASStatisticsCommand.cpp
 *  @brief: Implement the statistics command specific to DDASReadout.
 */

#include "CDDASStatisticsCommand.h"
#include "CMyEventSegment.h"
#include "CMyScaler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <string>
#include <string.h>


/**
 * construcutor
 *   @param  interp - interpreter on which the command is registered.
 *   @param  command- name of the command 'should/must' be "statistics"
 *                    to smoothly replace the SBSReaout framework command.
 *   @param pSeg    - Pointer to the event segment which provides byte counters.
 *   @param scalers - Reference to the array of scaler segments that provide the
 *                    individual module trigger statistics information.
 */
CDDASStatisticsCommand::CDDASStatisticsCommand(
    CTCLInterpreter& interp, const char* command, CMyEventSegment* pSeg,
    std::vector<CMyScaler*>& scalers
) :
    CTCLObjectProcessor(interp, command, true),
    m_pEventSegment(pSeg), m_Scalers(scalers)
{}

/**
 * destructor
 */
CDDASStatisticsCommand::~CDDASStatisticsCommand()
{}

/**
 * operator()
 *    Called to execute the Tcl command.
 *
 *  @param interp - references the interpreter executing the command.
 *  @param objv   - Command line parameters.
 *  @return int   - TCL_OK if success - TCL_ERROR on failure.
 *  
 */
int
CDDASStatisticsCommand::operator() (
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        requireExactly(
            objv, 1, "DDAS 'statistics' - incorrect command parameter count"
        );
        auto bytes = m_pEventSegment->getStatistics();
        CTCLObject result;
        result.Bind(interp);
        formatResult(interp, result, bytes.first, bytes.second);
        interp.setResult(result);
        
    }
    catch (std::exception & e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult(
            "Unanticipated exception type caugh in DDAS 'statistics' command"
        );
        return TCL_ERROR;
    }
    return TCL_OK;
}
/*--------------------------------------------------------------------------
 * Private utilities.
 */

/**
 * formatResult
 *    Computes and formats the result.  Note that we have to sum the trigger
 *    statistics over the modules in the system.
 * @param interp  - interpreter executing the command.
 * @param result  - reference to the CTCLObject in to which the result is formatted
 *                  must already be bound.
 * @param bytes   - Total number of bytes this program instance acquired.
 * @param runBytes -Number of bytes acquire over the last (or current) run.
 *
 *  The result is a two element list.  Each element is a three element sublist
 *  of statistics.  The first element contains cumulative statistics,
 *  the second the statistics from the current run or most recently ended run
 *  if data taking is not active.
 *  Each list has, in order, the following three subelements:
 *     -   Number of triggers.
 *     -   Number of accepted triggers.
 *     -   Number of bytes of data transferred.
 */
void
CDDASStatisticsCommand::formatResult(
    CTCLInterpreter& interp, CTCLObject& result,
    size_t bytes, size_t runBytes
)
{
    // Collect triggers statistic sums:
    
    CMyScaler::Statistics totals;
    memset(&totals, 0, sizeof(CMyScaler::Statistics));
    for (int i = 0; i < m_Scalers.size(); i++) {
        auto moduleStats = m_Scalers[i]->getStatistics();
        totals.s_cumulative.s_nTriggers += moduleStats.s_cumulative.s_nTriggers;
        totals.s_cumulative.s_nAcceptedTriggers +=
            moduleStats.s_cumulative.s_nAcceptedTriggers;
        
        totals.s_perRun.s_nTriggers += moduleStats.s_perRun.s_nTriggers;
        totals.s_perRun.s_nAcceptedTriggers += moduleStats.s_perRun.s_nTriggers;
    }
    // Now we can format the two sublists and append them to result.
    
    CTCLObject totalobj;
    totalobj.Bind(interp);
    CTCLObject perRunObj;
    perRunObj.Bind(interp);
    
    formatCounters(
        totalobj,
        totals.s_cumulative.s_nTriggers, totals.s_cumulative.s_nAcceptedTriggers,
        bytes
    );
    formatCounters(
        perRunObj,
        totals.s_perRun.s_nTriggers, totals.s_perRun.s_nAcceptedTriggers,
        runBytes
    );
    
    result += totalobj;
    result += perRunObj;
}
/**
 * formatCounters
 *    Format a three element list from the individual counters for a statistics
 *    sublist.  See formatResult for a description of the resulting list.
 *
 *  @param result   - object into which the list will be created.
 *                    Must already be bound to an interpreter.
 *  @param triggers - Number of triggers.
 *  @param accepted - Number of accepted triggers.
 *  @param bytes    - Number of bytes.
 */
void
CDDASStatisticsCommand::formatCounters(
    CTCLObject& result, size_t triggers, size_t accepted, size_t bytes
)
{
    result += int(triggers);
    result += int(accepted);
    result += int(bytes);
}
