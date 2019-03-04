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

#include "CSIS3820.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


static const int ModuleSize(0xb00);


// Below are register offsets into the module:

				// CSR
static const unsigned long CSR(0);
static const unsigned int  LEDOn(1);
static const unsigned int  LEDOff(0x10000);
static const unsigned int  LEDIsOnMask(1);
static const unsigned int  ISARMED(0x1000000);
static const unsigned int  SCALERENABLED(0x10000);
static const unsigned int  MCSENABLED(0x40000);
static const unsigned int  ENABLEREFPULSER(0x40);
static const unsigned int  DISABLEREFPULSER(0x400000);
static const unsigned int  ENABLETESTPULSER(0x30);
static const unsigned int  DISABLETESTPULSER(0x300000);
static const unsigned int  REFPULSERON(0x40);
static const unsigned int  TESTPULSERON(0X30);

				// Module Id/Firmware register

static const unsigned long IdAndFirmware(0x4);
				// The id part of the register:
static const unsigned int  IDShift(16);
static const unsigned int  IDMask(0xffff); // After shift that is.
				// The revlevel part:
static const unsigned int  REVMask(0xff);  // Revision fields 8bits
static const unsigned int  MAJORShift(8); 


                               // Acquisition mode register

static const unsigned int ACQMode(0x100);
static const unsigned int OperatingModeMask(0x70000000);
static const unsigned int InputModeMask(0x0070000);
static const unsigned int LatchModeMask(0x00000070);
static const unsigned int NONCLEARINGMODE(1);

// bits 32-47 of channel 1/17 48 bit channels:

static const unsigned int HIGHBITS(0x210);

				//  Reset key register.

static const unsigned int RESET(0x400);

				// Clear channels key register.

static const unsigned int CLEAR(0x40c);   //???

				// Latch key register...

static const unsigned int LATCH(0x410);

				// Arm key register...

static const unsigned int ARM(0x414);


				// key registers to enable/disable.

static const unsigned int ENABLE(0x418);
static const unsigned int DISABLE(0x41c);


				// The scaler counters:

static const unsigned int SHADOWCOUNTERS(0x800); // Shadow regs.
static const unsigned int COUNTERS(0xa00);  // Live counters.
static const unsigned int ChannelCount(32); // # channels.

/*!
   Construct a module object.  The module object is the software entity
   through which the device will be controlled.

   The default mode of the device is a latching scaler with
   inputs in InputLatchInhibitAllAndLatch (inputs configured to
   support external latch as well as inhibits on the external latch
   and all counters.

   We also arrange the LNE source to be front panel (VME Key is
   always a valid option).

   \param base (unsigned long [in])
      base address of the module.
   \param crate (int [in] = 0):
      Crate in which the module lives.

   \throw string
     If the module cannot be mapped a string describing the problem is thrown
*/
CSIS3820::CSIS3820(unsigned long base, int crate) throw (string) :
  CModule32(base, ModuleSize, crate)
{

  uint32_t ModuleInfo = getModuleInfo();

  int     id         = ((ModuleInfo >> IDShift) & IDMask);

  if(id != 0x3820) {
    ThrowString("CSIS3820::CSIS3820",
	      "Base address does not point to an SIS3820 module");
  }
  Reset();

  // Setup the module to default mode:

  

}

/*!
   Returns the revision level of the module as a floating point value of the
   form major.minor.
*/
double
CSIS3820::getRevision() const
{
  uint32_t id = getModuleInfo();
  
  int minor = id & REVMask;
  int major = ((id >> MAJORShift) && REVMask);
  char versionstring[100];
  sprintf(versionstring, "%d.%d", major, minor);
  return atof(versionstring);
}

/*!
   Return the module information (id and firmware) register:
*/
uint32_t
CSIS3820::getModuleInfo() const
{
  return peek(IdAndFirmware);
}
/*!
    Turn on the user led.  This is done by turning on the 
    bottom bit in the CSR.
*/
void
CSIS3820::LightOn() const
{
  setCSR(LEDOn);
}
/*!
   Turn off the user led.  This is done by turning on the
   16'th bit of the CSR (numbered from 0).
*/
void
CSIS3820::LightOff() const
{
  setCSR(LEDOff);
}

