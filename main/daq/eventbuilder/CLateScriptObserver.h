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

#ifndef CLATESCRIPTOBSERVER_H
#define CLATESCRIPTOBSERVER_H

/**
 * @file CLateScriptObserver.h 
 * @brief  Defines a class that can observe data late fragments and call a script
 */


#include "CFragmentHandler.h"
#include <string>

// Forward definitions:

class CTCLInterpreter;

/**
 * @class CLateScriptObserver
 * @brief Late fragment observer that executes a script.
 *
 *  This is an observer that self registsers with the fragment handler
 *  When a late fragment is sensed, it dispatches to a Tcl script.
 *  The script has the following appended to it:
 *  - source id of fragment.
 *  - timestamp of fragment.
 *  - newest timestamp seen by the fragment handler.
 *  - Binary object that is the fragment body.
 */
class CLateScriptObserver : public CFragmentHandler::DataLateObserver
{
  // private data:

private:
  CTCLInterpreter& m_interpreter;
  std::string      m_scriptPrefix;	// What the user supplies.

public:
  CLateScriptObserver(CTCLInterpreter& interp, std::string prefix);
  virtual ~CLateScriptObserver();

  // virtual method overrides:

public:
    virtual void operator()(const ::EVB::Fragment& fragment,  uint64_t newest);
    
    std::string getScript() const;

  // Utility methods:

private:
  std::string createCommand(const ::EVB::Fragment& fragment, uint64_t newest);

  
};


#endif
