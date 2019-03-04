/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CDLateStatsCommand.h"
#include "CDataLateStatistics.h"
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <TCLObject.h>


/*--------------------------------------------------------------------------------
** Implementations of canonical methods.
*/

/**
 * constructor:
 *
 *   Construct the object.  The base class constructor is going to 
 *   register the command.  We need to create the observer which will
 *   register itself with the fragment handler.
 *
 * @param interp - Reference to the encapsulated interpreter on which the
 *                 command will be registered.
 * @param command - The command that will be registered.
 */
CDLateStatsCommand::CDLateStatsCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true), 
  m_pStatistics(0)
{
  m_pStatistics = new CDataLateStatistics;
}
/**
 * destructor
 *
 * Destroy the object.  The base class destructor will unregister the command.
 */
CDLateStatsCommand::~CDLateStatsCommand()
{
  delete m_pStatistics;
}
/*--------------------------------------------------------------------------------
** Public method implementations:
*/

/**
 * operator()
 *   Fetch the statistics out of the observer (m_pStatistics) They will be returned 
 *   as a list containing the following elements:
 *   - Total number of data late fragments.
 *   - Worst case time difference with newest fragment.
 *   - A list (possibly empty) containing one element for each data source that
 *     has had data late errors.  Each element contains the following subelements:
 *     # Source id.
 *     # Number of data late fragments.
 *     # worst case time difference with the newest fragment for this source.
 *
 * @param interp - Encapsulated Tcl Interpreter that is running the command.
 * @param objv   - Vector of encapsulated Tcl_Obj*'s that make up the command words.
 *
 * @return int
 * @retval TCL_OK - success, see above for the result specification.
 * @retval TCL_ERROR - faiulre witht the result an error message string.
 */
int
CDLateStatsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{

  // Exactly 1 command  line argument:


  CDataLateStatistics::Statistics stats;

  try {
    requireExactly(objv, 1);
    bindAll(interp, objv);
    stats = m_pStatistics->getStatistics();

  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  //Marshall the result list:

  CTCLObject list;		// Top level list.
  CTCLObject sourceList;	// Per source statistics list.
  list.Bind(interp);
  sourceList.Bind(interp);

  list += static_cast<int>(stats.s_summary.s_nLateFragmentCount);
  list += static_cast<int>(stats.s_summary.s_nWorstTimeDifference);

  for (int i = 0; i < stats.s_perSource.size(); i++) {
    CTCLObject source;
    source.Bind(interp);

    source += static_cast<int>(stats.s_perSource[i].s_nSourceId);
    source += static_cast<int>(stats.s_perSource[i].s_nLateFragmentCount);
    source += static_cast<int>(stats.s_perSource[i].s_nWorstTimeDifference);

    sourceList += source;
  }
  list += sourceList;

  interp.setResult(list);
  return TCL_OK;


}
