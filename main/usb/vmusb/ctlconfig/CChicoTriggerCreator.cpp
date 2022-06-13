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
#include "CChicoTriggerCreator.h"
#include "ChicoTrigger.h"

#include <memory>

/**
 * operator()
 *   Create a ChicoTrigger object
 *
 * @param name - The name of the module to create.
 *
 * @return CControlHardware*
 */
CControlHardware*
CChicoTriggerCreator::operator()()
{
  return new ChicoTrigger;
}
