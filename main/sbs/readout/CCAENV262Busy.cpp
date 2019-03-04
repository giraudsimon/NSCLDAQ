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
#include "CCAENV262Busy.h"
#include <CaenIO.h>

/*!
  Construct from a base address and optional crate:
  \param base  - Base address of the module
  \param crate - Optional VME crate number Defaults to zero.
*/
CCAENV262Busy::CCAENV262Busy(uint32_t base, unsigned crate) :
  m_busy(*(new CCaenIO(base, crate)))
{
  
}
/*!
  Construct from a pre-existing CAEN V262 I/O module
  \param module previously constructed module.
*/
CCAENV262Busy::CCAENV262Busy(CCaenIO& module) :
  m_busy(module)
{}
/*!
   Copy construction:
*/
CCAENV262Busy::CCAENV262Busy(const CCAENV262Busy& rhs) :
  m_busy(rhs.m_busy)
{}

/////////////////////////////////

/*!
   Indicate we are going busy by pulsing SHP0
*/
void
CCAENV262Busy::GoBusy()
{
  m_busy.PulseOutput(0);
}
/*!
   Indicate we're going ready by pulsing clears
   and going clear outputs:
*/
void
CCAENV262Busy::GoReady()
{
  m_busy.PulseOutput(3);
  m_busy.PulseOutput(1);
}

/**
 * Virtual function that indicatse we are going clear/ready:
 */
void
CCAENV262Busy::GoClear()
{
  GoReady();
}
