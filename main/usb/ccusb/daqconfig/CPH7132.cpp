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
#include "CPH7132.h"

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
static XXUSB::CConfigurableObject::limit LastSlot(23);

static XXUSB::CConfigurableObject::Limits SlotLimits(One, LastSlot); // CAMAC crate.


/**********************************************************************/
/*  Canonical Member function implementation                          */
/**********************************************************************/


/*!
   Construction is largely a no-op because the configuration is
   hooked to this modules at attachment time.

*/
CPH7132::CPH7132()
{
  m_pConfiguration = 0;
}
/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CPH7132::CPH7132(const CPH7132& rhs)
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
CPH7132& 
CPH7132::operator=(const CPH7132& rhs)
{
  return *this;
}
/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CPH7132::~CPH7132() {}

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
CPH7132::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  configuration.addParameter("-slot", XXUSB::CConfigurableObject::isInteger,
			     &SlotLimits, "0");
}
/*!
   Just prior to data taking, the module is initialized in accordance
   with its configuration. 
   \param controller  a reference to a CCCUSB& controller object that
                      is connected to the CCUSB we are using.


*/
void
CPH7132::Initialize(CCCUSB& controller)
{
  int slot = getIntegerParameter("-slot");

  // Clear all scalers.  Clear the mode to 3 and scaelers to 24 bit mode.
  // this mode value prevents scalers from inhibiting on overflow.

  uint16_t qx;
  controller.inhibit();
  controller.simpleWrite16(slot, 0, 17, 0x30, qx);
  controller.simpleControl(slot, 4, 11, qx);
  controller.simpleControl(slot, 12, 11, qx);
  controller.uninhibit();
}

/*!
   Add the commands required to read the module to a CCUSB readout list.
   In our case we add the following (since I don't trust Qstop mode);
   - set sequential address -> 0
   - Repeat 32 reads for the scalers.
   - Clear all scalers.

   \param list - the CCCUSBReadoutList into which to put the instructions.
*/
void
CPH7132::addReadoutList(CCCUSBReadoutList& list)
{
  int slot = getIntegerParameter("-slot");

  // there will be skew, lost counts due to the lack of a latch.
  
  list.addControl(29, 9, 29);	// I on
  list.addWrite16(slot, 1, 17, 0);
  //  list.addRepeat(slot, 15, 4, 32); // Read the scalers.
  for (int i =0; i < 32; i++) {
    list.addRead24(slot, 15, 4);
  }
  list.addControl(slot, 4, 11);
  list.addControl(29, 9, 26);	// I off
}
/*!
  Clone self:
*/
CReadoutHardware*
CPH7132::clone() const
{
  return new CPH7132(*this);
}
