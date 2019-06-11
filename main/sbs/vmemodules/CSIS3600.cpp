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

#include "CSIS3600.h"

#include <VmeModule.h>

using namespace std;


// Structure of the module registers (offsets).

#define CSR              0x000	// Control status register offset.
#define IDIRQ            0x004	// Identification and IRQ register.
#define FCLEARWIN        0x008	// Fast clear window.
#define OPFREQ           0x00c	// Ouput pulse frequency generator.
#define WRITEFIFO        0x010	// Write to fifo in test mode.
#define CLEARFIFO        0x020	// KEY: Clear the FIFO.
#define NEXTCLOCK        0x024	// KEY: VME Next clock.
#define ENABLENEXTCLOCK  0x028	// KEY: Enable next clock logic.
#define DISABLENEXTCLOCK 0x02c	// KEY: Disable next clock logic.
#define ENABLEFCLEAR     0x050	// KEY: Enable fast clear.
#define DISABLEFCLEAR    0x054	// KEY: Disable fast clear.
#define RESET            0x060	// KEY: Reset the module.
#define PULSEOUT         0x068	// KEY: Generate an output pulse.
#define CBLTSETUP        0x080  // CBLT setup register.
#define FIFO             0x100  // FIFO (data).

// The CSR is both a status and a control register.  The
// bit structures below (assuming that bits are defined low to high'
//e the structure of these registers..

/*!
   Module status register (read at 0).
*/
struct StatusRegister {
  unsigned LedOn:1;		//!< 1 if user led is turned on.
  unsigned TestingFIFO:1;	//!< 1 if fifo test enabled.
  unsigned OutputMode:2;	//!< Current output mode.


  unsigned OutputPulseEnabled:1; //!< 1 If output pulses are enabled.
  unsigned isPipelined:1;	//!< 1 if pipeline mode enabled.
  unsigned isBroadcastOn:1;	//!< 1 If broadcast mode enabled.
  unsigned isBCastController:1;	//!< 1 if this module is bcast controller.

  unsigned FIFOEmpty:1;		//!< FIFO is empty.
  unsigned FIFOAlmostEmpty:1;	//!< FIFO almost empty.
  unsigned FIFOHalfFull:1;	//!< FIFO 1/2 full.
  unsigned FIFOAlmostFull:1;	//!< FIFO is almost full

  unsigned FIFOFull:1;		//!< FIFO is full.
  unsigned Unused1:1;		//!< Unused bit
  unsigned FastClearEnabled:1;	//!< 1 if fast clear is enabled.
  unsigned isEnableNext:1;	//!< 1 if enable next set.

  unsigned isExternalNext:1;	//!< 1 if external next enabled.
  unsigned isExternalClear:1;	//!< 1 if external clear enabled.
  unsigned isLatchGate:1;	//!< status of latch gate.
  unsigned isCoincidenceMode:1;	//!< 1 if module in coincidence mode.

  unsigned isEnableVMEIrq0:1;	//!< Status of the  various
  unsigned isEnableVMEIrq1:1;	//!< vme interrupt source enables.
  unsigned isEnableVMEIrq2:1;
  unsigned isEnableVMEIrq3:1;

  unsigned Unused2:1;		//!< Unused.
  unsigned Unused3:1;		//!< Unused.
  unsigned InternalVMEIRQ:1;	//!< ???
  unsigned VMEIRQ:1;		//!< VME interrupt requested.

  unsigned VMEIRQ0:1;		//!< IRQ for CIP.
  unsigned VMEIRQ1:1;		//!< IRQ for FIFO almost empty.
  unsigned VMEIRQ2:1;		//!< IRQ for FIFO 1/2 full.
  unsigned VMEIRQ3:1;		//!< IRQ FOR FIFO FULL.
};

/*!
  Module control register (read at 0).
*/
struct ControlRegister {
  unsigned SetLEDOn:1;		//!< Turn on the LED.
  unsigned SetFIFOTest:1;	//!< Turn on FIFO test mode.
  unsigned SetModeBit0:1;	//!< Turn on output mode 0 bit
  unsigned SetModeBit1:1;	//!< Turn on output mode 1 bit.

