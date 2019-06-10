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
#include "CV977Busy.h"
#include <CCAENV977.h>


/*!
   Construction from scratch:
   @param base  -  Base address of the V977 module
   @param crate -  Optional AVME crate moduel is in.  Defaults to 0.

*/
CV977Busy::CV977Busy(uint32_t base, unsigned crate) :
  m_busy(*(new CCAENV977(base, crate)))
{
  initialize();
}
/*!
  Construction from an existing CCAENV977 module.  Still initialize:
  @param module - Existing module.
*/
CV977Busy::CV977Busy(CCAENV977& module) :
  m_busy(module)
{
  initialize();
}

/*!
   Copy construction. In this case no initialization is done as the underlying
   module is already initialized
   @param rhs  - Object we are cloning.
*/
CV977Busy::CV977Busy(const CV977Busy& rhs) :
  m_busy(rhs.m_busy)
{}


/*!
  Go busy by setting bit 1:
*/
void
CV977Busy::GoBusy()
{
  m_busy.outputSet(1);
}
/*!
   Go clear by clearing the output register:
*/

void
CV977Busy::GoClear()
{
  m_busy.outputSet(2);
  m_busy.outputSet(0);
  m_busy.outputClear();
 
}

/*
** Initialize a module for use.
*/
void
CV977Busy::initialize()
{
  m_busy.Reset();
  m_busy.controlRegister(0);
  m_busy.inputMask(0);
  m_busy.outputMask(0xfffe);	// Compatible with trigger class.
  m_busy.outputClear();
}
