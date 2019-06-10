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

// Implementation of the CAD811 Ortec AD811 module for the CC-USB DAQ


#include <config.h>
#include "CAD811.h"

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


/**********************************************************************/
/*  Canonical Member function implementation                          */
/**********************************************************************/


/*!
   Construction is largely a no-op because the configuration is
   hooked to this modules at attachment time.

*/
CAD811::CAD811() 
{
  m_pConfiguration = 0;

}
/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CAD811::CAD811(const CAD811& rhs) 
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}

/*!
  Assignment should probably have been made illegal.  We'll keep our
  current idea of the configuration.
  \param rhs  - The module that is being assigned to us.
  \return CAD811&
  \retval *this
*/
CAD811&
CAD811::operator=(const CAD811& rhs)
{
  return *this;
}

/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CAD811::~CAD811()
{
}
/*************************************************************************/
/*  Implementing the CReadoutHardware Interface                          */
/*************************************************************************/


/*!
   This function is called when the module is attached to its configuration.
   It is responsible for defining the set of parameters that can be
   set, their types and validity checkers that enforce any value/type
   constraints on the configuration values, as well as parameter initial
   values.

   See the header file for a definition of the set of parameters, types
   and defaults that will be defined for this module.

   \param configuration - reference to the configuration object that
                          will be associated with this module.

*/
void
CAD811::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  configuration.addParameter("-slot", XXUSB::CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-id", XXUSB::CConfigurableObject::isInteger,
			     &IdLimits, "0");
}
/*!
   Just prior to data taking, the module is initialized in accordance
   with its configuration. If a parameter array is disabled, I'm not going
   to bother loading it.  This reduces the configuration time to the set
   of operations that are absolutely necessary.

   Prior to intializing the module, the crate is inhibited and the module
   cleared to ensure it remains not-busy during initialization.
   Following initialization, the crate Inhibit is released.

   \param controller  a reference to a CCCUSB& controller object that
                      is connected to the CCUSB we are using.


*/
void
CAD811::Initialize(CCCUSB& controller)
{
  int slot = getIntegerParameter("-slot");

  // Slot must be non zero or the module was not configured:

  if(!slot) {
    throw "An AD811 module has not been cofigured (-slot missing).";
  }
  // The module requires no hardware initialization..other than a clear.

  
  checkedControl(controller, slot, 12, 11,
		 "Clearing AD811 in slot %d");
  
}

/*!
   Add the commands required to read the module to a CCUSB readout list.
   In our case we add the following:
   - Insert the marker in the event.
   - Read all registrees with f2 so the module auto-clears.

   \param list - the CCCUSBReadoutList into which to put the instructions.
*/
void
CAD811::addReadoutList(CCCUSBReadoutList& list)
{
  int slot = getIntegerParameter("-slot");
  int id   = getIntegerParameter("-id");

  // Insert the markers:

  list.addMarker(id);

  // Read  & clear the channels:

  for (int  i = 0; i < 8; i++) {
    list.addRead16(slot, i, 2, false); // user must set the delay.
  }

}


/*!

   Clone ourself.  This is essentially a virtual contstructor.
   We just do a new *this

*/
CReadoutHardware*
CAD811::clone() const
{
  return new CAD811(*this);
}
