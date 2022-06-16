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

#include "CBarrierStatsCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CBarrierStats.h"
#include "CIncompleteBarrierStats.h"
#include <Exception.h>
#include <string>
/*----------------------------------------------------------------------------------
** Implementations of canonicals:
*/

/**
 * @fn CBarrierStatsCommand::CBarrierStatsCommand
 *
 *  - The base class will register us with the interpreter.
 *  - We need to construct m_pCompleteStats
 *  - We need to construct m_pIncompleteStats.
 *
 * @param interp[in,out] - reference to an encapsulated Tcl interpreter object.
 * @param commandName[in] - String that will be used to invoke the command.
 *
 */
CBarrierStatsCommand::CBarrierStatsCommand(CTCLInterpreter& interp, std::string commandName) :
  CTCLObjectProcessor(interp, commandName),
  m_pCompleteStats(0),
  m_pIncompleteStats(0)
{
  // This try/catch block ensures that failures in construction dont' lead memory:
  try {
    m_pCompleteStats   = new CBarrierStats;
    m_pIncompleteStats = new CIncompleteBarrierStats;
  }
  catch (...) {
    freeObjects();

    throw;			// Re-throw to/through caller.
  }
}
/**
 * @fn CBarrierStatsCommand::~CBarriersStatsCommand
 *
 * Destructor.  Free the statistics objects.
 */
CBarrierStatsCommand::~CBarrierStatsCommand()
{
  freeObjects();
}


/*---------------------------------------------------------------------------------
**
*   Public entry points.
*/

/**
 * @fn CBarrierStatsCommand::operator()
 *
 *   Called to execute the command.
 *
 * @param interp[in,oput]  - Encapsulated interpreter object. 
 * @param objv[in]         - Vector of encapsulated TCL_Obj's that make up the command words.
 *
 * @return int
 * @retval TCL_OK - Command succeeded  The result is described in the class comments.
 * @retval TCL_ERROR - Command failed. The result is a descriptive error message.
 */
int
CBarrierStatsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // each branch of this try/catch sets the result differently.

  CTCLObject* pComplete(0);
  CTCLObject* pIncomplete(0);
  int         status = TCL_OK;

  try {
    CTCLObject* pComplete    = completeStats(interp);
    CTCLObject* pIncomplete  = incompleteStats(interp);
    
    CTCLObject result;
    result.Bind(interp);
    result += *pComplete;
    result += *pIncomplete;


    interp.setResult(result);
  }
  // the catch blocks are most known error
  // conditions:

  catch (std::string msg) { 
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (const char* msg) {
    interp.setResult(msg);
    status = TCL_ERROR;
  }
  catch (CException& e) {
    interp.setResult(e.ReasonText());
    status = TCL_ERROR;
  }
  catch (...) {
    std::string msg("barrierstats - caught an unanticipated exception.");

  }
 
  // if we did not catch:

  return status;

}
/* @fn CBarrierStatsCommand::completeStats
 *
 * Provides a dynamically allocated CTCLObject pointer containing
 * the complete barrier statistics as described in the class comment heaers.
 *
 * @param[in,out]  interpreter that is bound to objets to assist in parsing.
 *
 * @return CTCLObject* 
 * @retval Ppointer to the event array.
 */
CTCLObject*
CBarrierStatsCommand::completeStats(CTCLInterpreter& interp)
{
  CTCLObject* pResult = new CTCLObject;
  pResult->Bind(interp);


  CBarrierStats::BarrierStats stats = m_pCompleteStats->getStatistics();

  // Do the outer stuff:

  (*pResult) += static_cast<int>(stats.s_barriers.s_barrierCount);
  (*pResult) += static_cast<int>(stats.s_homogeneous);
  (*pResult) += static_cast<int>(stats.s_heterogeneous);

  // Make the frags by type list:

  CTCLObject* pFragsByType = compileBarrierStats(interp, stats.s_barriers);
  (*pResult) += (*pFragsByType);
  delete pFragsByType;


  // per data source statistics:

  CTCLObject sourceStats;
  sourceStats.Bind(interp);
  
  for (
       std::map<uint32_t, CBarrierStats::BarrierTypeStats>::iterator p = stats.s_sourceCounts.begin();
       p != stats.s_sourceCounts.end(); p++
  ) {
    CTCLObject source;
    source.Bind(interp);

    source += static_cast<int>(p->first);

    CTCLObject* pBarrierTypes = compileBarrierStats(interp, p->second);
    source += *pBarrierTypes;
    delete pBarrierTypes;

    sourceStats += source;
  }

  (*pResult) += sourceStats;
  return pResult;
}

