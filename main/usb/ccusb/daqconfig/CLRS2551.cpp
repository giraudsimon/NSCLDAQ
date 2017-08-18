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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>
#include "CLRS2551.h"

#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;


/************************************************************************/
/*                        Local constants                               */
/************************************************************************/

// Configuration value constants and limits:

static XXUSB::CConfigurableObject::limit One(1); 
static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit FULL16(0xffff);
static XXUSB::CConfigurableObject::limit LastSlot(23);

static XXUSB::CConfigurableObject::Limits SlotLimits(One, LastSlot); // CAMAC crate.
static XXUSB::CConfigurableObject::Limits IdLimits(Zero, FULL16);

/*************************************************************************/
/*    Canonical member implementation                                    */
/*************************************************************************/

/*!
   Construction - initialize the configuration to null
*/
CLRS2551::CLRS2551()
{
  m_pConfiguration = 0;

}
/*!
   Copy construction
*/
CLRS2551::CLRS2551(const CLRS2551& rhs)
{
  m_pConfiguration = 0;

  // If the sourcde has a configuration deep copy it:
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration)); 
  }
}
/*!
   Assignemnt:
*/
CLRS2551&
CLRS2551::operator=(const CLRS2551& rhs)
{
  if ((this != &rhs) && (rhs.m_pConfiguration)) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
  return *this;
}
/*!
   Destruction is a no-op ast here are currently issues with deleting a
   configuration. This implies a memory leaK!!
*/
CLRS2551::~CLRS2551()
{
}

/********************************************************************/
/*   Implementing the CReadoutHardware interface                    */
/*******************************************************************/

/*!
   Attach the module to a configuration.  This is when the 
   module registers its options as well as validators
   @param configuration - Configuration object to be used by this object.
*/
void
CLRS2551::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration; // store for later.

  // Create the configuration options:

  configuration.addParameter("-slot", XXUSB::CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-id", XXUSB::CConfigurableObject::isInteger,
			     &IdLimits,"0");
  configuration.addParameter("-insertid", XXUSB::CConfigurableObject::isBool,
			     0, "false");
  configuration.addParameter("-cumulative", XXUSB::CConfigurableObject::isBool,
			     0, "false");
}

/*!
   Called just prior to data taking.  The module must be initialized
   Configuration parameter errors are also checked at this time.
   Errors are thrown as exceptions to ensure that data taking is not started.
   @param controller CCUSB Controller object
*/
void
CLRS2551::Initialize(CCCUSB& controller)
{
  // Validate parameters.

  int slot = getIntegerParameter("-slot");
  if(!slot) {
    throw "An LRS2551 module has not had its slot configured (-slot missing)";
  }

  // Clear the module counters.
  // Can't use checked control because there's no Q:

  checkedControl(controller,
		 slot, 0, 9,
		 string("Failed to initialize LRS 2551 n=%d, a=%d f=%d (%d)"),
		 false);
}
/*!
   Add the readout commands to the readout list.
   There's some conditionalization:
   - the function used to do the read depends on the value of -cumulative
   - whether or not the marker is inserted depends on the value of -insertid.

   @param list CCUSBReadout list that drives the read.
*/
void
CLRS2551::addReadoutList(CCCUSBReadoutList& list)
{
  int slot         = getIntegerParameter("-slot");
  int id           = getIntegerParameter("-id");
  bool insertid    = getBoolParameter("-insertid");
  bool cummulative = getBoolParameter("-cumulative");

  int function     = cummulative ? 0 : 2;    // not cumulative f2 clears.

  if(insertid) {
    list.addMarker(id);
  }
  // Can't do a Q-scan as that is only 16 bits wide in software support.

  for (int i = 0; i < 12; i++) {
    list.addRead24(slot, i, function, false);
  }
    
}
/*!
   Clone self:
*/
CReadoutHardware*
CLRS2551::clone() const
{
  return new CLRS2551(*this);
}
