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
#include <config.h>
#ifndef __DAQDATATYPES_H
#include <daqdatatypes.h>
#ifndef __DAQDATATYPES_H
#define __DAQDATATYPES_H
#endif
#endif
#include "CCAENV262Trigger.h"
#include <CaenIO.h>

/*!
   Construct the trigger that uses a CAEN V262 module.  
   This makes a new CCaenIO object
   \param base   - base address of the CAEN V262
   \param crate  - Number of the VME crate in which the module lives.
                   this defaults to zero.

*/
CCAENV262Trigger::CCAENV262Trigger(uint32_t base, unsigned crate) :
  m_TriggerModule(*(new CCaenIO(base, crate)))
{
}

/*!
  Construct from an existing IO Module
  \param module - THe module to be used for trigger.
*/
CCAENV262Trigger::CCAENV262Trigger(CCaenIO& module) :
  m_TriggerModule(module)
{}

/*!
  Copy construction works because CCaenIO has a copy constructor:
*/
CCAENV262Trigger::CCAENV262Trigger(const CCAENV262Trigger& rhs) :
  m_TriggerModule(*(new CCaenIO(rhs.m_TriggerModule)))
{
}


/*!
  Check for true on input 1.. if found,
  pulse the acknowledge and return true.. return false otherwise.
*/
bool
CCAENV262Trigger::operator()()
{
  if (m_TriggerModule.ReadInput(0) != 0) {
    m_TriggerModule.PulseOutput(2);
    return true;
  }
  else {
    return false;
  }
}
