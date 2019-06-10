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

/**
 * @file CReviveSocketCommand.cpp
 * @brief Implementation of the CReviveSocketCommand class.
 */

#include "CReviveSocketCommand.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "CFragmentHandler.h"

/*----------------------------------------------------------------------
** Canonical method implementations.
*/

/**
 * constructor
 *
 *   The base class does the heavy lifting of registering us so that our operator()
 *   is invoked in response to the command.
 *
 * @param interp - reference to the encapsulated interpreter the command will be registered on.
 * @param name   - Command name.
 */
CReviveSocketCommand::CReviveSocketCommand(CTCLInterpreter& interp, std::string name) :
  CTCLObjectProcessor(interp, name)
{
}
/**
 * destructor
 *
 *   The base class destructor agains does everyting for us:
 */
CReviveSocketCommand::~CReviveSocketCommand()
{}

/*---------------------------------------------------------------------------
** public interface
*/

/**
 *  operator()
 *
 *   Called in response to the command.   In addition to the command word,
 *  we also have a socket name.  The socket name is a Tcl socket name from eg.
 *  the [socket] comand.
 *
 * @param interp - the interpreter that is running the command.
 * @param objv   - The command words.
 */
int
CReviveSocketCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  bindAll(interp, objv);
  requireExactly(objv, 2);

  std::string sockName  = static_cast<std::string>(objv[1]);
  
  // get the fragment handler and tell it what to do:

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->reviveSocket(sockName);

  return TCL_OK;
}
