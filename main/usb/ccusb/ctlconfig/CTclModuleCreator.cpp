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

/**
 * @file CTclModuleCreator.cpp
 * @brief Implementation of the module creator for CTclControlModule objects.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include "CTclModuleCreator.h"
#include "CTclControlModule.h"
#include <TCLInterpreter.h>

CTclModuleCreator::CTclModuleCreator(CTCLInterpreter& interp)
  : m_interp(interp)
{}

/**
 * operator()
 *   The creational
 *
 * @param name - Name of the module.
 * @return CControlHardware* Pointer to the newly created module.
 */
  CControlHardware*
CTclModuleCreator::operator()()
{
  return (new CTclControlModule(m_interp));
}
