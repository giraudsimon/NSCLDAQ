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
#include "CV977Trigger.h"
#include <CCAENV977.h>

///////////////////////////////////////////////////////////////////////////////////


/*!
   Construct the trigger.  Initialization of the module is deferred
   until the setup member is called.

   A note about the m_ownModule member.  This will be true if at 
   construction time, the m_pModule points to storage that should be 
   deleted.  This is really more important to be able to support the
   constructor around an existing module as that pointer could be to either
   dynamic storage we manage, dynamic storage that the caller manages or 
   static/automatic storage.

   \param base   - Base address of the module in the VME crate.
   \param crate  - Number of the VME crate that holds the module.
                   This defaults to zero which is suitable for single crate systems.
*/
CV977Trigger::CV977Trigger(uint32_t base, unsigned crate) :
  m_pModule(new CCAENV977(base, crate)),
  m_ownModule(true)
{
}
/*!
  Construct the trigger from an exiting CCAENV977 object.
  \param pModule  - The module to use for the trigger.
  \param deleteme - If true, when this object is destroyed, the
                    object pointed to by pModule is deleted too.
		    Defaults to true.
*/
CV977Trigger::CV977Trigger(CCAENV977* pModule, bool deleteme) :
  m_pModule(pModule),
  m_ownModule(deleteme)
{}

/*!
   Destruction - if the module storage is owned by this class, the
   module storage must be deleted:
*/
CV977Trigger::~CV977Trigger()
{
  if (m_ownModule) {
    delete m_pModule;
  }

}
////////////////////////////////////////////////////////////////////////////////
/*!
  setup the module for our purposes:
  - We really need a control register of 0. Too bad if someone else is sharing
    this module with us and needs a different control register setting,
    they can get their own damned module.
  - Clear the bottom bits in the input and output masks.
  - Clear the output register...for now too bad if that hurts anyone sharing this
    module.
*/
void
CV977Trigger::setup()
{
  m_pModule->controlRegister(0);
  UShort_t inputMask = m_pModule->inputMask();
  inputMask &= 0xfffe;
  m_pModule->inputMask(inputMask);

  UShort_t outputMask = m_pModule->outputMask();
  outputMask &= 0xfffe;
  m_pModule->outputMask(outputMask);
  UShort_t outputs    = m_pModule->outputSet();
  outputs &= 0xfffe;
  m_pModule->outputSet(outputs);

}
/*!
   Check for the trigger.. If the trigger fires;
   return true,
   else return false.

 
*/
bool
CV977Trigger::operator()()
{
  bool triggered = (m_pModule->singleHitRead() & 1) != 0;

  return triggered;
}
