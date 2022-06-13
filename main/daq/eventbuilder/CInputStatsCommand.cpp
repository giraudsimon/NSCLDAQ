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
#include "CInputStatsCommand.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <tcl.h>

/*-------------------------------------------------------------------
** Canonicals:
*/

/**
 * CInputStatsCommand
 *
 *   Constructor; registers the object as a command processor
 *   on an interpreter.
 *
 * @param interp - Interpreter on which we're registering.
 * @param command - Command name (e.g. ::EVB::inputStats).
 */
CInputStatsCommand::CInputStatsCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command, true)
{}

/**
 * ~CInputStatsCommand
 * 
 * Destructor: the base class constructor takes care of unregistration.
 */
CInputStatsCommand::~CInputStatsCommand()
{}

/*------------------------------------------------------------------
** Virtual method overrides.
*/

/**
 * operator()
 *
 *  This method is called by the Tcl interpreter when the command is
 *  invoked.
 *
 * @param interp - Reference to the interpreter that is executing the command.
 * @param objv   - Reference to the vector of command line words.
 *
 * @return int - Status of the command:
 * @retval TCL_OK - success.
 * @return TCL_ERROR - failure. The only possible failure is that there are too
 *                     many command objects.
 * @note See the class description for information about the result returned by this
 *       command.  On error, an error message is set as the result.
 */
int
CInputStatsCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if (objv.size() != 1) {
    std::string error="Too many command line parameters";
    interp.setResult(error);
    return TCL_ERROR;
  }
  // Get the fragment handler and statistics strruct:

  CFragmentHandler*                 pQueues = CFragmentHandler::getInstance();
  CFragmentHandler::InputStatistics stats   = pQueues->getStatistics();

  /*
    Marshall the data into a result object and set it as the result.
    we use objects because it's easier to build the lists/nestsed lists that way.
    Note that tcl.h assures us that Tcl_WideInt is at least 64 bits wide.
  */
  CTCLObject result;

  result.Bind(interp);

  // Timestamps get added as WidInt objs?

  CTCLObject wideInt;
  wideInt.Bind(interp);
 
  wideInt = (Tcl_WideInt)(stats.s_oldestFragment);  // 0
  result += wideInt;

  wideInt = (Tcl_WideInt)(stats.s_newestFragment); // 1
  result += wideInt;

  result += (int)(stats.s_totalQueuedFragments); // 2

  // Now the individual queue stats as a list of lists:

  CTCLObject QueueStatList;
  QueueStatList.Bind(interp);


  for (int i = 0; i < stats.s_queueStats.size(); i++) { //3 
    CTCLObject aQueueStat;
    aQueueStat.Bind(interp);
    
    aQueueStat += (int)stats.s_queueStats[i].s_queueId;
    aQueueStat += (int)stats.s_queueStats[i].s_queueDepth;

    wideInt     = (Tcl_WideInt)(stats.s_queueStats[i].s_oldestElement);
    aQueueStat += wideInt;
 
    wideInt    = (Tcl_WideInt)(stats.s_queueStats[i].s_queuedBytes);
    aQueueStat += wideInt;
    
    wideInt     = (Tcl_WideInt)(stats.s_queueStats[i].s_dequeuedBytes);
    aQueueStat += wideInt;
    
    wideInt     = (Tcl_WideInt)(stats.s_queueStats[i].s_totalQueuedBytes);
    aQueueStat += wideInt;
    
    QueueStatList += aQueueStat;
  }
  result += QueueStatList;

  wideInt = (Tcl_WideInt)stats.s_inflight;  // 4
  result += wideInt;                       // Total in flight frags.
  
  interp.setResult(result);
  return TCL_OK;

}
