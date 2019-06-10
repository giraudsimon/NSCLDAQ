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

#include "CVMUSBCreator.h"
#include "CVMUSBModule.h"
#include <memory>

/**
 * operator()
 *    Creates the module.
 *
 * @param name - The name of the  module.
 * @return CControlHardware* - Pointer to the created CVMUSBModule.
 */
CControlHardware*
CVMUSBCreator::operator()()
{
  return (new CVMUSBModule);
}
