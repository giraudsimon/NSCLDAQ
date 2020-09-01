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
#include "COutputStatsCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "COutputStatsObserver.h"



/*------------------------------------------------------------------------------------------
** Implementations of canonical operations.
*/

/**
 * Constructor:
 *    - Construct us as an interpreter command.
 *    - Create the observer (which registers it).
 *
 * @param interp  - reference to the encapsulated interpreter.
 * @param command - The command name.
 */
COutputStatsCommand::COutputStatsCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true),
  m_pObserver(0)
{
  m_pObserver = new COutputStatsObserver;
}
/**
 * destructor:
 *
 *   Kill off the observer, which also un-registers it.
 */
COutputStatsCommand::~COutputStatsCommand()
{
  delete m_pObserver;
}

/*-----------------------------------------------------------------------------------------------
** Public entries:
*/

/**
 * operator()
 *
 *   Command dispatcher. 
 *   - Ensures there is a subcommand.
 *   - Dispatches to it to get the job done or an error if the subcommand is illegal.
 *
 * @param interp - Encapsualted interpreter.
 * @param objv   - Reference to a vector of encapsulated Tcl_Obj's that make up the 
 *                 command words.
 * @return int
 * @return TCL_OK - Command succeeded.
 * @return TCL_ERROR - Command failed.
 *
 * @note with the exception of errors detected by this method directly, 
 *       the result comes from the specific command execution method.
 */
int
COutputStatsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  // Ensure there's at least a subcommadn:

  try {
    requireAtLeast(objv, 2);
    std::string subCommand = objv[1];

    if (subCommand == "get") {
      return get(interp, objv);
    } else if (subCommand == "clear") {
      return clear(interp, objv);
    } else {
      throw std::string("output statistics: ivaild subcommand.");
    }
  }
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------
** Command executors.
*/

/**
 * get
 *   Retrive the statistics.  The are returned as a two element Tcl list
 *   - Total output fragments
 *   - List of source-id, output fragment count pairs.
 *
 * @param interp - Encapsualted interpreter.
 * @param objv   - Reference to a vector of encapsulated Tcl_Obj's that make up the 
 *                 command words.
 * @return int
 * @return TCL_OK - Command (see above for return value contents):
 * @return TCL_ERROR - Command failed.
 */
int
COutputStatsCommand::get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireExactly(objv, 2);


  COutputStatsObserver::Statistics stats = m_pObserver->getStatistics();


  // Marshall and set the results:

  CTCLObject result;
  result.Bind(interp);

  result += static_cast<int>(stats.s_nTotalFragments); // 0
  CTCLObject fragList;
  fragList.Bind(interp);

  for (int i = 0; i < stats.s_perSourceFragments.size(); i++) { // 1
    CTCLObject fragEntry;
    fragEntry.Bind(interp);

    fragEntry += static_cast<int>(stats.s_perSourceFragments[i].first); //1.0
    fragEntry += static_cast<int>(stats.s_perSourceFragments[i].second); // 1.1
    fragList += fragEntry;
  }
  result += fragList;

  interp.setResult(result);

  return TCL_OK;

}


/**
 * clear
 *   Clear the statistics.
 *
 * @param interp - Encapsualted interpreter.
 * @param objv   - Reference to a vector of encapsulated Tcl_Obj's that make up the 
 *                 command words.
 * @return int
 * @return TCL_OK - Command - ther is no result.
 * @return TCL_ERROR - Command failed.
 */
int
COutputStatsCommand::clear(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  requireExactly(objv, 2);

  m_pObserver->clear();
  return TCL_OK;
}