/*!
   Return the current contents of the CSR.  This 
   is required if nothing else to make my test suite's run
   but may be of interest to users as well. It's harmless since
   I'm only giving read access.
*/
uint32_t
CSIS3820::getCsr() const 
{
  return peek(CSR);
}
/*!
   Utility function to return the value of the Acquisition 
   mode register.
*/
uint32_t
CSIS3820::getAcqMode() const
{
  return peek(ACQMode);
}
/*!
  Reset the module.  Module goes back to its power up state
  but does not take the time to do self tests (this is instantaneous).

*/
void
CSIS3820::Reset() const
{
  Key(RESET);
}

/*!
    Set the operating mode to whatever the user requests.
    At present the following mode are supported:
    - CSIS3820::OperatingMode::LatchingScaler
        Count freely as indicated by the front panel inputs.
	Latch signals from the front panel will transfer counts
        to the shadow registers from where they can be read.
        Scaler registers can also be read while counting (non
	latched mode).
   All other modes will cause an exception to be thrown.

   \param mode  (CSIS3820::OperatingMode  [in]):
       The desired operating mode.  See the text above for
       restrictions.

   \throws string
       If the operating mode is illegal.
*/
void
CSIS3820::setOperatingMode(CSIS3820::OperatingMode mode)  const
  throw (string)
{
  const char* pLeader("CSIS3820::setOperatingMode");
  switch (mode) {
  case LatchingScaler:
    {
      unsigned long  
	CurrentMode = getAcqMode();
      CurrentMode   = (CurrentMode & ~getOperatingMode()) | mode;
      setAcqMode(CurrentMode);
    }
    break;
  case MultiChannelScaler:
    ThrowString(pLeader, 
		"MultiChannel mode  unsupported at this time.");
  case HistogrammingScaler:
    ThrowString(pLeader,
		"Histogramming mode unsupported at this time");
  case RamTestScaler:
    ThrowString(pLeader,
		"Ramtest mode unsupported at this time");
    
  default:
    ThrowString(pLeader,
		"Unknown operating mode requested");
  }
}
/*!
  Return the current operating mode.
*/
CSIS3820::OperatingMode
CSIS3820::getOperatingMode() const
{
  unsigned long CurrentMode = getAcqMode();
  CurrentMode &= OperatingModeMask;
  return static_cast<OperatingMode>(CurrentMode);
}

