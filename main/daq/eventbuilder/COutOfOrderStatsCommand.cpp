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

/** @file:  COutOfOrderStatsCommand.h
 *  @brief: Implement the out of order statistics ommand.
 */
#include "COutOfOrderStatsCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"

#include <string>
#include <stdexcept>
#include <Exception.h>
#include <algorithm>



/**
 * constructor
 *   Register the command.
 *   Register our observer.
 */
COutOfOrderStatsCommand::COutOfOrderStatsCommand(
    CTCLInterpreter& interp, const char* command
) : CTCLObjectProcessor(interp, command, true),
m_Observer(this)
{
    CFragmentHandler::getInstance()->addNonMonotonicTimestampObserver(
        &m_Observer
    ) ;
    m_Statistics.s_totals.s_count   = 0;
    m_Statistics.s_totals.s_priorTs = 0;
    m_Statistics.s_totals.s_Ts      = 0;
}
/**
 * destructor
 *   Unregister the observer.
 */
COutOfOrderStatsCommand::~COutOfOrderStatsCommand()
{
    CFragmentHandler::getInstance()->removeNonMonotonicTimestampobserver(
        &m_Observer
    );
}
/**
 * operator()
 *    Called to execute the command.  Fetches the statistics in the
 *    and sets the result in the following way:
 *    - Two lists. The first list is
 *       * Total Counts
 *       * prior timstamp
 *       * offending timestamp.
 *    - The second list is a list of four element sublists containing:
 *       * Source Id.
 *       * Total Counts
 *       * prior timstamp
 *       * offending timestamp.
*/
int COutOfOrderStatsCommand::operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    try {
        requireExactly(objv, 1);
        CTCLObject result;  result.Bind(interp);
        
        // We have global and per source sublists:
        
        CTCLObject global; global.Bind(interp);
        CTCLObject perSrc; perSrc.Bind(interp);
        
        // Fill in the global counters then add that to the list.
        
        global = makeStatsList(interp, m_Statistics.s_totals);
        result += global;
        
        // Now iterate over the source id records in map.
        
        std::for_each(
            m_Statistics.s_bySource.begin(), m_Statistics.s_bySource.end(),
            [this, &perSrc, &interp](std::pair<int, OutOfOrderRecord> p) {
                CTCLObject stats = makeStatsList(interp, p.second);
                CTCLObject item; item.Bind(interp);
                item = p.first;
                Tcl_ListObjAppendList(
                    interp.getInterpreter(), item.getObject(), stats.getObject()
                );
                perSrc += item;
            }
        );
        result += perSrc;
        
        // finally set the result
        
        interp.setResult(result);
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type:");
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * logOutOfOrder
 *    Called by the observer to let us log a new entry.
 *
 *  @param sourceid - id of the offending source.
 *  @param priorTimestamp - prior timestamp.
 *  @param thisTimestamp - offending timestamp.
 */
void COutOfOrderStatsCommand::logOutOfOrder(
    unsigned sourceid, std::uint64_t priorTimestamp,
    std::uint64_t thisTimestamp
)
{
    m_Statistics.s_totals.s_count++;
    m_Statistics.s_totals.s_priorTs = priorTimestamp;
    m_Statistics.s_totals.s_Ts      = thisTimestamp;
    
    auto p = m_Statistics.s_bySource.find(sourceid);
    if (p == m_Statistics.s_bySource.end()) {
        m_Statistics.s_bySource[sourceid] = {
            1, priorTimestamp, thisTimestamp
        };
    } else {
        p->second.s_count++;
        p->second.s_priorTs = priorTimestamp;
        p->second.s_Ts      = thisTimestamp;
    }
}
/**
 * makeStatsList
 *    Creates a statistics list from an OutOfOrderRecord
 *    This consists of the count, prior timestamp, offending timestamp
 *    in order.
 * @param interp - interpreter used to build the list.
 * @param stats  - Reference to the statistics record to use.
 * @return CTCLObject the object containing the list.
 */
CTCLObject
COutOfOrderStatsCommand::makeStatsList(
    CTCLInterpreter& interp, const OutOfOrderRecord& stats
)
{
    CTCLObject result;
    result.Bind(interp);
    
    CTCLObject count; count.Bind(interp);
    count = static_cast<int>(stats.s_count);
    CTCLObject priorTs = uint64Obj(interp, stats.s_priorTs);
    CTCLObject badTs   = uint64Obj(interp, stats.s_Ts);
    
    result += count;
    result += priorTs;
    result += badTs;
    
    return result;
}
/**
 * uint64Obj
 *   Convenience method to make a CTCLObject from a uint64_t
 *     (longint).
 *   @param interp - bound to the object.
 *   @param value  - uint64_t to use.
 *   @return CTCLObject encapsulating the value.
 */
CTCLObject
COutOfOrderStatsCommand::uint64Obj(CTCLInterpreter& interp, uint64_t value)
{
    CTCLObject result;
    result.Bind(interp);
    
    Tcl_Obj* objVal = Tcl_NewLongObj(value);
    result = objVal;
    
    return result;
}
////////////////////////////////////////////////////////////////
// Implement the observer.

/**
 * observer constructor.
 *   @param pCommand -pointer to the command handler.
 */
COutOfOrderStatsCommand::MyObserver::MyObserver(COutOfOrderStatsCommand* pCommand) :
  m_pCommand(pCommand)
{}

/**
 * operator()
 *    Relays the observation to the command.
 *
 * @param sourceid - the source that was out of order.
 * @param piorts   - The timestamp prior to the out of order item.
 * @param  ts      - The offending timestamp (smaller than priorts).
 */
void
COutOfOrderStatsCommand::MyObserver::operator()(
    unsigned sourceid, std::uint64_t priorts, std::uint64_t ts
)
{
    m_pCommand->logOutOfOrder(sourceid, priorts, ts);
}