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
#ifndef CPARTIALBARRIERCALLBACK_H
#define CPARTIALBARRIERCALLBACK_H

/**
 * @file CPartialBarrierCallback.h
 * @brief Define a partial barrier observer that executes Tcl scripts.
 */

#include "CFragmentHandler.h"
#include <string>

// forward definitions:

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CPartialBarrierCallback
 *
 * This is  an observer for partial barriers.  It supports storing an encapsulated
 * interprer and the base part of a script that will be invoked in that interpreter
 * when a partial barrier is processed.
 *
 *  The script has two command words appended to it.  The first work is
 *  a list of pairs of source ids and barrier types emitted by those sources.
 *  The second additional word is a list of missing source ids.
 *
 *  This class is a subclass of CFragmentHandler::PartialBarrierObserver
 *  and construction/desctruction auto-registers/auto-deregisters.
 *
 */
class CPartialBarrierCallback : public CFragmentHandler::PartialBarrierObserver
{
  // private data

private:
  CTCLInterpreter& m_Interp;
  std::string      m_script;

  // Supported canonicals:

public:
  CPartialBarrierCallback(CTCLInterpreter& interp, std::string script);
  virtual ~CPartialBarrierCallback();

  // unsupported canonicals:

private:
  CPartialBarrierCallback(const CPartialBarrierCallback&);
  CPartialBarrierCallback& operator=(const CPartialBarrierCallback&);
  int operator==(const CPartialBarrierCallback&) const;
  int operator!=(const CPartialBarrierCallback&) const;

  // Public interface:

public:
  virtual void operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
			  const std::vector<uint32_t>& missingSources);

  std::string getCommand() const;

  // private utilities:

  std::string makeCommand(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes, 
			  const std::vector<uint32_t>& missingSources);
  CTCLObject* makeBarrierTypeList(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes);
  CTCLObject* makeMissingSourceList(const std::vector<uint32_t>& missingSources);

};


#endif