  unsigned EnableOutputPulses:1; //!< Enable output pulser.
  unsigned EnablePipeline:1;	//!< Enable pipeline mode.
  unsigned EnableBroadcast:1;	//!< Enable broadcast mode.
  unsigned EnableBcastController:1; //!< Enable this as bcast controller

  unsigned ClearLEDOn:1;	//!< Turn off the user led.
  unsigned ClearFIFOTest:1;	//!< Turn off fifo test mode.
  unsigned ClearModeBit0:1;	//!< Turn off output mode 0 bit
  unsigned ClearModeBit1:1;	//!< Turn off output mode bit 1.

  unsigned DisableOutputPulses:1; //!< Disable the output pulses
  unsigned DisablePipeline:1;	//!< Disable pipeline mode.
  unsigned DisableBroadcast:1;	//!< Disable broadcast mode.
  unsigned DisableBcastController:1; //!< Disable this as bcast controller.

  unsigned EnableExternalNext:1; //!< Enable external latch signal.
  unsigned EnableExternalClear:1; //!< Enable external clear.
  unsigned SetLatchGate:1;	//!< Set latch gate.
  unsigned SetCoincidenceMode:1; //!< Run in coincidence mode.

  unsigned EnableIRQ0:1;	//!< Enable the irq sources.
  unsigned EnableIRQ1:1;
  unsigned EnableIRQ2:1;
  unsigned EnableIRQ3:1;

  unsigned DisableExternalNext:1; //!< Disable the external next latch
  unsigned DisableExternalClear:1; //!< Disable the external clear.
  unsigned ClearLatchGate:1;	//!< Clear the latch gate.
  unsigned ClearCoincidenceMode:1; //!< Run in latching mode.

  unsigned DisableIRQ0:1;	//!< Disable the interrupt sources.
  unsigned DisableIRQ1:1;
  unsigned DisableIRQ2:1;
  unsigned DisableIRQ3:1;

};

/*!
  The structure below defines the module identification and
  interrupt control register bits.
 */
struct IdIrq {
  unsigned IRQVector:8;		//!< Interrupt Vector.

  unsigned IPL:3;		//!< Interrupt priority level.
  unsigned IRQEnable:1;		//!< Interrupt enabled.

  unsigned Version:4;		//!< Module firmware version

  unsigned ModuleId:16;		//!< Module id.

};


//
//  The following union definitions make it easier to deal with
//  the bit-wise/long wise duality of registers:
//
union IdRegister {		// ID IRQ register...
  IdIrq         bits;
  uint32_t l;
};


union StatReg {
  StatusRegister  bits;
  uint32_t   l;
};

union CtlReg {
  ControlRegister bits;
  uint32_t   l;
};

// The register unions above have a structure that is identical
// in order to allow us to do the following little macro games:

#define SetBit(reg, field, value) reg.bits.field  = value
#define GetBit(reg, field)        reg.bits.field
#define Zero(reg)                 reg.l           = 0
#define Long(reg)                 reg.l


static const uint32_t ModuleLength(0x200); //!< size of the module.

