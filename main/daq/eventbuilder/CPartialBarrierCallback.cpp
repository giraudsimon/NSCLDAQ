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
 * @file CPartialBarrierCallback.cpp
 * @brief Implementation of CPartialBarrierCallback - call Tcl script for partial barriers.
 */

#include "CPartialBarrierCallback.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>

#include <iostream>

/*------------------------------------------------------------------------
** Canonical method implementations.
*/

/**
 * constructor:
 *
 *  - Save the interpreter and the script 
 *  - Register ourselves with the fragment handler singleton.
 *
 * @param interp - Reference to the encapsulated interpreter.
 * @param script - Base of the script to run at observation time.
 */
CPartialBarrierCallback::CPartialBarrierCallback(CTCLInterpreter& interp, std::string script) :
  m_Interp(interp),
  m_script(script)
{
  CFragmentHandler* pFragHandler = CFragmentHandler::getInstance();
  pFragHandler->addPartialBarrierObserver(this);
}
/**
 * Destructor
 *
 *  Unregister ourselves from the fragment handler singleton:
 */
CPartialBarrierCallback::~CPartialBarrierCallback()
{
  CFragmentHandler* pFragHandler = CFragmentHandler::getInstance();
  pFragHandler->removePartialBarrierObserver(this);
}
/*----------------------------------------------------------------------------
** Public method implementations:
*/

/**
 * operator()
 *
 *   Called when a partial barrier has been processed.  This method 
 *   constructs the command and executes it.
 *
 * @param barrierTypes - a vector of pairs. Each pair is a source id and the
 *                       type of barrier emitted by that source.
 * @param missingSources - A vector of the sources that did not contribute barriers
 *                       prior to the timeout.
 *
 */
void
CPartialBarrierCallback::operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
				     const std::vector<uint32_t>& missingSources)
{

  try {
    std::string command = makeCommand(barrierTypes, missingSources);
    m_Interp.GlobalEval(command);
  }
  catch (std::string msg) {
    std::cerr << "Partial barrier observer script error: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "Partial barrier observer script error: " << msg << std::endl;
  }
  catch (CException& e) {
    std::cerr << "Partial barrier observer script error: " << e.ReasonText() << std::endl;
  }
  catch (...) {
    std::cerr << "Partial barrier obserbver; unexopected exception type\n";
  }
}
/**
 * getCommand
 *
 *   Return the command base string
 *
 * @return std::string
 * @retval m_script
 */
std::string
CPartialBarrierCallback::getCommand() const
{
  return m_script;
}
/*-----------------------------------------------------------------------------------
** private utility methods:
*/

/**
 * makeCommand
 *
 * Constructs the command from the base command (m_script) and the vectors passed into the
 * observer.
 *
 * @param barrierTypes - Vector of pairs of source id and the type of barrier produced by that
 *                       source.
 * @param missingSources - Vector of source Ids that did not supply a barrier event fragment.
 *
 * @return std::string
 * @retval the full commandto invoke for this barrier.
 */
std::string
CPartialBarrierCallback::makeCommand(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
				     const std::vector<uint32_t>& missingSources)
{
  CTCLObject commandObj;
  commandObj.Bind(m_Interp);
  
  commandObj = m_script;	// base command

  CTCLObject* pTypeList = makeBarrierTypeList(barrierTypes);
  commandObj += *pTypeList;

  CTCLObject* pMissingList = makeMissingSourceList(missingSources);
  commandObj += *pMissingList;

  std::string command = static_cast<std::string>(commandObj);
  delete pTypeList;
  delete pMissingList;

  return command;
}
/**
 * makeBarrierTypeList
 *
 * Constructs a list of pairs.  Each pair contains a source id and the barrier type contributed
 * by that source.
 *
 * @param barrierTypes - The set of barrier types received.
 *
 * @return CTCLOBject* - pointer to the dynamically created list encapsulated in a CTCLObject*
 *                       the caller must delete this explicitly.
 */
CTCLObject*
CPartialBarrierCallback::makeBarrierTypeList(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes)
{
  CTCLObject* pList = new CTCLObject;
  pList->Bind(m_Interp);

  for (int i =0; i < barrierTypes.size(); i++) {
    CTCLObject pair;
    pair.Bind(m_Interp);

    pair += static_cast<int>(barrierTypes[i].first);
    pair += static_cast<int>(barrierTypes[i].second);

    (*pList) += pair;

  }

  return pList;
}
/**
 * makeMissingSourceList
 *
 * Takes the vector of source ids that did not contribute to an incomplete barrier and
 * turns them into a Tcl list that is encapsulated in a dynamically allocated
 * CTCLObject.
 *
 * @param missingSources - vector of missing source ids.
 *
 * @return CTCLObject* - Pointer to the generated encapsulated list  The caller must delete this.
 */
CTCLObject*
CPartialBarrierCallback::makeMissingSourceList(const std::vector<uint32_t>& missingSources)
{
  CTCLObject* pList = new CTCLObject;
  pList->Bind(m_Interp);

  for (int i = 0; i < missingSources.size(); i++) {
    (*pList) += static_cast<int>(missingSources[i]);
  }

  return pList;
}
