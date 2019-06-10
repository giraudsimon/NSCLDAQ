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
/**
 * @file CSourceCommand.cpp
 * @brief Implementation of the CSourceCommand class.
 */

#include "CSourceCommand.h"
#include "CFragmentHandler.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>

/*----------------------------------------------------------------------------
** Canonical methods.
*/


/**
 * Constructor
 *
 *  The base class constructor takes care of registering our command.
 *
 * @param[inout] interp - reference to an encapsulate interpreter object.
 * @param[in] command - Name to use when registering the command with Tcl.
 */
CSourceCommand::CSourceCommand(CTCLInterpreter& interp, std::string command) :
  CTCLObjectProcessor(interp, command)
{}

/**
 * Destructor
 *
 *  The base clss unregisters the command.
 */
CSourceCommand::~CSourceCommand()
{
}
/*--------------------------------------------------------------------------
** public interface:
*/

/**
 * operator()
 *  
 * Called when the command is executed.  See the class documentation
 * for the format of the command.
 *
 * @param[inout] interp - The encapsulated interpreter.
 * @param[in]    objv   - The objects that make up the command word.
 *
 * @return int
 * @retval TCL_OK - On success. If no source ids are present, the list of 
 *                  current source ids is the command result.
 */
int
CSourceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  requireAtLeast(objv, 2);
  try {
    if (objv.size() == 2) {
      return listSources(interp);
    } else {
      return addSources(objv);
    }
  } 
  catch (std::string msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (const char* msg) {
    interp.setResult(msg);
    return TCL_ERROR;
  }
  catch (CException& exception) {
    interp.setResult(exception.ReasonText());
    return TCL_ERROR;
  }
}
/*------------------------------------------------------------------------------
** Private utilities:
*/

/**
 * listSources
 *
 * List the source ids that are known by the fragment handler into
 * the interpreter result.
 *
 * @param[inout] interp - the interpreter.
 *
 * @return int - TCL_OK most likely.
 */
int
CSourceCommand::listSources(CTCLInterpreter& interp)
{
  CTCLObject result;
  result.Bind(interp);

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();

  CFragmentHandler::InputStatistics stats = pHandler->getStatistics();

  std::vector<CFragmentHandler::QueueStatistics>& qstats(stats.s_queueStats);
  for (int i = 0; i < qstats.size(); i++) {
    result += static_cast<int>(qstats[i].s_queueId);
  }

  interp.setResult(result);

  return TCL_OK;
}
/**
 * addSources
 *
 *  Adds a set of sources to the source queues managed by the fragment handler.
 *
 * @param[in] objv - vector of Tcl_Obj's that make up the command words.
 *
I* @return int TCL_OK 
*/
int
CSourceCommand::addSources(std::vector<CTCLObject>& objv)
{
  // Convert all objv elements to ints first so that this is all or nothing.
  // the loop will throw an exception if an objv is not an integer.

  std::string socketString = static_cast<std::string>(objv[1]);

  std::vector<uint32_t> ids;
  for (int i = 2; i < objv.size(); i++) {
    ids.push_back(static_cast<uint32_t>(static_cast<int>(objv[i]))); 
  }
  // The remainder can't fail:

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();

  for (int i = 0; i < ids.size(); i++) {
    pHandler->createSourceQueue(socketString, ids[i]);
  }

  return TCL_OK;
  
}
