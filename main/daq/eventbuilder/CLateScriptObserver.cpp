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
 * @file CLateScriptObserver.cpp
 * @brief Implementation of the CLateScriptObserver class.
 */

#include "CLateScriptObserver.h"
#include "fragment.h"
#include <TCLInterpreter.h>
#include <Exception.h>
#include <TCLObject.h>
#include <TCLList.h>

#include <string>
#include <iostream>


/*--------------------------------------------------------------------------------------
** Implementation of Canonical methods.
*/

/**
 * constructor
 * 
 * - Construct the data members from the parameters.
 * - Register as a data late observer with the CFragmentHandler singleton.
 *
 * @param interp - reference to the encapsulated interpreter.
 * @param prefix - Script prefix.  See the header comments and createCommand
 *                 to see how this relates to the actual script invoked.
 */
CLateScriptObserver::CLateScriptObserver(CTCLInterpreter& interp, std::string prefix) :
  m_interpreter(interp),
  m_scriptPrefix(prefix)

{
  // Hook us up with the fragment handler:

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addDataLateObserver(this);
}
/**
 * destructor
 *   Just remvoe us from the observer list.
 */
CLateScriptObserver::~CLateScriptObserver()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeDataLateObserver(this);
}

/*-------------------------------------------------------------------------------------
** public methods.
*/

/**
 * operator()
 *
 *  Receives control when a data late fragment is detected.
 *  - The command string is created.
 *  - The command string is executed in a heave try/catch block that
 *    outputs to stderr any problems detected.
 *
 * @param fragment - refrence to the fragment.
 * @param newest   - newest timestamp seen.
 */
void
CLateScriptObserver::operator()(const ::EVB::Fragment& fragment, uint64_t newest)
{
  std::string fullCommand = createCommand(fragment, newest);
  std::string errorMessage;

  try {
    m_interpreter.GlobalEval(fullCommand);
  }
  catch (std::string msg) {
    errorMessage = msg;
  }
  catch (const char* pMsg) {
    errorMessage = pMsg;
  }
  catch (CException& e) {
    errorMessage = e.ReasonText();
  }
  catch  (...) {
    errorMessage = "Unanticipated exception thrown by data late handler script";
  }
  if (errorMessage.size() != 0) {
    std::cerr << "Error in datal ate script: "
	      << errorMessage << std::endl;
  }
}

/**
 * getScript
 *
 *   Returns the current script prefix:
 *
 * @return std::string - the current script prefix.
 */
std::string
CLateScriptObserver::getScript() const
{
  return m_scriptPrefix;
}
/*---------------------------------------------------------------------------------
** Private utilities
*/


/**
 * createCommand
 *
 *  The actual command is a command prefix supplied at construction time
 *  with several parameters appened to it as described by the 
 *  class comments.
 *  
 *  This utility builds the final command string by taking apart the prefix as a list
 *  and then recombining its elements with the parameters we append.
 *  Finally the string representation of the list is fetched and returned.
 *
 * @param fragment - Reference to the fragment that was late.
 * @param newest   - timestamp of the newest fragment seen.
 *
 * @return std::string - the command to execute
 */
std::string
CLateScriptObserver::createCommand(const ::EVB::Fragment& fragment, uint64_t newest)
{

  // Split up the prefix into a list of command prefix words.

  CTCLList prefixList(&m_interpreter, m_scriptPrefix);
  StringArray listElements;
  prefixList.Split(listElements);

  // Put the prefix into a CTCLObject:

  CTCLObject commandList;
  commandList.Bind(m_interpreter);
  
  for (int i = 0; i < listElements.size(); i++) {
    commandList += listElements[i];
  }

  // Now we need to add in the other chunks.
  
  // Fragment source id, timestamp and newest timestmp.

  commandList += static_cast<int>(fragment.s_header.s_sourceId);
  commandList += static_cast<int>(fragment.s_header.s_timestamp);
  commandList += static_cast<int>(newest);

  // Contents of the event.

  uint32_t size  = fragment.s_header.s_size;
  void*    pBody = fragment.s_pBody;

  Tcl_Obj* pBodyObject = Tcl_NewByteArrayObj(reinterpret_cast<const unsigned char*>(pBody), 
					     size);
  commandList += pBodyObject;

  return static_cast<std::string>(commandList);

  
}