/*!
   Construct a module.  The module consists of a VME space definition
   that is held by m_Module.  Note that use of m_Module ensures
   portability between SBS Bit3 and Wiener bus bridge modules.
   \param base (uint32_t [in]):
      VME Base address of xthe module.
   \param crate (int [in default = 0])
      Selects the VME crate holding the module.
*/
CSIS3600::CSIS3600(uint32_t base, int crate) :
  CModule32(base, ModuleLength, crate)
{
  IdRegister id;


  uint32_t csr = peek(CSR); // Sometimes needed for good->bad
				// module construction to shake up
				// the bus bits a bit.

  Long(id) = peek(IDIRQ);

  if(GetBit(id,ModuleId) != 0x3600) {
    ThrowString("CSIS3600::CSIS3600",
		"No 3600 at this base address");
  }
  Reset();			// Reset the module.
}
/*!
  Destructor.
*/
CSIS3600::~CSIS3600()
{
}
/*!
   Light the led.  Note that if the output mode is 2 (not
   yet implemented), this will also assert the user output).

*/
void
CSIS3600::LightLed() const
{

  CtlReg csr;

  Zero(csr);
  SetBit(csr, SetLEDOn, 1);

  poke(Long(csr), CSR);

  
}
/*!
  Turn off the led.  Note that if the output mode is 2
  (not yet implemented), this will also turn off the user output).
*/
void
CSIS3600::ClearLed() const
{

  CtlReg csr;

  Zero(csr);
  SetBit(csr, ClearLEDOn, 1);


  poke(Long(csr), CSR);

  
}
/*!
   Check to see if the led (user output in the as yet unimplemented
   output mode 2) is lit.
   \return bool
   - true  - the LED is lit.
   - false - the LED is not lit. 
*/
bool
CSIS3600::isLedLit() const
{
  StatReg sr;

  Long(sr) = peek(CSR);
  return (GetBit(sr,LedOn) == 1);
}
/*!
   Tell the module to run in latch mode.  Note that
   latch (or strobed mode as the docs call it) is mutually
   incompatible with coincidence mode (see SetCoincidenceMode)

*/
void
CSIS3600::SetLatchMode() const 
{

  CtlReg cr;

  Zero(cr);
  SetBit(cr,ClearCoincidenceMode, 1);

  poke(Long(cr), CSR);
}
/*!
   Tell the module  to run in coincidence mode.  Note that
   coincidence mode is not compatible with strobed (Latched) mode.
*/
void
CSIS3600::SetCoincidenceMode() const
{

  CtlReg cr;

  Zero(cr);
  SetBit(cr,SetCoincidenceMode,1);

  poke(Long(cr), CSR);
}
/*!
   Returns true if the module is in latch mode.
   This will be the case if the coincidence mode bit of the
   status register is not set.
*/
bool
CSIS3600::isLatchMode() const
{
  StatReg sr;

  Long(sr) = peek(CSR);

  return (GetBit(sr,isCoincidenceMode) == 0);
}
/*!
  Reset the module back to power up setttings.
*/
void
CSIS3600::Reset()
{
  Key(RESET);
  m_nFastClearWindow = 220;	// Default initial fast clear window.
}
/*!
    Enable the external next event input.
    This is required to accept an external latch or coinc. gate.
*/
void
CSIS3600::EnableExternalNext() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr, EnableExternalNext, 1);
  poke(Long(cr), CSR);
}
/*!
   Disable the external next event input.
   (Not recommended).
*/   
void 
CSIS3600::DisableExternalNext() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr, DisableExternalNext, 1);
  poke(Long(cr), CSR);
}
/*!
    Return true if the external next event input is
    enabled.
*/

