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
#include "CTraditionalV977Trigger.h"
#include "CCAENV977.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
   Construct a module given the base address and
   crate defining where the module lives.
   @param base
      Module base address in the VME crate.
   @param crate
      VME crate in which the module lives.  defaults to 0.
*/
CTraditionalV977Trigger::CTraditionalV977Trigger(ULong_t base, UShort_t crate) :
  m_Module(*(new CCAENV977(base, crate))),
  m_LastPattern(0)
{
}


/*!
   Destroy the trigger by destroying the module underlying it.
*/
CTraditionalV977Trigger::~CTraditionalV977Trigger()
{
  delete &m_Module;
}
/*!
  Initialize the trigger module.. the module is set into pattern mode with
  the output mask all 1's. All inputs, outputs and multihits are cleared.
   The saved last pattern is also cleared.
*/
void
CTraditionalV977Trigger::Initialize()
{
  m_Module.Reset();
  Disable();
  m_Module.outputMask(0xffff);
  Clear();

}
/*!
   Enable the trigger.  This means removing the mask from the gate.
*/
void
CTraditionalV977Trigger::Enable()
{
  m_Module.controlRegister(CCAENV977::control_Pattern);
}
/*!
   Disable the trigger.  This means setting the gate mask so that gates
   are now ignored.
*/
void
CTraditionalV977Trigger::Disable()
{
  m_Module.controlRegister(CCAENV977::control_Pattern |
                           CCAENV977::control_gateMask);
}

/*!
   Check the trigger.  Returns true if there is a trigger.
   The single hit register is read into the m_LastPattern short
   If non zero, this returns true indicating a trigger.
   The pattern word is there for the application to read via the
   getTriggerPattern member.
*/
bool
CTraditionalV977Trigger::Check()
{
  return (m_LastPattern = m_Module.singleHitReadAndClear());
}
/*!
   Clear the trigger.. the single and multi hit registers are cleared.
   The saved pattern: m_LastPattern is undisturbed.
*/
void
CTraditionalV977Trigger::Clear()
{
  m_Module.singleHitReadAndClear();
  m_Module.multiHitReadAndClear();
  m_Module.outputSet(0);
}

/*!
   Return the most recent trigger pattern read from the single hit register.
   This is modified each time the Check() member is called.
*/
UShort_t
CTraditionalV977Trigger::getTriggerPattern() const
{
  return m_LastPattern;
}
