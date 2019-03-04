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
#include "CMASE.h"
#include "CReadoutModule.h"

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace XLM;

////////////////////////////////////////////////////////////////////////////////////////
// Local constant definitions:
////////////////////////////////////////////////////////////////////////////////////////

// Address modifiers we are going to use:

static const uint8_t   registerAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   blockTransferAmod(CVMUSBReadoutList::a32UserBlock); 

// Clock ns per VMUSB delay tick.

static const float     vmusbClockTick(12.5);
static const float     busDelay(200/vmusbClockTick); // ticks to delay prior to getting the bus.

////////////////////////////////////////////////////////////////////////////////////////
//  Canonical function implementations:
////////////////////////////////////////////////////////////////////////////////////////
/*!
  Construction.. the baseclass constructor is just fine for us now:
*/
CMASE::CMASE() {}
/*!  Copy Construction: base class is fine for us too:

 */
CMASE::CMASE(const CMASE& rhs) :
  CXLM(rhs) {}
/*!
  Destructor is here to chain to the base class too:
*/
CMASE::~CMASE() {}

/*!
  Assignment is also just fine in the base class:


   @param rhs What we are `assigning' to *this.
   @return CMASE&
   @retval *this
*/
CMASE&
CMASE::operator=(const CMASE& rhs)
{
  return reinterpret_cast<CMASE&>(CXLM::operator=(rhs));
}

///////////////////////////////////////////////////////////////////////////////////
// CReadoutHardware interfac methods
///////////////////////////////////////////////////////////////////////////////////

/*!
   At initialization time, we load the firmware file.  It is an error for the
   firmware file to not be defined via the -firmware configuration parameter.
   Note that -firmware option is defined by our base class; CXLM.
   @param CVMUSB& controller - Reference to a VM-USB controller object.
*/
void
CMASE::Initialize(CVMUSB& controller)
{
  string firmware = m_pConfiguration->cget("-firmware");
  if(firmware == "") {
    string me    = m_pConfiguration->getName();
    string error = "Firmware file for mase object: ";
    error       += me;
    error       += " not set.  Unable to initialize the module";
    throw error;
  }

  // Load the firmware:

  loadFirmware(controller, firmware);

}
/*!
  Sets the readout list for the MASE module.  The assumption is that by now sufficient
  delay has been added that we can just get going:
  - Acquire SRAMB.
  - delay 200ns for the XLM to get off the bus since we can't check that we have it yet.
  - Read location 0 of the XLM SRAMB as the word count.
  - Do a block read using the count from the prior operation.
  - Release SRAMB and acquire BUS X to the FPGA register set.
  - delay 200ns
  - Write to the FPG base aaddress
  - Release Xbus.
  @param list - The VMUSB Readout list to which we are adding our instructions.

*/
void
CMASE::addReadoutList(CVMUSBReadoutList& list)
{
  // Figure out my bus delay in integer ticks.. fractional clock ticks result in an extra
  // tick being added:

  unsigned int busDelayTicks = static_cast<unsigned int>(busDelay);

  addBusAccess(list, CXLM::REQ_B,
	       static_cast<uint8_t>(busDelayTicks));
  uint32_t sram = sramB();	// Base address of sram B:
  list.addBlockCountRead32(sram, 0xffffffff, registerAmod); // Transfer count for block read./
  list.addMaskedCountBlockRead32(sram + sizeof(uint32_t), blockTransferAmod);  // transfer count includes self.

  // Just for fun set a known pattern in the SRAM data part of the next event..
  // it should get overwritten...

  list.addWrite32(sram, registerAmod, (uint32_t)0); // In case there's no data next event.

  addBusAccess(list, CXLM::REQ_X,
	       static_cast<uint8_t>(busDelayTicks)); //  Switch to xbus...while holding B bus??
  list.addWrite32(FPGA(), registerAmod, (uint32_t)0);
  addBusAccess(list, 0, static_cast<uint8_t>(0)); // Release all busses and off we go.

}

/*!
  Clone, as usual, is just a call to our copy constructor:
*/
CReadoutHardware*
CMASE::clone() const
{
  return new CMASE(*this);
}
