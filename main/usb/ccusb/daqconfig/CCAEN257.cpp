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

//  Implementation of the CCAEN257 CAEN C257 scaler module support  for CC-USB DAQ

#include <config.h>
#include "CCAEN257.h"



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
CCAEN257::CCAEN257()
{
  m_pConfiguration = 0;		// Attach will supply the configuration.
}
/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CCAEN257::CCAEN257(const CCAEN257& rhs) 
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
  \return CCAEN257&
  \retval *this
*/
CCAEN257&
CCAEN257::operator=(const CCAEN257& rhs)
{
  return *this;
}

/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CCAEN257::~CCAEN257()
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
CCAEN257::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = & configuration;

  // Create the parameters:

  configuration.addParameter("-slot",
			     XXUSB::CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-id",
			     XXUSB::CConfigurableObject::isInteger,
			     &IdLimits, "0");

  configuration.addParameter("-insertid",
			     XXUSB::CConfigurableObject::isBool,
			     NULL,
			     "false"); // Typically scalers don't insert ids.
  configuration.addParameter("-readinhibit",
			     XXUSB::CConfigurableObject::isBool,
			     NULL,
			     "true"); // May be needed to get good reads.

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
CCAEN257::Initialize(CCCUSB& controller)
{
  // The only initialization we need to do is to clear the scaler.
  // The LAM will be disabled.

  int slot = getIntegerParameter("-slot");

  // Slot must be non zero else it has not been set:

  if (slot  == 0) {
    string name = m_pConfiguration->getName();
    string error = "The CAEN C257 module: ";
    error       += name;
    error       += " did not have its mandatory -slot parameter specified";
    throw error;
  }

  //  Clear the module and see if there's a module plugged in:

  uint16_t qx;
  controller.simpleControl(slot, 0, 9, qx);

  // Lack of an X indicates the module may not exist.. but we continue anyway:

  if ((qx & CCCUSB::X) == 0) {
    cerr << "Warning I got no X response from CAEN C257: " 
	 << m_pConfiguration->getName() 
	 << " Be sure the -slot parameter is correct\n";
    cerr << "Continuing in spite of this\n";
  }

  // Disable the LAM:

  controller.simpleControl(slot, 0, 24, qx);

}
/*!
   Add the commands required to read the module to a CCUSB readout list.
   In our case the read has a few dependencies:
   - If -insertid is true, we add the marker to the input stream.
   - If -readinhibit is true we Inhibit, use F2 to do the reads then uninhibit
   - If -readinhibit is false, we use F0 to do the reads and then manually clear
     with F9.
   

   \param list - the CCCUSBReadoutList into which to put the instructions.

   \note Presumably the Initialize function has ensured the slot is defined
         so we don't double check that:

*/
void
CCAEN257::addReadoutList(CCCUSBReadoutList& list)
{
  int slot = getIntegerParameter("-slot");

  // If -insertid insert the id marker:

  if  (getBoolParameter("-insertid")) {
    uint16_t id = getIntegerParameter("-id");
    list.addMarker(id);
  }

  // Readout depends on -readinhibit:

  if (getBoolParameter("readinhibit")) {
    list.addControl(29, 9, 24);	// Inhibit crate.
    list.addQScan(slot, 0, 2, 16); // Read all channels.. read of a15 clears.
    list.addControl(29, 9, 26);	   // Uninhibit crate.
  }
  else {
    for (int i=0; i < 16; i++) {
      list.addRead24(slot, i, 0);
    }
    list.addControl(slot, 0, 9);   // Manually clear the counters/lam.
  }
}
/*!
  The interface requires that we know how to copy ourself for 
  'virtual' copy construction e.g.:

*/
CReadoutHardware*
CCAEN257::clone() const
{
  return new CCAEN257(*this);
}
