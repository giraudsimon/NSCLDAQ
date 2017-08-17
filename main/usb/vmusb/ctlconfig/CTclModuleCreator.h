/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CTCLMODULECREATOR_H
#define CTCLMODULECREATOR_H

/**
 * @file CTclModuleCreator.h
 * @brief Creates CTclControlModule objects...wrappers for Tcl driver instances.
 * @author Ron Fox <fox@nscl.msu.edu>
 */


#include <CModuleCreator.h>
#include <CControlHardware.h>
#include <memory>

class CTCLInterpreter;

/**
 * Concrete CModuleCreator that created Tcl driver wrapper objects.
 */
class CTclModuleCreator : public CModuleCreator
{
  private:
    CTCLInterpreter& m_interpreter;

public:
  CTclModuleCreator(CTCLInterpreter& interp);
  virtual CControlHardware* operator()();
};


#endif