/*!
   Set the input mode of the module.
   The input mode defines which inputs do what on the module.
   Since the module is run by an FPGA, these are highly programmable.
   The following modes are supported (undescribed inputs do 
   nothing):
   - CSIS3820::NoInputs - The Inputs do nothing.
   - CSIS3820::InputLatchInhibitLatch  
   \verbatim  
       Input1 is an external latch signal.  
       Input 4 inhibits this latch.
   \endverbatim
   - CSIS3820::InputLatchInhibitAllAdnLatch
   \verbatim
      Input1 is an External latch signal.
      Input3 inhibits the counting on all channels.
      Input4 inhibits the external latch.
    \endverbatim
   - CSIS3820::InputLatchInhibitAll
    \verbatim
      Input1 is an external latch signal.
      Input4 inhibits all scaler channels.
    \endverbatim
   - CSIS3820::InhibitGroups
    \verbatim
      Input1 Inhibits counting in channels 1-8
      Input2 Inhibits counting in channels 9-16
      Input3 Inhibits counting in channels 17-24
      Input4 Inhibits counting in channels 25-32
    \endverbatim
    - CSIS3820::InputLNEHiscal
      \verbatim
         Input 1 External LNE
         Input 2 Exterinal HISCAL_STart (time reste/start pulse).
                 see the manual
      \endverbatim
    - CSIS3820::InputLNEInhAllExtClear (requres at lest FW 1.08)
      \verbatim
         Input 1 External LNE
         Input 2 Inhibit counting in all channels.
         Input 3 clear all counters.
         Input 4 external user bit 1.
      \endverbatim
    - InputReserved{5,6} not unimplemented in the module.

    \param mode  (CSIS3820::InputMode [in])
       desired new input mode.

    \throws string
      If an invalid input mode is specified.
*/
void
CSIS3820::setInputMode(CSIS3820::InputMode mode) const throw (string)
{
  const char* pLeader("CSIS3820::setInputMode");
  switch (mode) {
    // Valid modes:
  case NoInputs:
  case InputLatchInhibitLatch:
  case InputLatchInhibitAllAndLatch:
  case InputLatchInhibitAll:
  case InhibitGroups:
  case InputLNEHiscal:
    {
      unsigned long current = getAcqMode();
      current = (current & ~getInputMode()) | mode;
      setAcqMode(current);
    }
    break;
  case InputLNEInhAllExtClear:
    if (getRevision() >= 1.08) {
      unsigned long current = getAcqMode();
      current = (current & ~getInputMode()) | mode;
      setAcqMode(current);
    }
    else {
      ThrowString(pLeader,"InputLNEInhAllExtClear requires FW rewv 1.5 and above.");
    }
    break;
    // unimplemented modes.
  case InputReserved7:
    
    ThrowString(pLeader,
		"Unimplemented input mode requested (InputReserved*)");
    // Unknown mode:
  default:
    ThrowString(pLeader,
		"Unrecognized input mode requested");
  }
}

/*!
   Return the current input mode.
   We are helped by the fact that the input mode enum is set
   up so that the values are the values stored in the bitfield,
   already appropriately positioned:
   \return CSIS3820::InputMode
      The current input mode.
*/
CSIS3820::InputMode
CSIS3820::getInputMode() const
{
  unsigned long mode = getAcqMode();
  mode &= InputModeMask;
  return static_cast<InputMode>(mode);
}

/*!
    Set the source of the latch request.
    The latch request will copy all scalers to the shadow
    registers within 5ns, where they will be stable and can be read.
    The modes defined by the module are:
    - LatchVMEOnly - Latching can only be done via a VME request.
    - LatchFP      - Latching is done by VME request or by   
                     a pulse on the front panel.
    - Latch10Mhz   - Latches are done by the VME or whenever
                     the scaler's 10Mhz clock ticks.
    - LatchChannelN - \em (unsupported) Latch is done by
                     counts in a selected channel
    - LatchPresetN  - \em (unsupported) Latch is done when the
                      value of a counter reaches the present value.
    - LatchReserved{5,6,7} - \em (unsupported) modes the hardware
                    does not suport.

    \param mode (CSIS3820::CLNESource [in]):
       The latch mode to be selected.

    \throw string
       if the latch mode is unsupported.

*/
void
CSIS3820::setLatchSource(LNESource mode) const
  throw (string)
{

  const char* pLeader("CSIS3820::SetLatchSource");

  switch (mode) {
    // supported modes.
  case LatchVMEOnly:
  case LatchFP:
  case Latch10Mhz:

    {
      unsigned long acqmode = getAcqMode();
      acqmode    = (acqmode & ~getLatchSource()) | mode;
      setAcqMode(acqmode);
    }
    break;

    // unsupported modes.

  case LatchChannelN:
  case LatchPresetN:
  case LatchReserved5:
  case LatchReserved6:
  case LatchReserved7:
    ThrowString(pLeader,
		"Unsupported latch mode.");

    // unrecognized mode.

  default:
    ThrowString(pLeader,
		"Unrecognized latch mode ");
  }
  
}
/*!
   Return the current value of the latching mode.
*/
CSIS3820::LNESource
CSIS3820::getLatchSource() const
{
  unsigned long mode = getAcqMode();
  return static_cast<LNESource>(mode & LatchModeMask);
}

/*!
  Arm the module. To count, the module must be armed and
  enabled.

 */
void
CSIS3820::Arm() const
{
  Key(ARM);
}

