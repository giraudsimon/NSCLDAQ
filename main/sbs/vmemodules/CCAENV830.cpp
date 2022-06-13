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
#include "CCAENV830.h"
#include "CCAENV820Registers.h"
#include <CVMEInterface.h>


#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


using namespace std;



// The 'macros' below are used to do structure like offsets
// using the register structure through VmeModule:
//

#define GetShort(field) (m_pModule->peekw(SOffset(CAENV830Registers,field)))
#define GetLong(field)  (m_pModule->peekl(LOffset(CAENV830Registers,field)))
#define PutShort(field,value) (m_pModule->pokew((value),  \
                                  SOffset(CAENV830Registers,field)))
#define PutLong(field,value)  (m_pModule->pokel((value), \
                                  LOffset(CAENV830Registers,field)))
#define GetProm(m,field) (m.peekw(SOffset(CAENV830ROM,field)))
 
static inline void ThrowIntString(const char* formatstring, int value) 
  
{
  char msg[strlen(formatstring) + 100];
  sprintf(msg, formatstring, value);
  throw string(msg);
}
static inline void ThrowIfBadChannel(const char* pFunction,
				   int nChannel) 
{
  const char* pFmtString=
    "CCAENV830::%s Invalid channel must be in [0,31] %d";
  if( (nChannel < 0) || (nChannel > 31)) {
    char msg[100 + strlen(pFunction) + strlen(pFmtString)];
    sprintf(msg, pFmtString, pFunction, nChannel);
    throw string(msg);
  }
}
/*!
    Make a new CCAENV830 module.  This involves:
    - Mapping the proposed module.
    - Ensuring the module is a V820 or V830.
    - Initializing the module to a known initial state.
   \param slot (in):
      The slot number the adc is in.  If the module is 
      run in Geo address mode, this slot is used to open
      the initial map.  Otherwise it is programmed into the
      module Geo register.
   \param crate (in):
      The VME crate the module lives in.  This defaults to zero.
   \param geo   (in):
      A flag that defaults to true.. .when true, the module 
      will be used in geographical address mode for initialization
      purposes.
   \param base (in):
      If geo is false, this must be the module base address as set
      in it's rotary switches.

*/
CCAENV830::CCAENV830(int slot,
		     int crate,
		     bool geo,
		     uint32_t base) :
  m_nCrate(crate),
  m_nSlot(slot),
  m_nBase(base),
  m_pModule(0),
  m_fGeo(geo)
{
  try {
    MapModule();		// Access the module.
    InitModule();		// Setup initial module state.
  } catch (...) {
    delete m_pModule;		// Prevent leakage...
    throw;			// But let the exception propagate.
  }

}
/*!
   Destroy a VME module:
*/
CCAENV830::~CCAENV830()
{
  UnmapModule();		// Destroy mapping info.
}

// Functions that operate on the module:



// Utility functions:

/*!
   Enable an individual channel of the module:
   \param nChannel (in):
      Number of channel to turn on [0-31].
*/
void
CCAENV830::Enable(int nChannel) 
{
  ThrowIfBadChannel("Enable", nChannel);
  uint32_t nMask  = GetLong(Enables);
  nMask     |= 1 << nChannel;
  SetEnableMask(nMask);
}

/*!  
  Disable an individual channel of the module:
  \param nChannel (in):
    Number of channel to turn on [0-31]
 */
void
CCAENV830::Disable(int nChannel) 
{
  ThrowIfBadChannel("Disable", nChannel);
  uint32_t nMask = GetLong(Enables);
  nMask    &= ~(1 << nChannel); 
  SetEnableMask(nMask);
}

/*!
  Set the module enable register.  This sets the state of all
  the channel enables according to the mask parameter.
  \param nMask (in):
    Mask of channel enables.
*/
void
CCAENV830::SetEnableMask(uint32_t nMask)
{
  PutLong(Enables, nMask);
  ComputeEventSize();		// Update the event size.
}

/*!
  Set the dwell time.  In periodic trigger mode, this is the
  length of time the module counts between latching triggers.
  \param n400ns [in]:
     The number of 400ns clock ticks to count between
     periodic triggers.

  \note  If the module is not in periodic trigger mode,
         the dwell time is ignored.  It is not an error to set
	 this value if that is the case however since we don't want
	 to impose a programming order on the client.
*/
void
CCAENV830::SetDwellTime(int n400ns) 
{
  PutLong(DwellTime, n400ns);
}

