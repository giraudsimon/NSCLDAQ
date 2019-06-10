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
 * @file CCompleteBarrierCallback.cpp
 * @brief Implement the CCompleteBarrierCallback which supplies Tcl script observation of
 *        complete barrier processing.
 */

#include "CCompleteBarrierCallback.h"
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include "Exception.h"

#include <iostream>


/*-----------------------------------------------------------------------------
** Implement canonical methods:
*/

/**
 * constructor
 *
 *   Create the object.   In addition to saving the parameters for 
 *   when the observer is invoked, we also register ourselves with the
 *   fragment handling singleton.
 *
 * @param interp - Reference to the TCL Interpreter that should be used
 *                 to run the callback script
 * @param command - Root of the command that shouild be called-back.
 */
CCompleteBarrierCallback::CCompleteBarrierCallback(CTCLInterpreter& interp, std::string command) :
  m_Interpreter(interp),
  m_script(command)
{
  // Register this observer with the fragment handler singleton:

  CFragmentHandler* pFragHandler = CFragmentHandler::getInstance();
  pFragHandler->addBarrierObserver(this);
}

/**
 * Destructor
 *
 *  In our case we need to  unregister this from the fragment handler's set of
 *  observers.
 */
CCompleteBarrierCallback::~CCompleteBarrierCallback()
{
  CFragmentHandler* pFragHandler = CFragmentHandler::getInstance();
  pFragHandler->removeBarrierObserver(this);
}
/*-----------------------------------------------------------------------------
** Public member functions:
*/

/**
 * operator()
 *
 *   Called when a complete barrier is processed.  
 *  - Generate the full text of the script to execute.
 *  - Execute it.
 *
 *  @param barrierTypes - vector of sourceid,barrierType pairs.  These are marshalled
 *                        into a Tcl list of pairs.
 *
 */
void
CCompleteBarrierCallback::operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes)
{
  try {
    std::string fullCommand = makeScript(barrierTypes);
    m_Interpreter.GlobalEval(fullCommand);
  }
  catch (std:: string msg) {
    std::cerr << "Error running Tcl complete barrier observer: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "Error running Tcl complete barrier observer: " << msg << std::endl;
  }
  catch (CException& e) {
    std::cerr << "Error running Tcl complete barrier observer: " << e.ReasonText() << std::endl;
  }
  catch (...) {
    std::cerr << "Unanticipated exception type running Tcl complete barrier observer script\n";
  }
}
/**
 * getCommand 
 *
 * Return the string that is the current command.
 *
 * @return std::string
 */
std::string
CCompleteBarrierCallback::getCommand() const
{
  return m_script;
}

/*---------------------------------------------------------------------------------
**  Private utilities
*/

/**
 * makeScript
 *
 *   Create the script from the m_script base and the information
 *   about the source ids and the barrier types they emitted.
 *
 * @param barrierTypes - Vector of pairs. The first element of each pair is a source id.
 *                       the second element of each pair is the barrier type emitted  by that
 *                       source
 *
 * @return std::string - The full script to run.
 */
std::string
CCompleteBarrierCallback::makeScript(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes)
{
  // Build the script up in a Tcl_Obj:

  CTCLObject script;
  script.Bind(m_Interpreter);
  script = m_script;

  // Now the single parameter... a list of pairs.

  CTCLObject parameter;
  parameter.Bind(m_Interpreter);

  for (int i = 0; i < barrierTypes.size(); i++) {
    uint32_t source = barrierTypes[i].first;
    uint32_t type   = barrierTypes[i].second;
    CTCLObject pair;
    pair.Bind(m_Interpreter);
    
    pair += static_cast<int>(source);
    pair += static_cast<int>(type);

    parameter += pair;
    
  }
  script += parameter;

  return static_cast<std::string>(script);
}