/*!
  Determine if the module is armed.
*/
bool
CSIS3820::isArmed() const
{
  unsigned int csr = getCsr();
  return ((csr & ISARMED) != 0);

}
/*!
   Enable the scaler.
*/
void
CSIS3820::Enable() const
{
  Key(ENABLE);

}
/*!
  Disable the scaler.
*/
void 
CSIS3820::Disable() const
{
  Key(DISABLE);
}
/*!
   Return true if the module is enabled.
   note that in order to deal with future developments 
   supporting MCS mode, Enabled is defined as either of the
   SCALERENABLED and MCSENABLED bits set in the csr.
*/
bool
CSIS3820::isEnabled() const
{
  unsigned int csr = getCsr();
  return ((csr & (SCALERENABLED | MCSENABLED)) != 0);
}

/*!
    Select the input of channel 1 to be the reference pulser.
    The reference pulser is a 50Mhz crystal oscillator. Note
    that 25Mhz test mode has priority over this setting.
*/
void
CSIS3820::EnableReferencePulser() const
{
  setCSR(ENABLEREFPULSER);
}
/*!
   Turn off the reference pulser going into channel 1. After this,
   channel 1 is a normal scaler that counts whenever it sees an
   input pulse on the front panel.
*/
void 
CSIS3820::DisableReferencePulser() const
{
  setCSR(DISABLEREFPULSER);
}
/*!
   Return true if the reference pulser is currently enabled
  into channel 1 of the module
  \return bool
  - true if the reference pulser is on.
  - false if not.
*/
bool
CSIS3820::isReferencePulserEnabled() const
{
  unsigned int csr = getCsr();
  return ((csr & REFPULSERON) != 0);
}


/*!
  Enable the 25Mhz test pulser to clock all scaler channels.
  This setting overrides the reference pulser setting if both 
  are set.
*/
void
CSIS3820::EnableTestCounter() const
{
  setCSR(ENABLETESTPULSER);
}
/*!
  Disable the 25Mhz test pulser.
*/
void 
CSIS3820::DisableTestCounter() const
{
  setCSR(DISABLETESTPULSER);
}
/*!
  True if the 25Mhz test pulser is enabled into all the channels.
  \return bool
  - true the 25Mhz test pulser is on.
  - false the 25Mhz test pulser is off.
*/
bool
CSIS3820::isTestCounterOn() const
{
  unsigned int csr = getCsr();
  return ((csr & TESTPULSERON) == TESTPULSERON); // not simple bits.
}
/*!
  Clear all the channels via write to the clear key register.

 */
void
CSIS3820::ClearChannels() const
{
  Key(CLEAR);
}
/*!
  Disable the auto clear on the latch function. After this 
  is called, the scaler counters will not clear when latched but
  will continue to count.
  
 */
void 
CSIS3820::DisableClearOnLatch() const
{
  unsigned long mode = getAcqMode();
  mode |= NONCLEARINGMODE;
  setAcqMode(mode);
}
/*!
   Enable the autoclear on latch function.
  After this is called scaler counters will be cleare when
  latched...and then continue to count.
*/
void
CSIS3820::EnableClearOnLatch() const
{
  unsigned long mode = getAcqMode();
  mode &= ~NONCLEARINGMODE;
  setAcqMode(mode);
}

/*!
  Read a channel of the scaler.  Note that this reads the live
  scaler channels, not the latched scalers.  To read latched
  scaler values, use ReadLatchedChannel

  \param num (unsigned int [in]):
     The channel number in the range [0,ChannelCount).
  
  \return unsigned long 
     The instantaneous channel value.

   \note
     Reading a channel also causes a latch of the counters
     into the shadow registers.  In other words it's synchronous
     (within 5ns) to read one channel and then read the rest
     with ReadLatchedChannel e.g.

*/
uint32_t
CSIS3820::ReadChannel(unsigned int num) const
{

  ValidChannel(num, "CSIS3820::ReadChannel");
  return peek(ChannelOffset(COUNTERS, num));

}