/*!
   Set the trigger mode:
   \param mode (in):
      The desired trigger mode.  This can be one of:
      - CCAENV830::TriggerMode::Disabled 
                   The module is never triggered.
      - CCAENV830::TriggerMode::Random   
                    The module is triggered via VME command or
                   by front panel input.
      - CCAENV830::TriggerMode::Periodic - The module is periodically triggerred as
                   determined by the dwell time (see SetDwellTime).
*/
void
CCAENV830::SetTriggerMode(CCAENV830::TriggerMode mode) 
{
  uint16_t csr = GetShort(Csr);
  csr      &= ~CSRAcqMode;
  switch (mode) {
  case Disabled:
    csr |= CSRAcqDisable;
    break;
  case Random:
    csr |= CSRAcqRandom;
    break;
  case Periodic:
    csr |= CSRAcqPeriodic;
    break;
  default:
    ThrowIntString("CCAENV830::SetTriggerMode invalid trigger mode %d",
		mode);
		
  }
  PutShort(Csr, csr);
}


/*!
  Set the scaler readout format to wide mode.  This drops channel
  labelling information off the event... it then becomes the 
  user's responsibility to know which channel is which in the
  otput stream.  Use with caution, and only if 26 bits of scaler
  information is not sufficient.
  
 */
void CCAENV830::SetWide()
{
  PutShort(Bitclear1, CSRFormat);
}
/*!
  Set the scaler readout format to narrow mode.  This adds
  channel labelling information to each channel.  It is best used
  when you have enabled a noncontiguous set of channels and the
  counts range of 26 bits is adequate for your application. Note
  that 26 bits gives somewhat more than
   64Million counts before overflowing.
*/
void CCAENV830::SetNarrow()
{
  PutShort(Bitset1, CSRFormat);
}
/*!
   Enable the inclusion of a scaler header.  When this feature is
   turned on, the scaler will include header information that
   includes:
   - The geographical address of the scaler.
   - The number of words of scaler information present.
   - The trigger mode used.
   - The trigger number (event number) of the trigger.
*/
void
CCAENV830::EnableHeader()
{
  PutShort(Bitset1, CSRHeader);
  ComputeEventSize();		// Update event size.
}
/*!
  Disable the inclusion of a scaler header.  When this feature is
  turned off, only the bare scaler channels will be read.
*/
void
CCAENV830::DisableHeader()
{
  PutShort(Bitclear1, CSRHeader);
  ComputeEventSize();		// Update the event size.
}
/*!
   Controls the action of the front panel clear depending on the
   parameter:
   \param state (bool in):
   - true - The multievent buffer is cleared as well as the
            counters  by a front panel clear.
   - false- The counters are cleared but the multievent buffer
            is unmolested by a front panel clear.
*/
void
CCAENV830::FPClearsMEB(bool state)
{
  if(state) {
    PutShort(Bitset1, CSRFPClear);
  }
  else {
    PutShort(Bitclear1, CSRFPClear);
  }
}
/*!
  Enables automatic reset of the scalers on trigger.
  When this mode is set, each trigger clears the scalers after 
  latching them.

 */
void
CCAENV830::EnableAutoReset()
{
  PutShort(Bitset1, CSRAutoClr);
}
/*!
  Disables automatic reset of scalers on trigger.
  When this mode is set, triggers do not clear scalers after
  latching them.
*/
void
CCAENV830::DisableAutoReset()
{
  PutShort(Bitclear1, CSRAutoClr);
}
/*!
    Checks the data ready bit in the status register.

    \return bool :
    - true - if there is at least one complete event in the MEB
    - false - if there is not a complete event in the MEB

    \note  The definition of a complete event depends on the
    state of the header enabled bit.  If the header is enabled,
    as complete event is defined as a complete package of data
    associated with a trigger (header and all associated scaler
    channels.  If header is disabled, a complete event is defined
    as any number of data words in the MEB.  This definition makes
    the use of the module in multievent mode with headers disabled
    dangerous or useless (take your pick) in my opinion.
 */
bool
CCAENV830::isDataReady()
{
  uint16_t stat = GetShort(Status);
  return ((stat & STATUSDready) != 0);
}
/*!
  Checks the almost full bit in the module status register.
  This bit is normally used if the scaler is used in multi-event
  mode.
  
  \return bool : 
  - true Indicates that the MEB contains at least a number of 
          words equal to the value set with SetAlmostFullLevel.
  - false indicates there are less words (possibly 0!) than the
          value set with SetAlmostFullLevel
 */
