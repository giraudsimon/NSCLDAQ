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
/*
Device driver for the WASHINGTON U. PSD-ADC chip
    using XLM controller and Vmusb interface

*/

#include <config.h>
#include "CPSD.h"
#include "CReadoutModule.h"

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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
static const float     busDelay(10/vmusbClockTick); // ticks to delay prior to getting the bus.
// map some XLM register addresses
static const uint32_t  BUSAOwner  (0x810000); // Shows who has bus A (SRAM A)
static const uint32_t  BUSBOwner  (0x810004); // Shows who has bus B (SRAM B)
static const uint32_t  BUSXOwner  (0x810008); // Shows who has bus X (FPGA).
static const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.
// bits in XLM registers
static const uint32_t ForceOffBusForce(0x00000001); // Force all but VME off bus.
////////////////////////////////////////////////////////////////////////////////////////
//  Canonical function implementations:
////////////////////////////////////////////////////////////////////////////////////////
/*!
  Construction.. the baseclass constructor is just fine for us now:
*/
CPSD::CPSD() {}
/*!  Copy Construction: base class is fine for us too:

 */
CPSD::CPSD(const CPSD& rhs) :
  CXLM(rhs) {}
/*!
  Destructor is here to chain to the base class too:
*/
CPSD::~CPSD() {}

/*!
  Assignment is also just fine in the base class:


   @param rhs What we are `assigning' to *this.
   @return CPSD&
   @retval *this
*/
CPSD&
CPSD::operator=(const CPSD& rhs)
{
  return reinterpret_cast<CPSD&>(CXLM::operator=(rhs));
}

///////////////////////////////////////////////////////////////////////////////////
// CReadoutHardware interfac methods
///////////////////////////////////////////////////////////////////////////////////

/*!
   At initialization time, we don't load the firmware file.
   @param CVMUSB& controller - Reference to a VM-USB controller object.
*/
void
CPSD::Initialize(CVMUSB& controller)
{
  int i,j,k,status;
  uint32_t response[2048];
  size_t bytesRead;
  uint32_t base, fpga, srama;
  uint16_t test1;
  uint32_t test2;


  base = sramA();  // base addr of VME module
  CXLM::accessBus(controller, static_cast<uint32_t>(CXLM::REQ_X));
  controller.vmeRead32(base+XLM::BUSXOwner, registerAmod, &test2);
  printf("bus control X reads %x\n",test2);
  fpga = FPGA();           // establish base address of module's FPGA
  controller.vmeRead32(base+0x820048, registerAmod, &test2);
  printf("XLM serial number read from VME addr 0x%x\n",base+0x820048);
  printf("XLM serial number is 0x%x\n",test2);
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)0); // turn off glbl_enbl
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)forcereset); // reset the chips
  controller.vmeWrite32(fpga+FPGA_enblA*4, registerAmod, (uint32_t)1); // turn on ext enbl
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)glbl_enable); // turn on glbl_enbl
  // now set XLM to Unified readout mode, without internal ADC
  controller.vmeWrite32(fpga+ReadoutMode*4, registerAmod, (uint32_t)1); // ADC off, Unified On

  CXLM::accessBus(controller, static_cast<uint32_t>(0)); // release bus

}

/*!
  Sets the readout list for the HINP-XLM module.  The assumption is that by now sufficient
  delay has been added that we can just get going:
  - Acquire SRAMA.
  - delay 200ns for the XLM to get off the bus since we can't check that we have it yet.
  - Read location 0 of the XLM SRAMA as the word count.
  - Do a block read using the count from the prior operation.
  - Release SRAMA and acquire BUS X to the FPGA register set.
  - delay 200ns
  - Turn off global enable, enable external trigger, then turn on global enable
  - Release Xbus.
  @param list - The VMUSB Readout list to which we are adding our instructions.

*/
void
CPSD::addReadoutList(CVMUSBReadoutList& list)
{
  // Figure out my bus delay in integer ticks.. fractional clock ticks result in an extra
  // tick being added:

  unsigned int busDelayTicks = static_cast<unsigned int>(busDelay);
  uint32_t fpga = FPGA();

  addBusAccess(list, CXLM::REQ_A | CXLM::REQ_B | CXLM::REQ_X,
	       static_cast<uint8_t>(2));
  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)0); // turn off glbl_enbl
  uint32_t srama = sramA();	// Base address of sram A
  uint32_t sramb = sramB();	// Base address of sram B
  // note that first word of memory is count of 16-bit elements of sata
  //  by masking off LSB, we get a count of 32-bit words to read
  list.addBlockCountRead32(srama, (uint32_t)0x00000ffe, registerAmod); // Transfer count for combined tag/data
  //  read chip ID and channel address
  list.addMaskedCountBlockRead32(srama + sizeof(uint32_t), blockTransferAmod);
  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)forcereset); // reset the chips
  list.addWrite32(fpga+FPGA_enblA*4, registerAmod, (uint32_t)1); // turn on ext enbl
  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, (uint32_t)glbl_enable); // turn on glbl_enbl
  addBusAccess(list, 0, static_cast<uint8_t>(0)); // Release all busses and off we go.

}

/*!
  Clone, as usual, is just a call to our copy constructor:
*/
CReadoutHardware*
CPSD::clone() const
{
  return new CPSD(*this);
}