/*!
   Reads all scaler counters.  See ReadChannel for more information
   about what this means and when it might be inappropriate.
   \param buffer  (unsigned long* [out]):
      Pointer to a 32 long buffer to hold the scaler values.
*/
void
CSIS3820::ReadAllChannels(uint32_t* buffer) const
{
  for(unsigned int i =0; i < ChannelCount; i++) {
    *buffer++ = ReadChannel(i);
  }
}
/**
 * ReadHighBits
 *
 *   Reads the register that contains the high 16 bits of channels
 *   1 and 17 which are  48 channel scalers.
 *
 * @return uint32_t
 */
uint32_t
CSIS3820::ReadHighBits()
{
  return peek(HIGHBITS);
}
/*!
   Latch the current counters into the shadow registers.
   Once this has been done, the channels can be read via
   ReadLatchedChannel and ReadAllLatchedChannesl

*/
void
CSIS3820::Latch() const
{
  Key(LATCH);
}
/*!
   Read a specific channel from the latched channel 
   (shadow) registers.  In latch mode, the LNE or Latch operation
   (either front panel or Latch function) transfers the scaler
   counter values to shadow registers where they remain stable
   until the next latch function.

   The transfer takes 5ns.  This function reads a channel
   of these registers. 
   \param num (unsigned int [in]):
       The number of the channel to read, must be in the range
       [0, 32).

   \return unsigned long
     The value of the latched register for that channel.
     
   \throw string
     If the channel value is out of range.

*/
uint32_t
CSIS3820:: ReadLatchedChannel(unsigned int num) const
{
  ValidChannel( num, "CSIS3820::ReadLatchedChannel");
  return peek(ChannelOffset(SHADOWCOUNTERS,num));

}
/*!
    Read all latched channels.  See ReadLatchedChannel and Latch
    for more information, however this function reads the values
    of all shadow registers into a user buffer.
    \param buffer (unsigned long* [out]):
       user buffer of ChannelCount longs to hold the channel values.

*/
void
CSIS3820:: ReadAllLatchedChannels(uint32_t* buffer) const
{
  for(int i =0; i < ChannelCount; i++) {
    *buffer++ = ReadLatchedChannel(i);
  }
}
/*!
   This convenience function does a Latch and then a call to
   ReadAllLatchedChannels.

   \param buffer (unsigned long * [out]):
     Pointer to user buffer to hold the scalers.
*/
void
CSIS3820::LatchAndRead(uint32_t* buffer) const
{
  Latch();
  ReadAllLatchedChannels(buffer);
}
///////////////////////////// Utilities ////////////////////////

/*!
   Utility function to set a new value for the acquisition mode
   register:
*/
void
CSIS3820::setAcqMode(unsigned long value) const
{
  poke(value, ACQMode);
}

/*!
   Write a value to the CSR:
*/
void
CSIS3820::setCSR(unsigned int long data) const
{
  poke(data, CSR);
}
/*!
   Throws a string exception if the channel number passed in is not
   a valid channel.
   \param chan (unsigned int [in]):
      The number of the channel to check.
   \param pWhere (const char* pWhere):
      The where context for the string.
   \throw string
      If the channel is out of range.
*/
void
CSIS3820::ValidChannel(unsigned int chan, 
		       const char *pWhere) throw (string)
{
  if(chan >= ChannelCount) {
    char msg[100];
    sprintf(msg, "Channel %d is out of range must be in [0,32)",
	    chan);
    ThrowString(pWhere, msg);
  }
}
/*!
   Returns the offset into the module associated with a channel.
   \param base  (unsigned long):
      Base of the register page, usually either COUNTERS or
      SHADOWCOUNTERS (in future may be in the fifo/ram).
   \param chan (unsigned int):
      Channel number to compute the offset for.
   \return unsigned long
     The offset at which the channel can be read.
*/
uint32_t
CSIS3820:: ChannelOffset(uint32_t base, 
			 uint32_t  chan)
{
  return base + chan*sizeof(uint32_t);
}
