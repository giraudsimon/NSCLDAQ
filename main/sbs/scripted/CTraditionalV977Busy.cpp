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
#include "CTraditionalV977Busy.h"
#include "CCAENV977.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

// Constants:

static const UShort_t GOINGREADY(1);
static const UShort_t GOINGBUSY(2);
static const UShort_t CLEARS(0xfffc);

/*!
   Construct a busy module for the traditional readout system.
   The busy module is based on a CAEN V977 I/O register module.
   @param base
      VME base address set by the rotary switches of the module.
   @param crate
      VME crate in which the module lives (defaults to 0).

*/
CTraditionalV977Busy::CTraditionalV977Busy(ULong_t base, UShort_t crate) :
  m_Module(*(new CCAENV977(base, crate)))
{
}
/*!
   Destroy the module by deleting it.
*/
CTraditionalV977Busy::~CTraditionalV977Busy()
{
  delete &m_Module;
}
/*!
  Set the busy (pulse GOINGBUSY)
 */
void
CTraditionalV977Busy::Set()
{
  PulseOutputs(GOINGBUSY);
}
/*!
  Clear the busy (Pulse GOINGREADY)
*/
void
CTraditionalV977Busy::Clear()
{
  PulseOutputs(GOINGREADY);
}
/*!
   Pulse module clear (CLEARS):
*/
void
CTraditionalV977Busy::ModuleClear()
{
  PulseOutputs(CLEARS);
}

// Utility to pulse a mask of bits inthe output register:

void
CTraditionalV977Busy::PulseOutputs(UShort_t mask)
{
  m_Module.outputSet(mask);
  m_Module.outputSet(0);
}
