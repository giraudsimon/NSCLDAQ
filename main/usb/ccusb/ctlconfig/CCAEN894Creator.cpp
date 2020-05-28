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
#include "CCAEN894Creator.h"
#include "C894.h"

/**
 * @file CCAEN894Creator.cpp
 * @brief Creates a C894 for the module factory.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

/**
 * operator()
 *   The creational method
 *
 * @param name - the name to assign the new module object.
 */
CControlHardware*
CCAEN894Creator::operator()(void* unused)
{
  return  (new C894);
}

/**
 * describe
 *    Provide a description of the module created by this:
 */
std::string
CCAEN894Creator::describe() const
{
  return "caen894 - leading edge discriminator";
}