/**
 * @fn CBarrierStatsCommand::incompleteStats
 *
 * Marshall the Tcl List representation of the statistics for incomplete
 * barriers.  See the class description for the format of this
 * list 
 * 
 * @param interp - Reference to an encapsulated interpreter that willb e bound into
 *                 CTCLObjects created to support advanced parsing.
 * 
 * @return CTCLObject* Pointer to a dynamically allocated CTCLOBject containing the result list.
 *                 The client must delete this list at an appropriate time.
 */
CTCLObject* 
CBarrierStatsCommand::incompleteStats(CTCLInterpreter& interp)
{
  CIncompleteBarrierStats::Statistics stats = m_pIncompleteStats->getStatistics();
  CTCLObject* pResult = new CTCLObject;

  pResult->Bind(interp);

  (*pResult) += static_cast<int>(stats.s_totalIncomplete);
  (*pResult) += static_cast<int>(stats.s_homogeneous);
  (*pResult) += static_cast<int>(stats.s_heterogeneous);


  // Accumulate the histogram:

  CTCLObject histogram;
  histogram.Bind(interp);

  for (
       std::map<uint32_t, size_t>::iterator p = stats.s_missingHistogram.begin();
       p != stats.s_missingHistogram.end(); p++
  ) {
    CTCLObject channel;
    channel.Bind(interp);
    channel += static_cast<int>(p->first);
    channel += static_cast<int>(p->second);

    histogram += channel;
  }
  (*pResult) += histogram;

  // Accumulate the missing source counts:

  CTCLObject missingSources;
  missingSources.Bind(interp);

  for (
       std::map<uint32_t, size_t>::iterator p = stats.s_sourceMissingCount.begin();
       p != stats.s_sourceMissingCount.end(); p++
  ) {
    CTCLObject source;
    source.Bind(interp);

    source += static_cast<int>(p->first);
    source += static_cast<int>(p->second);

    missingSources += source;
  }
  (*pResult) += missingSources;


  return pResult;
}
 
            

/**
 * @fn CBarrierStatsCommand::compileBarrierStatsCommand
 *
 *  Compiles the contents of a CBarrierStats::BarrierTypeStats struct into
 *  Tcl List form.
 *
 * @param interp[in]    Encapsulated interpreter.
 * @param typeStats[in] reference to a CBarrierStats::BarrierTypeStats object.
 *
 * @return CTCLObject* - dynamically allocated result list.
 *                       must be deleted by the client.
 */
CTCLObject* 
CBarrierStatsCommand::compileBarrierStats(CTCLInterpreter& interp,CBarrierStats::BarrierTypeStats& stats)
{
  CTCLObject* pResult = new CTCLObject;
  pResult->Bind(interp);



  //  (*pResult) += static_cast<int>(stats.s_barrierCount);
  for (
       std::map<uint32_t, size_t>::iterator p = stats.s_typeCount.begin(); 
       p != stats.s_typeCount.end(); p++
  ) {
    CTCLObject typeStats;
    typeStats.Bind(interp);
    typeStats += static_cast<int>(p->first);
    typeStats += static_cast<int>(p->second);

    (*pResult) += typeStats;

  }

  return pResult;
}

/**
 * @fn CBarrierStatsCommand::freeObjects
 *
 *   Common code factored out of con/destructor to kill off
 *   dynamically allocated member data.
 */
void
CBarrierStatsCommand::freeObjects()
{
  delete m_pCompleteStats;
  delete m_pIncompleteStats;
}
