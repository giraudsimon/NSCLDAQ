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
#ifndef CCOMPLETEBARRIERCALLBACK_H
#define CCOMPLETEBARRIERCALLBACK_H

/**
 * @file CCompleteBarrierCallback.h
 * @brief Define an observer that provides the ability to call a
 *        tcl script on a complete barrier.
 */

#include "CFragmentHandler.h"
#include <string>

// Forward definitions:

class CTCLInterpreter;


/**
 * @class CCompleteBarrierCallback
 *
 *  This is an observer for complete callbacks.  It supports storing an encapsulated
 *  intepreter object as well as a script and invokes the script when the barrier 
 *  happens. The script has appended to it a list  of pairs where the pairs are the
 *  source Id and barrier type emitted by that source.
 *
 *  This class is therefore a subclass of CFragmentHandler::BarrierObserver and auto
 *  registers/de-registers with the fragment handler singleton.
 */

class CCompleteBarrierCallback : public CFragmentHandler::BarrierObserver
{
  // private data:

private:
  CTCLInterpreter&   m_Interpreter;
  std::string        m_script;

  // Canonicals that are implemented:

public:
  CCompleteBarrierCallback(CTCLInterpreter& interp, std::string command);
  virtual ~CCompleteBarrierCallback();

  // canonicals that are not supported:

private:
  CCompleteBarrierCallback(const CCompleteBarrierCallback&);
  CCompleteBarrierCallback& operator=(const CCompleteBarrierCallback&);
  int operator==(const CCompleteBarrierCallback&) const;
  int operator!=(const CCompleteBarrierCallback&) const;

  // public membes (specifically the observer callback):

public:
  virtual void operator()(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes);
  std::string getCommand() const;

  // private utilities:

  std::string makeScript(const std::vector<std::pair<uint32_t, uint32_t> >& barrierTypes);

private:
  

};




#endif
