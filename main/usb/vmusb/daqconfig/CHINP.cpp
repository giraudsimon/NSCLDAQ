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
Device driver for the HINP chip using XLM controller and Vmusb interface

*/

#include <config.h>
#include "CHINP.h"
#include "CReadoutModule.h"

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

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
static const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.

// FPGA mode bits:

static const uint32_t MODE_UNIFIED(1);
static const uint32_t MODE_HAVEFADC(4);


// Import the XLM namespace
using namespace XLM;


////////////////////////////////////////////////////////////////////////////////////////
//  Canonical function implementations:
////////////////////////////////////////////////////////////////////////////////////////
/*!
  Construction.. the baseclass constructor is just fine for us now:
*/
CHINP::CHINP() {}
/*!  Copy Construction: base class is fine for us too:

 */
CHINP::CHINP(const CHINP& rhs) :
  CXLM(rhs) {}
/*!
  Destructor is here to chain to the base class too:
*/
CHINP::~CHINP() {}

/*!
  Assignment is also just fine in the base class:


   @param rhs What we are `assigning' to *this.
   @return CHINP&
   @retval *this
*/
CHINP&
CHINP::operator=(const CHINP& rhs)
{
  return reinterpret_cast<CHINP&>(CXLM::operator=(rhs));
}

/**
 * onAttach
 * 
 *  Defines configuration parameters for the device:
 *
 * @param configuration - reference to the configuration database for this object.
 */
void
CHINP::onAttach(CReadoutModule& configuration)
{
  CXLM::onAttach(configuration); // base class stuff too.
  configuration.addParameter("-readsramb", XXUSB::CConfigurableObject::isBool, 
			     NULL, "false");
  configuration.addParameter("-havefadc", XXUSB::CConfigurableObject::isBool, NULL, "true");
  configuration.addParameter("-clearveto", XXUSB::CConfigurableObject::isBool, NULL, "true");
}

///////////////////////////////////////////////////////////////////////////////////
// CReadoutHardware interfac methods
///////////////////////////////////////////////////////////////////////////////////

/*!
   At initialization time, we don't load the firmware file.
   @param CVMUSB& controller - Reference to a VM-USB controller object.
*/
void
CHINP::Initialize(CVMUSB& controller)
{
  int i,j,k,status;
  uint32_t response[2048];
  size_t bytesRead;
  uint32_t base, fpga, srama;
  uint16_t test1;
  uint32_t test2;

  bool haveFadc = m_pConfiguration->getBoolParameter("-havefadc");

  uint32_t fpgaMode = MODE_UNIFIED;
  if (haveFadc) fpgaMode |=  MODE_HAVEFADC;


  base = sramA();  // base addr of VME module
  CXLM::accessBus(controller, static_cast<uint32_t>(CXLM::REQ_X));
  controller.vmeRead32(base+BUSXOwner, registerAmod, &test2);
  printf("CHINP 5/1/2012\n");
  printf("bus control X reads %x\n",test2);
  fpga = FPGA();           // establish base address of module's FPGA
  controller.vmeRead32(base+0x820048, registerAmod, &test2);
  printf("XLM serial number read from VME addr 0x%x\n",base+0x820048);
  printf("XLM serial number is 0x%x\n",test2);
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // turn off glbl_enbl
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(forcereset)); // reset the chips
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // turn off reset

  // This could just be a usleep right(?).


  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // wait
 // now set XLM to Unified readout mode, without internal ADC
  printf("Setting mode to %d 5 means you selected XLM-XXV 1 means you have an external FADC\n", fpgaMode);
  controller.vmeWrite32(fpga+ReadoutMode*4, registerAmod, fpgaMode); // ADC on, Unified On
  // prepare XLM to accept triggers
  controller.vmeWrite32(fpga+FPGA_enblA*4, registerAmod, static_cast<uint32_t>(1)); // turn on ext enbl
  controller.vmeWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(glbl_enable)); // turn on glbl_enbl

  if(m_pConfiguration->getBoolParameter("-readsramb")) {
    controller.vmeWrite32(fpga+FPGA_enblB*4, registerAmod, static_cast<uint32_t>(1)); // external enable for B.
    controller.vmeWrite32(fpga+FPGA_Bbus*4, registerAmod, static_cast<uint32_t>(glbl_enable)); // Global enable for B.
  }

  if (m_pConfiguration->getBoolParameter("-clearveto")) {
      controller.vmeWrite32(fpga+FPGA_clear_veto*4, registerAmod, static_cast<uint32_t>(1)); // clear veto_reset
  }
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
CHINP::addReadoutList(CVMUSBReadoutList& list)
{
  // Figure out my bus delay in integer ticks.. fractional clock ticks result in an extra
  // tick being added:

  unsigned int busDelayTicks = static_cast<unsigned int>(busDelay);
  uint32_t fpga = FPGA();

  addBusAccess(list, CXLM::REQ_A | CXLM::REQ_B | CXLM::REQ_X,
	       static_cast<uint8_t>(2));
  //  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, static_cast<uint32_t>(0)); // turn off glbl_enbl
  uint32_t srama = sramA();	// Base address of sram A
  uint32_t sramb = sramB();	// Base address of sram B
  //  read chip ID and channel address
  list.addMarker(static_cast<uint16_t>((fpga >> 27) & 0x07 | 0x1ff0));           // add ID for XLM slot #
  list.addBlockCountRead32(srama, static_cast<uint32_t>(0x00000ffe), registerAmod); // Transfer count for 32-bit word data
  list.addMaskedCountBlockRead32(srama + sizeof(uint32_t), blockTransferAmod);  

  if (m_pConfiguration->getBoolParameter("-readsramb")) {
    list.addMarker(static_cast<uint16_t>((fpga >> 27) & 0x07 | 0x2ff0));           // add ID for XLM slot #
    list.addBlockCountRead32(sramb, static_cast<uint32_t>(0x00000ffe), registerAmod); // Transfer count for 32-bit word data
    list.addMaskedCountBlockRead32(sramb + sizeof(uint32_t), blockTransferAmod);
  }
  // read ADC data (non-unified only)
  //  list.addBlockCountRead32(srama, 0x00000fff, registerAmod); // Transfer count for data
  //  list.addMaskedCountBlockRead32(sramb, blockTransferAmod);
  //  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, forcereset); // reset the chips
  //  list.addWrite32(fpga+FPGA_enblA*4, registerAmod, 1); // turn on ext enbl
  //  list.addDelay(50);              // allow things to quiet down before enabling discs
  //  list.addWrite32(fpga+FPGA_ABus*4, registerAmod, glbl_enable); // turn on glbl_enbl
  printf("clear_veto reg addr is %x\n",fpga+FPGA_clear_veto*4);
  // zero the word count to prevent rereading this event
  //  list.addWrite32(srama, registerAmod, static_cast<uint32_t>(0));
  if (m_pConfiguration->getBoolParameter("-clearveto")) {
    list.addWrite32(fpga+FPGA_clear_veto*4, registerAmod, static_cast<uint32_t>(1)); // clear veto_reset
  }
  addBusAccess(list, 0, static_cast<uint8_t>(0)); // Release all busses and off we go.

}

/*!
  Clone, as usual, is just a call to our copy constructor:
*/
CReadoutHardware*
CHINP::clone() const
{
  return new CHINP(*this);
}