bool
CCAENV830::isAlmostFull()
{
  uint16_t stat = GetShort(Status);
  return ((stat & STATUSAFull) != 0);
}
/*!
   Checks the full bit in the status register.  This bit is set
   when there is insufficient space in the MEB to allow the storage
   of the module's worst case event (33 longs). When the full
   bit is set, the module will no longer accept triggers until
   this condition is cleared either by reading data from the MEB
   or by clearing the MEB.

   \return bool : 
   - true   - The multievent buffer is full.
   - false  - The multievent buffer is not full.
*/
bool
CCAENV830::isFull()
{
  uint16_t stat = GetShort(Status);
  return ((stat & STATUSFull) != 0);
}
/*!
   Checks the global data ready bit.  This is bit is set when
   the front panel control connector is bussed with other modules
   and at least one module on the bus has it's Dready set.  In
   pure CAEN module systems, this can be used as a trigger
   condition.
  
   \return bool : 
   - true  - Global Dready is set.
   - false - Global Dready is not set.
*/
bool
CCAENV830::isGlobalDready()
{
  uint16_t stat = GetShort(Status);
  return ((stat & STATUSGDready) != 0);
}
/*!
  Checks the Global busy bit.  The use of the global busy bit
  depends on whether or not the front panel control connector is
  bussed with other modules.
  - If bussed: This function indicates that at least one module
    on the bus has busy set (either digitizing/transfering or
    MEB full).
  - If not bussed: This function indicates that this module is 
    busy (either transferring or MEB full).

 */
bool
CCAENV830::isGlobalBusy()
{
  uint16_t stat = GetShort(Status);
  return ((stat & STATUSGBusy) & 1);

}

/*!
  Resets the module.  A reset restores the power up configuration
  of the module with the exception of:
  - The address relocation and address enable registers.
  - The Multicast/CBLT address.
  - Any programmed Geographical address 
  - The dummy register (but who cares about that !).
*/
void
CCAENV830::Reset()
{
  PutShort(SwReset, 0);		// That I write is all that matters
}
/*!
   Peforms a software clear:
   - The counters are cleared.
   - The MEB is emptied.
   - The trigger counter is cleared.
   - The MEB Event number register is cleared.
*/
void
CCAENV830::Clear()
{
  PutShort(SwClear, 0);
}
/*!
   Provides a vme trigger fo the module  Note that for this to
   have any effect, the module must be in random trigger mode.
*/
void
CCAENV830::Trigger()
{
  PutShort(SwTrigger, 0);
}
/*!
   Sets the current value of the trigger counter. When the header
   is enabled, the trigger counter appears in each event header.

   \param n (int in):
     New value of the trigger counter.
*/
void
CCAENV830::SetTriggerCounter(int n)
{
  PutLong(TriggerCounter, n);
}
/*!
   Set the current almost full value.
   \param n (int in):
      The bottom 16 bits of this value are set as the almost full
      level.
*/
void
CCAENV830::SetAlmostFullLevel(int n)
{
  PutShort(AlmostFull, (n & 0xffff));
}
/*!
  Get the total number of events in the MEB. This only means
  something if the module is not busy!!!!
*/
int
CCAENV830::GetMEBEventCount()
{
  return GetShort(MEBCount);
}

