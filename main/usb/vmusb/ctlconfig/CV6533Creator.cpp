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

#include "CV6533Creator.h"
#include "CV6533.h"
#include <memory>

/**
 * @file CV6533Creator.cpp
 * @brief Implements the creational for a CV6533 HV control module.
 */

  CControlHardware*
CV6533Creator::operator()(void* unused)
{
  return (new CV6533);
}
/**
 * describe the module created.
 */
std::string
CV6533Creator::describe() const
{
  return "v6533 - CAEN VME High voltage controller";
}