bool
CSIS3600::ExternalNextEnabled() const
{
  StatReg sr;
  Long(sr) = peek(CSR);
  return (GetBit(sr, isExternalNext) == 1);
}
/*!
  Enable the external clear.  Once enabled, the external clear
  can be used as a fast clear mechanism to prevent an event
  from making it into the FIFO.  Note this is not the same as 
  a module clear that must be done from the VME...however there is
  an external RESET input that does that (I don't think it requires
  an enable??
*/
void
CSIS3600::EnableExternalClear() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr,EnableExternalClear ,1);
  poke(Long(cr), CSR);
    
}
/*!
  Disable the external clear. 
*/
void 
CSIS3600:: DisableExternalClear() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr, DisableExternalClear, 1);
  poke(Long(cr), CSR);
}
/*!
  True if the external clear is enabled.
*/
bool
CSIS3600::ExternalClearEnabled() const
{
  StatReg sr;
  Long(sr) = peek(CSR);
  return (GetBit(sr, isExternalClear) == 1);
}
/*!
   Set the fast clear window.  The fast clear window register
   determines the time period following the initial falling edge
   of the strobe that a fast clear can be accepted. If accepted,
   the fast clear will abort the transmission of the latched data
   into the event fifo.
   \param ns (int [in]):
      Width of the fast clear interval in nanoseconds.  Note that
      the actual window must be in the range [220,25720] ns and
      that the granularity of the register is 100ns steps...
      with an offset of 20 (e.g. 220, 320, 420...).
      The value provided, once range checked, will be converted
      to the closest legal value.
    \throw string
       If the value fails range check a string exception is thrown.
*/
void
CSIS3600::SetFastClearWindow(int ns)
{
  m_nFastClearWindow = ns;
  ns += 50;			// This ensures rounding.
  uint32_t reg =ns;
  reg = (reg - 120)/100 -1;
  if(reg > 0xff) {
    ThrowString("CSIS3600::SetFastClearWidow",
		"Fast clear window range is [220, 25720]ns");
  }

  poke(reg, FCLEARWIN);
}
/*!
   Get the value of the fast clear window.  The fast clear window
   register is an 8 bit register the relationship:
   \verbatim
   
   ns = (r + 1) *100 + 120

   \endverbatim
   Describes the relationship between the register value (r) and
   the fast clear window in ns.  
   \return int
    The number of \em nanoseconds in the fast clear window.  

*/
int
CSIS3600::GetFastClearWindow() const
{
  return m_nFastClearWindow;
}
/*!
  Start a latch cycle.  The coincidence register includes the
  ability for the software to gate signals.  This is done by
  invoking this function to start the coincidence interval in 
  which the latch is taken and then invoking EndLatch to end 
  the latch cycle signal.  At the end of the latch signal, data 
  are transferred to the module fifo. 

*/
void
CSIS3600::StartLatch() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr, SetLatchGate,1);

  poke(Long(cr), CSR);
}
/*!
  Stop a latch cycle.  For more information about software
  latch cycles, see StartLatch above.
*/
void
CSIS3600::EndLatch() const
{
  CtlReg cr;
  Zero(cr);
  SetBit(cr, ClearLatchGate,1);

  poke(Long(cr), CSR);
}
/*!
  Returns true if the FIFO is not empty.
  FIFO not empty is determined by not seeing bit FIFOEmpty in
  the status register.
*/
bool
CSIS3600::DataReady()  const
{
  StatReg  sr;

  Long(sr) = peek(CSR);

  return ((GetBit(sr, FIFOEmpty)) == 0);
}
/*!
   Enable the module to receive data (strobes).
*/
void
CSIS3600::Enable() const
{
  Key(ENABLENEXTCLOCK);
}
/*!
  Disable the module to receive strobes clocks.
*/
void
CSIS3600::Disable() const
{
  Key(DISABLENEXTCLOCK);
}
/*!
  Clock the module.
*/
void
CSIS3600::Clock() const
{
  Key(NEXTCLOCK);
}
/*!
   Clear the FIFO.
*/
void
CSIS3600::ClearData()  const
{
  Key(CLEARFIFO);
}
/*!
   Read an 'event' from the FIFO.  Note that an event consists
   of a single longword of data.  If no data are available this
   fact is thrown as an exception.  To prevent this from happening,
   the caller needs to check for events via  DataReady().

   \return uint32_t
      The longword latched by the least recently unread event.

   \throw string
      Exception describing what happened if the read was attempted
      without data ready.
*/
uint32_t
CSIS3600::Read() const 

{
  if(DataReady()) {
    return peek(FIFO);
  }
  else {
    ThrowString("CSIS3600::Read",
		"No data present");
  }
  return 0;             // Really we threw already.
}
/*!
   Read a block of events from the FIFO.  Events are read into
   the user buffer until either:
   - The buffer space is exhausted,
   - The FIFO is empty.
   Thus, a typical usage might be:
   \code
   uint32_t buffer[100];
   CSIS3600* pLatch = &(somexisting_latch);
   while(pLatch->DataReady()) {
      uint32_t nread = pLatch->Read(buffer, 100);
      DoSomethingWithData(buffer, nread);
   }
   \endcode

   \param pBuffer (void* [out]):
      Pointer to a buffer into which the data will be read.
   \param nLongs (int [in]):
      Number of longwords in pBuffer.  The storage pointed to
      by pBuffer must be at lest nLongs longwords of valid
      data space.
   \return
      uint32_t
        Number of long words read.  This may be less than
	nLongs or even 0 if the number of events in the FIFO
	is fewer than nLongs (or the FIFO is empty e.g.).
*/
size_t
CSIS3600::Read(void* pBuffer, 
	       int nLongs) const
{
  uint32_t* p = (uint32_t*)pBuffer;
  uint32_t   n = 0;

  // This code assumes that a throw/catch is relatively expensive.

  while(DataReady() && nLongs) {
    *p++ = Read();
    n++;
    nLongs--;
  }
  return n;
}