/*!
   Map the vme module.  This involves:
   - Creating a CVmeModule object.  If the module is 
     to be geographically addressed, a geographical map is created
     first, long enough to program the address relocation, and
     then an extended map is created.
   - The following information is pulled from the module:
     #  Module type which must be 830 or 820
     #  Module Firmware revision
     #  Module serial numberr.

     The CAEN Modules use the addressing convention that the
     Geographical VME address is slot << 19.  We use the convention
     that these modules are then remapped to slot << 24.
*/
void
CCAENV830::MapModule() 
{
  // Validate the member data:

  if(m_nCrate < 0) {
    ThrowIntString("CCAENV820::MapCard - invalid crate %d",
		   m_nCrate);
  }
  if((m_nSlot < 2) || (m_nSlot > 24)) {	// Can't be controller !
    ThrowIntString("CCAENV820::MapCard - invalid slot %d",
		   m_nSlot);
  }
  

  // If geographically addressed,  First access in geo mode and 
  // then re-program.  We're going to check the module's
  // validity later... hopefully this won't harm anything
  // if this is not a V820/V830:
  //
  if(m_fGeo) {
    m_pModule = new CVmeModule(CVmeModule::geo,
			       m_nSlot << 19,
			       sizeof(CAENV830Registers),
			       m_nCrate);
    
    PutShort(Ader_24, 0);	// Set the new base address.
    PutShort(Ader_32, m_nSlot);	
    PutShort(EnableAder,ADEREnable); // Turn on the new address.
    m_nBase = m_nSlot << 24;	// Let remaining code know it.
    delete m_pModule;
    m_pModule = 0;
    
  } 
  // At this point, the module is supposed to be set up to respond
  // to the address at m_nBase, either because that's what it's
  // rotaries were set to or that's where the module was programmed
  // to be.  Now we create the 'permanent' CVmeModule for the
  // registers and a temporary one for the prom.  Pull the prom
  // characteristics and take a bit of time to ensure that 
  // this module is what we think it is...

  CVmeModule prom(CVmeModule::a32d32,
		  m_nBase + CAENV830_PROMBASE,
		  sizeof(CAENV830ROM), 
		  m_nCrate);
  m_nModuleType = ((GetProm(prom, ModelH) & 0xff) << 16 ) |
                  ((GetProm(prom, ModelM) & 0xff) <<  8 ) |
                   (GetProm(prom, ModelL) & 0xff);
  m_nSerialno   = ((GetProm(prom, SerialH) & 0xff)  << 8)  |
                   (GetProm(prom, SerialL) & 0xff);
  if((m_nModuleType  != 820) && 
     (m_nModuleType  != 830)) {
    ThrowIntString("CCAENV830::MapModule Module type invalid %d",
	     m_nModuleType);
  }
  // Now create the 'permanent' register map.
  
  m_pModule = new CVmeModule(CVmeModule::a32d32,
			     m_nBase,
			     sizeof(CAENV830Registers),
			     m_nCrate);

  // We attempt to set the module's slot number:
  // If this module accepts geo addressing, the slot will already
  // match the slot param.  If not, then this will set the
  // slot address for the data.  It is considered an error to 
  // use base addressing on a non geo accessed slot.. so we'll
  // throw an error if the slot address reg. doesn't match after
  // all this is done:

  PutShort(Slot, m_nSlot);
  if((GetShort(Slot) & 0x1f) != m_nSlot) {
    ThrowIntString("CCAENV830::MapModule - Module's slot register \
doesn't match requested slot %d",
	     m_nSlot);
  }
  m_nFirmware = GetShort(FirmwareRev) & 0xff;
  
}
/*!
   Setup a module.  The default setup is:
   - All channels are enabled.
   - The trigger mode is random.
   - Data is in tagged (narrow) format.
   - Header is enabled.
   - Front panel clear clears MEB.
   - Auto reset.
   - Trigger counter is 1.
*/
void
CCAENV830::InitModule()
{
  // Set module to power up settings, and clear the data
  Reset();
  Clear();

  //Setup the module to the default config:

  SetEnableMask(0xffffffff);	// All channels on.
  SetTriggerMode(Random);
  SetNarrow();
  EnableHeader();
  FPClearsMEB(true);
  EnableAutoReset();

}
/*!
  Figure out how many longwords are in an event.  This value is
  the sum of:
  - The number of enabled channels.
  - 1 if the header is enabled.
  This computed value is stuck in to m_nEventLength since it
  is a reliable indicator of the event size where the
  first word of the event is not if the header is disabled!!
*/
void
CCAENV830::ComputeEventSize()
{
  m_nEventLength = 0;
  if((GetShort(Csr) & CSRHeader) != 0) m_nEventLength++;
  
  uint32_t nEnables = GetLong(Enables);
  for(int i =0; i < 32; i++) {
    if( ((1 << i) & nEnables) != 0) m_nEventLength++;
  }

}
/*!
  Destroy any memory map held on the module:
*/
void
CCAENV830::UnmapModule()
{
  delete m_pModule;
  m_pModule = 0;
}
/*!
    Read Event: Reads the next event from the MEB.
    To do this we must:
    - Determine the size of the event (stored in m_nEventLength).
    - Read that many longs from the MEB.

    \param pBuffer (void* out):
    The buffer into which the event will be read.  This must be
    at least m_nEventLength long.

    \return int
    The number of words read from the module 
    (m_nEventLenght * sizeof(uint32_t)/sizeof(uint16_t))


    \note
    - If data are not ready, no words are returned
    - This is optimized in the case where the interface has
      mapping capabilities.
*/
int
CCAENV830::ReadEvent(void * pBuffer) 
{
  if(isDataReady()) {

    uint32_t* p(reinterpret_cast<uint32_t*>( pBuffer));
    for(int i = 0; i < m_nEventLength; i++) {
      *p++ = GetLong(MEB[i]);
    }

    return m_nEventLength * sizeof(uint32_t)/sizeof(uint16_t);
  }
  else {
    return 0;
  }
}

/*!
   Read an individual instantaneous scaler channel.
   \param nChannel (int in):
      Number of channel to read must be in the range [0,31].
   \return int
      Instantaneous value of the counter for the selected channel.
   \throw string
      If nChannel is invalid, an explanatory string exception
      is thrown.
*/
int
CCAENV830::ReadCounter(int nChannel)
{
  if((nChannel < 0) || (nChannel > 31)) {
    char text[1000];
    string msg;
    sprintf(text, "Invalid channel %d (must be in [0,31] in %s\n",
	    nChannel, "CCAENV830::ReadCounter");
    msg = text;
    throw msg;
  }
  return GetLong(Counters[nChannel]);
}
