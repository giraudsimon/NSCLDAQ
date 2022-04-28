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

#ifndef _CTCLMODULE_H
#define _CTCLMODULE_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward class definitions:

class CReadoutModule;
class CCCUSB;
class CCCUSBReadoutList;
class CTCLInterpreter;

/**
 * This class encapsulates a Tcl device support module.  Tcl device support modules
 * are a command ensemble with two methods:  Initialize, addReadoutList
 * The command ensemble normally is provided by an object from one of the Tcl OO extensions
 * such as snit, itcl or even native TclOO.  As such each class that implements device support
 * either inherits or must implement its own configuration scheme (itcl, and snit inherit from their
 * extension architecture).
 *
 * This module does not have/maintain its own configuration therefore.
 *
 *  The normal method of using this wrapper is to create an object/instance, configure it and then
 *  addTclDriver command-name  to wrap the object for use.
 */
class CTclModule : public CReadoutHardware
{
  std::string      m_command;
  CTCLInterpreter* m_pInterp;
public:
  CTclModule(std::string command, CTCLInterpreter& interp);
  CTclModule(const CTclModule& rhs);
  virtual ~CTclModule();
  CTclModule& operator=(const CTclModule& rhs);
private:
  int operator==(const CTclModule& rhs) const;
  int operator!=(const CTclModule& rhs) const;

  // Readout interface. Remember that only initialize, addReadoutList and clone() are actually
  // used.
public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual void onEndRun(CCCUSB& controller);
  virtual CReadoutHardware* clone() const;

private:
  std::string swigPointer(void* p, std::string typeName);
  
};

#endif
