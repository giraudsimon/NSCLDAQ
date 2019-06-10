/*
  Implementation of the CCAENV1x90 class.
  For more information about this class, see the associated header:
  CCAENV1x90.h
*/

// Headers etc.

#include <config.h>

#include "CCAENV1x90.h"
#include "CCAENV1x90Registers.h"
#include "CCAENV1x90Data.h"
#include "CCAENV1x90Opcodes.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <os.h>

#include <string>
#include <vector>



// Namespace imports:


using namespace std;

using namespace DesignByContract;
using namespace CCAENV1x90Registers;
using namespace CCAENV1x90Data;

// Static class level data.  This data is present in order
// To allow us to wrap some bit definitions into a name-space
// like scope via class inclusion.  These definitions are not
// suitable for the registers nor the data namespaces.
//

// Chip error bits:

const uint16_t CCAENV1x90::ERR_VERNIER    (0x0001);
const uint16_t CCAENV1x90::ERR_COARSE     (0x0002);
const uint16_t CCAENV1x90::ERR_SELECT     (0x0004);
const uint16_t CCAENV1x90::ERR_L1PARITY   (0x0008);
const uint16_t CCAENV1x90::ERR_TFIFOPARITY(0x0010);
const uint16_t CCAENV1x90::ERR_MATCHERROR (0x0020);
const uint16_t CCAENV1x90::ERR_RFIFOPARITY(0x0040);
const uint16_t CCAENV1x90::ERR_RDOSTATE   (0x0080);
const uint16_t CCAENV1x90::ERR_SUPPARITY  (0x0100);
const uint16_t CCAENV1x90::ERR_CTLPARITY  (0x0200);
const uint16_t CCAENV1x90::ERR_JTAGPARITY (0x0400);

// Tap contact bits (used in calibrate delay line).
// These bits are shifted into position appropriately
// to control fine adjustments of the RC delay line.

const uint16_t CCAENV1x90::TAP_CONTACT1(4);
const uint16_t CCAENV1x90::TAP_CONTACT2(2);
const uint16_t CCAENV1x90::TAP_CONTACT3(1);

//  Below are constants that are not worth member status
//  but also should not be used as 'magic numbers'.

static const uint32_t MINMATCHWIDTH(1);
static const uint32_t MAXMATCHWIDTH(0xfff);
static const uint32_t MATCHMASK(0xfff);

static const int          MINWINDOWOFFSET(-2048);
static const int          MAXWINDOWOFFSET(40);
static const int          WINDOWOFFSETMASK(0xfff);

static const uint32_t MINEXTRAMARGIN(0);
static const uint32_t MAXEXTRAMARGIN(2047);
static const uint32_t EXTRAMARGINMASK(0xfff);

static const uint32_t MINREJECTMARGIN(0);
static const uint32_t MAXREJECTMARGIN(2047);
static const uint32_t REJECTMARGINMASK(0xfff);

static const uint32_t EDGEDETECTIONMASK(3);

static const uint32_t EDGERESOLUTIONMASK(3);
static const uint32_t LERESOLUTIONMASK(7);

static const uint32_t PWRESOLUTIONRSHIFT(8);
static const uint32_t PWRESOLUTIONMASK(0xff00);

static const uint32_t TRUEBIT(1); // TRUE for bools from sequencer.

static const uint32_t COARSEMAX(0x7ff);
static const uint32_t COARSEMASK(0x7ff);

static const uint32_t VERNIERMAX(0x1f);
static const uint32_t VERNIERMASK(0x1f);

static const uint32_t CHANOFFSETMAX(0xff);
static const uint32_t CHANOFFSETMASK(0xff);

static const uint32_t TAPCONTACTMASK(0x7);
static const uint32_t TAPCONTACTRSHIFT(3);

static const uint32_t  MAXTESTVALUE(0x7fffff);

static const uint32_t MAXGEO(0x1f);
static const uint32_t GEOMASK(0x1f);

static const uint32_t RESET_DELAY(1000);

// Utility inline functions:

//! Convert byte offset to word offset.

static inline uint32_t OffsetW(uint32_t nByteOffset) 
{
  return nByteOffset/sizeof(uint16_t);
}
//! Convert byte offset into long offset.

static inline uint32_t OffsetL(uint32_t nByteOffset)
{
  return nByteOffset/sizeof(uint32_t);
}

//!  Convert a format string and a single number into an output 
//   string:

template <class T>
static inline string FormatValue(const char* pFormatString, T value)
{
  int nformatsize = strlen(pFormatString);
  char*  message = new char[nformatsize+100];
  sprintf(message, pFormatString, value);
  string m(message);
  delete []message;
  return m;

}

//! Extract HI 1/2 of a longword:

static uint16_t 
inline HI(uint32_t nValue) 
{
  return static_cast<uint16_t>((nValue >> 16) & 0xffff);
}

//! Extract Low 1/2 of a longword:

static uint16_t
inline LO(uint32_t nValue)
{
  return static_cast<uint16_t>(nValue & 0xffff);
}


///////////////// constructors etc. ////////////////////////////

/*!
  Construct an object that represents a single CAEN V1190 or V1290.
  Access is gotten via VmePtr to the module's register and config 
  rom address spaces.  Once the address space is verified to have 
  a supportable module, the member variables are initialized and 
  the config rom address space de-accessed.  

  Configuration information about the module is set to the 
  module's default power up configuration, and the module is reset.


 \param nSlot	(uint32_t	in)
   The slot number of the board.  Used program the GEO register 
   of the board as this code does not support GEO addressing.
 \param nCrate	(uint32_t	in)	
   The number of the VME crate in which the module is inserted.
 \param nBase	(uint32_t	in)
   Base address of the module.  The module base is assumed to be
   in the A32 address space.


   \throw DesignByContract::Require if the geographical address or
      crate are invalid.
   \throw CException  If the constructor is unable to access the
   VME crate the module is claimed to be in.
   \throw Ensure If the constructor could access the module but 
   it does not appear to be an 1190 or 1290.
   \throw string - If the geo register of the module could not be
   set (module supports geographical addressing, and that's not
   supported in this release of the device support).

   \pre nSlot is in the range [2, 20] (these modules cannot be
        system controllers).
   \pre nCrate  is in the range [0,7].

   \post m_nModel is 1190 or 1290,
   \post m_cVersion is A, B, or N.
*/
CCAENV1x90::CCAENV1x90(uint32_t nSlot,
		       uint32_t nCrate,
		       uint32_t nBase) :
  m_nBase(nBase),
  m_nCrate(nCrate),
  m_nSlot(nSlot),
  m_pRegisters(CVmeModule::a32d32, nBase, RegisterSize, nCrate)
{
  // Check preconditions:

  REQUIRE(((nSlot  >= 1)  && (nSlot <=20)), "Slot out of range");
  REQUIRE(((nCrate >= 0)  && (nCrate <= 7)), "Crate out of range");

  // Initializers create the module object for the 
  // control registers and data buffer.
  // we need to make one for the config prom so that we can
  // validate the module.

  CVmeModule pRom(CVmeModule::a32d32, 
		  m_nBase+ConfigRom, 
		  ConfigRomSize,
		  m_nCrate);

  // The configuration PROM can tell us if this is a valid board:

  ENSURE(ValidBoard(pRom), 
	 "Board is not a valid CAEN V1x90 module");

  // Since the board is valid, set up the member variables
  // from the board properties as described in the config rom
  // and the various board registers:

  BoardProperties(pRom);


  // Check postconditions for model etc.:

  ENSURE(((m_nModel == 1190) || (m_nModel == 1290)),
	 "Model number is incorrect on postcondition check");
  ENSURE(((m_cVersion == 'A') || (m_cVersion == 'B') || 
	  (m_cVersion == 'N')),
	 "Version is not A,B, or N in postcondition check");


  // Everything is good. Set slot,  turn on the FIFO:

  Reset();
  
}

/*!

Destructor: Deaccess the address space of the card.

Our destruction will result in the destruction of the VMEptr.
m_pRegisters which will free up all the resources we own.

 */
CCAENV1x90::~CCAENV1x90()
{}


/*!
  Determine if a specific bit is set in the control register.

  \param bit	(uint16_t	in)
  number from of the bit to check

  \return bool
  \retval -true if 1 << bit is set in the control register.
  \retval -false if 1 << bit is not set in the control register.

 */
bool 
CCAENV1x90::isSetCR(uint16_t bit)
{
  return ((ReadCR() & (1 << bit)) != 0);
}
/*!
  Terminate
  Set the module's software termination.
  Enables the software to control module termination and 
  turns on termination.
*/
void
CCAENV1x90::Terminate()
{
  BitSetCR(ControlRegister::NTERM_SW); // Enable software termination.
  BitSetCR(ControlRegister::NTERM); // And request termination.
}
/*!
  Unterminate
  set the module to unterminated
  Enables the software to control module termination and 
  turns off termination.

*/
void
CCAENV1x90::Unterminate()
{
  BitSetCR(ControlRegister::NTERM_SW); // Enable software termination ctl
  BitClearCR(ControlRegister::NTERM); // Request unterminated.
}
/*!
  TerminateViaSwitches
  Allow module termination to be controlled via the termination 
  switches mounted on the board.
  Turns off software control of the module termination.  
  When this is done, the module termination is controlled by a 
  switch on the module itself.  Note that this module does not 
  support 'partial' termination.

 */
void 
CCAENV1x90::TerminateWithSwitch()
{
  BitClearCR(ControlRegister::NTERM_SW); // Turn off software termination ctl.
}
/*!

Enables the module to write extended trigger time tagging 
information into the event if the module is in trigger matching 
mode.

 */
void
CCAENV1x90::EnableTriggerTagTime()
{
  BitSetCR(ControlRegister::NTRIGGER_TAG_ENABLE);
}
/*!
Disables the module from writing extended trigger time 
tagging information into the event if the module is in 
trigger matching mode.
 */
void
CCAENV1x90::DisableTriggerTagTime()
{
  BitClearCR(ControlRegister::NTRIGGER_TAG_ENABLE);
}
/*!
  Returns true if the module's status register indicates that 
  data is available in the module's fifo.

  \return bool
  \retval true - the module has data.
  \retval false - there is no data in the module's data buffer.


 */
bool
CCAENV1x90::DataReady()
{
  return isSetSR(StatusRegister::NDATA_READY);
}
/*!

Returns true if the module's status register indicates that the
mulitevent buffer is nearly full. The almost full buffer level 
is set using the AlmostFull member function.

\return bool
\retval true - if the almost full bit is present in the status reg.
\retval false- if the almost ful bit is not set.


*/
bool
CCAENV1x90::AlmostFull()
{
  return isSetSR(StatusRegister::NALM_FULL);
}

/*!
  Returns true if the module's status register indicates that 
  the module's multievent buffer is full.  
  The full condition is defined as fewer than 32 free words in 
  the output buffer.

  \return bool
  \retval true - If the module's full bit is set.
  \retval false- If the module's full bit is not set.



 */
bool
CCAENV1x90::isFull()
{
  return isSetSR(StatusRegister::NFULL);
}

/*!
returns true if the module is in trigger matching mode.  
In trigger matching mode, the module associates a set of input 
hits with an event that consists of a time window relative 
to the  time of the trigger input.  

Note that by setting this window appropriately, trigger 
matching mode can simulate both common start and common stop 
mode TDC's, as well as set the window to be a nearly arbitrary 
time interval about the trigger input time.

See the hardware manual for a description of the contraints on 
trigger windows.

  \return bool
  \retval true - Module is in trigger match mode.
  \retval false - module is in continuous storage mode.



 */
bool
CCAENV1x90::isTriggerMatching()
{
  return isSetSR(StatusRegister::NTRG_MATCH);
}
/*!
Returns true if the module has been enabled to produce TDC chip 
headers in trigger matching mode. 
See e.g. isTriggerMatching for a description of trigger 
matching mode.

\return bool
\retval - true if TDC chip headers are enabled.
\retval - false if TDC chip headers are disasbled.


 */
bool
CCAENV1x90::isHeaderEnabled()
{
  return isSetSR(StatusRegister::NHEADER_EN);
}

/*!
  Returns true if the module's inputs are terminated regardless 
  of how this is done.  See Terminate Unterminate and 
  TerminateViaSwitches for more information about the module 
  termination modes.
 
  \return bool
  \retval true   - The module is terminated either via software 
                   or the manual switch.
  \retval false  - The module is not terminated either via 
                   software or the manual switch
*/
bool
CCAENV1x90::isTerminated()
{
  return isSetSR(StatusRegister::NTERM_ON);
}
/*!
  Did the TDC have an error since last reset.
  Returns true if the status register indicates the specified 
  chip had an error.  The status register has a bit for each of 
  the 4 chips that may be present in a board.  
  This function uses the m_nChipCount member data to ensure 
  that the user is asking about a chip that actually exists on 
  the board.

    \param nChip	(uint32_t	in)
	TDC Chip  number (must be an allowed chip number 
	numbered from zero for the actual board).

    \return bool 
    \retval true - the chip has had an error.
    \retval false- the chip has not had an error.

    \pre nChip < m_nChipCount (number of chips on board).
 

 */
bool
CCAENV1x90::HadError(uint32_t nChip)
{
  // Check preconditions.

  REQUIRE(nChip < m_nChipCount, "Illegal chip number for device");
  
  // Compute the chip bit.  There's a huge assumption
  // here that the chip error bits are contiguous.
  // The debug code below will check this:

  ENSURE(((1 << StatusRegister::NCHIP0_ERROR)     |
	  (1 << (StatusRegister::NCHIP0_ERROR+1)) |
	  (1 << (StatusRegister::NCHIP0_ERROR+2)) |
	  (1 << (StatusRegister::NCHIP0_ERROR+3))) == 
	                              ((StatusRegister::CHIP0_ERROR) |
				       (StatusRegister::CHIP1_ERROR) |
				       (StatusRegister::CHIP2_ERROR) |
				       (StatusRegister::CHIP3_ERROR)),
	 "Chip error bits are not continguous in the register");
  uint32_t nChipBit = 1 << (StatusRegister::NCHIP0_ERROR + nChip);

  // Return true if the associated error bit is set:

  return isSetSR(nChipBit);

}
/*!
  Reads the status register and decodes the resolution field.  
  The resolution field is returned in ps

  \return uint32_t 
  \retval 800   - Resolution is 800ps.
  \retval 200   - Resolution is 200ps.
  \retval 100   - Resolution is 100ps.
  \retval 25    - Resolution is 25ps.


 */
int
CCAENV1x90::ReadResolution()
{
  int Resmask = SR() & StatusRegister::RESOLUTIONMASK;

  if(Resmask == StatusRegister::RES_800ps) {
    return 800;
  }
  else if (Resmask == StatusRegister::RES_200ps) {
    return 200;
  }
  else if (Resmask == StatusRegister::RES_100ps) {
    return 100;
  }
  else if(Resmask ==  StatusRegister::RES_25ps) {
    m_fIsHiResMode = true;	// module in hi res mode.
    return 25;
  } 
  else {
    CHECK(0, "Unrecognized resolution code");
  }
}

/*!
  Reads the module status register and returns true if the 
  module is in edge pair mode. Note that for the 1290, pair 
  mode is only possible when the resolution is not 25ps/channel.

  \return bool
  \retval   true     - module is in pair mode.
  \retval   false    - module is not in pair mode.

 */
bool
CCAENV1x90::isPairMode()
{
  // I'm going to do a bit more work than I probably need to
  // by not assuming the pair bit is meaningful when the
  // 1290 is in pair mode:

  if((m_nModel == 1290) && (ReadResolution() == 25)) {
    return false;		// 1290 in hi res.
  }
  return isSetSR(StatusRegister::NPAIR);
}
/*!
 Returns true if the module claims it received triggers that it 
 was not able to process either due to buffer full or pileup 
 conditions. 

 \return bool
 \retval true    - Triggers have been dropped by the module.
 \retval false   - Triggers have not been dropped by the module.

 */
bool
CCAENV1x90::WereTriggersLost()
{
  return isSetSR(StatusRegister::NTRIGGERLOST);
}
/*!
  Attempts to set nId in the GEO register of the module.  
  The id must be in the range 0-31.  Since it is possible to 
  have a module that understands geographical addressing 
  accessed via base addressing, and since those modules won't 
  allow you to program the GEO address (GEO will 
  unconditionally reflect the slot number of the module), 
  After writing the address, the GEO is read back. 
  If the read back value does not match the written value a 
  string exception will be thrown to indicate this.

   \param nId	(uint32_t	 in)
   The Desired GEO id for the board.


   \throw string - in the event the GEO register cannot be 
                   programmed.  This is the case if the module
		   support geographical address recognition.
   \pre nSlot >= 0  This is a virtual slot so slot 0 is allowed.
   \pre nSlot < 256  Number of used bits in the register.


 */
void 
CCAENV1x90::SetGeographicalID(uint16_t nSlot)
{
  // Check preconditions:


  REQUIRE(nSlot <= MAXGEO, "Slot number too big.");

  // Program the geo register:

  m_pRegisters.pokew(nSlot, OffsetW(WVirtualSlot));
  
  // Ensure it stayed programmed:

  if (GetGeographicalID() != nSlot) {
    throw 
      string ("Attempt to set geo register of geo-recognizing module");
  }

}

/*!
   Get the current geo Id of the module.  The GEO is a virtual
   slot number for the module when it does not support
   GEO addressing intrinsically.
   
   \return uint16_t
   \retval  The current geo ID.
*/
uint16_t
CCAENV1x90::GetGeographicalID()
{
  uint16_t g = m_pRegisters.peekw(OffsetW(WVirtualSlot));
  g &= GEOMASK;
  return g;

}
/*!
  Peform a software reset on the module.
 */
void
CCAENV1x90::Reset()
{
  m_pRegisters.pokew(0, OffsetW(WReset));
  m_fTriggerMatching = isTriggerMatching();
  WaitMicroWrite();


  // Re-asssert the slot and fifo enable.

  SetGeographicalID(m_nSlot);
  BitSetCR(ControlRegister::NEVENT_FIFO_ENABLE);


}
/*!
  Performs a module clear:
  - The TDCs are cleared
  - The data buffer is cleared
  - The event counter is reset to 0
  - A TDC chip global reset is performed.
  - The TDC configuration is not modified.

 */
void 
CCAENV1x90::Clear()
{
  m_pRegisters.pokew(0, OffsetW(WClear));
  m_fTriggerMatching = isTriggerMatching();
}

/*!
  Resynchronizes the TDC chip event counter with the on board 
  event counter.  Note that the manual says that this does not 
  clear the onboard event counter.
 */
void
CCAENV1x90::EventReset()
{
  m_pRegisters.pokew(0, OffsetW(WEventReset));
  m_fTriggerMatching = isTriggerMatching();
}

/*!
Performs a software trigger of the module.  
Presumably if the module is in trigger matching mode, 
this will establish a matching window.  
I imagine this is mostly used to test the module.
 */
void
CCAENV1x90::Trigger()
{
  m_pRegisters.pokew(0, OffsetW(WSWTrigger));
}

/*!
  Returns the number of triggers the module has received since 
  the last Clear or Reset function. I think this is meaningless 
  unless the module is in trigger matching mode.

  \return uint32_t
  \retval  The number of triggers received since last reset/clr.

*/
uint32_t
CCAENV1x90::TriggerCount()
{
  return m_pRegisters.peekl(OffsetL(LEventCounter));
}

/*!
  Returns the number of events in the output buffer 
  (only meaningful in trigger matching mode).

   \return uint16_t
   \retval number of complete events in the output buffer.
 */
uint16_t
CCAENV1x90::EventCount()
{
  return m_pRegisters.peekw(OffsetW(WEventStored));
}

/*!
  Sets a new almost full threshold value.  When more than 
  nWords longs are stored (backed up in) in the event buffer, 
  the board will  indicate an almost full condition.  
  This condition can be detected via the AlmostFull function.

  \param nWords	(uint32_t	in)	
      New threshold value [1-32767]

  \pre nWords >= 1
  \pre nWords <= 32735  (Magic number from the hardware manual.


 */
void
CCAENV1x90::SetAlmostFullLevel(uint32_t nWords)
{
  // Ensure preconditions are met:

  REQUIRE(nWords > 0, "Number of words is too small");
  REQUIRE(nWords <= 32735, "Number of words too big");

    // program the register.

  m_pRegisters.pokew(static_cast<uint16_t>(nWords), 
		     OffsetW(WAlmostFullLevel));

}

/*!
 Returns the current value of the almost full threshold.  
 This is the alst value set by a call to SetAlmostFullLevel.  
 Note that the power up value of this is 64 longs which is 
 quite small.

 \return uint16_t
 \retval the threshold above which the module declares itself
         almost full.
*/
uint16_t
CCAENV1x90::GetAlmostFullLevel()
{
  return m_pRegisters.peekw(OffsetW(WAlmostFullLevel));
}

/*!
  Selects which of the four possible meanings the OUT_PROG 
  signal on the ECL output will have. Legal values for 
  Signal are:
  - DATA_READY   - Asserted when the module has data in its 
                  output buffer
  - FULL         - Asserted when the module output buffer is full.
  - ALMOST_FULL  - Asserted when the module is in the almost full 
                   condition
  - ERROR        - Asserted when there's an un-reset TDC error 
                   condition.
   Note that these conditions correspond to the state of bits 
   in the module status register.

  \param Signal (ECLOutputSelect in) 
     Selects the desired signal value.

\pre Signal is a member of the set of legal enum values.
     unfortunately other than painstaking checking there's no
     way to know what that set is.

 */
void
CCAENV1x90::DefineECLOutput(CCAENV1x90::ECLOutputSelect Signal)
{
  // Verify the precondition:

  REQUIRE(((Signal == DATA_READY)   ||
	 (Signal == FULL)         ||
	 (Signal == ALMOST_FULL)  ||
	 (Signal == ERROR)),
	"Signal parameter is invalid.");


  // Convert our enum to register bits.

  uint16_t value;

  switch (Signal) {
  case DATA_READY:
    value = OutputControl::DATA_READY;
    break;
  case FULL:
    value = OutputControl::FULL;
    break;
  case ALMOST_FULL:
    value = OutputControl::ALM_FULL;
    break;
  case ERROR:
    value = OutputControl::ERROR;
    break;
    
    // Default case is prevented by precondition.
  }

  m_pRegisters.pokew(value, OffsetW(WOutputControl));

}
/*!
  Reads the OUT_PROG control register and returns a value 
  indicating how it has been programmed.


  \return ECLOutputSelect.  One of the legal values for the ECL OUT_PROG signal:
  \retval DATA_READY - Asserted when the module has data in its 
                       output buffer
  \retval FULL       - Asserted when the module output buffer 
                       is full.
  \retval ALMOST_FULL- Asserted when the module is in the 
                       almost full condition
  \retval ERROR      - Asserted when there's an un-reset TDC 
                       error condition. Note that these 
		       conditions correspond to the state of 
		       bits in the module status register.

 */
CCAENV1x90::ECLOutputSelect
CCAENV1x90::GetECLOutputDefinition()
{
  uint16_t 
    value = m_pRegisters.peekw(OffsetW(WOutputControl)) &
                                           OutputControl::MASK;

  // Convert the register value to the enum... the default
  // of the case below is illegal:

  if(value == OutputControl::DATA_READY) {
    return DATA_READY;
  } 
  else if (value == OutputControl::FULL) {
    return FULL;
  } 
  else if (value == OutputControl::ALM_FULL) {
    return ALMOST_FULL;
  }
  else if (value ==  OutputControl::ERROR) {
    return ERROR;
  }
  else {
    ENSURE(0, "Illegal value from output control register");
  }

}
/*!
  Returns the number of events in the output buffer.  
  This software support module turns on the event FIFO.  
  The event FIFO is a handy little register that contains the 
  number of words for each event.  It can be used by the caller 
  when the module is in Trigger match mode to know how big a 
  read to issue for an event.

  \return uint16_t
  \retval  The number of fifo entries in the FIFO.
 */
uint16_t
CCAENV1x90::EventFIFOCount()
{
  return m_pRegisters.peekw(OffsetW(WEventFIFOStored)) & 
                         EventFIFO::FIFOCOUNT_MASK;
}
/*!
  Assumes the fifoentry parameter is a longword read from the 
  event fifo and returns the sequence number of the event that 
  corresponds to it.

\param fifoentry	(uint32_t	IN)
	A longword read from the FIFO via ReadEventFIFO

\return uint16_t
\retval Event number corresponding to the fifo entry.


 */
uint16_t
CCAENV1x90::FIFOEventNumber(uint32_t fifoentry)
{
  return (fifoentry & EventFIFO::EVENTCOUNT_MASK) >>
                      EventFIFO::EVENTCOUNT_RSHIFT;
}

/*!
  Given a longword read from the event FIFO via ReadEventFIFO, 
  this function will return the number of words in the 
  corresponding event (note that in this case words are really 
  32 bit longwords).

    \param fifoentry	(uint32_t	IN)
    A longword read from the event FIFO via ReadEventFIFO

    \return uint16_t
    \retval Size of event corresponding to the event fifo entry
            (in longs).

 */
uint16_t
CCAENV1x90::FIFOWordCount(uint32_t fifoentry)
{
  return (fifoentry & EventFIFO::WORDCOUNT_MASK) >>
                      EventFIFO::WORDCOUNT_RSHIFT;
}

/*!
  Read the top word of the event fifo.  Note that this is a 
  destructive read.  Once read the event FIFO for an event can 
  never be re-read.  When the module is running in trigger 
  match mode, and data is ready, read the event fifo, and use 
  the result as the word count for the ReadEvent member 
  function call.  
  - It is not unambiguously clear to me what happens when the 
    event fifo gets full (e.g. the user never reads it), 
    however I believe that the board will assert busy in that 
    case and refuse triggers.
  - Note that the return value is logically two words. 
    The high order 16 bits contains the event number, and the 
    low order 16 bits the size of the event.

 \return uint32_t

 \pre The event fifo must have data (fifo ready).

 */
uint32_t
CCAENV1x90::ReadEventFIFO()
{
  // check preconditions.

  REQUIRE(isEventFIFOReady(), "Attempting to read empty FIFO");

  return m_pRegisters.peekl(OffsetL(LEventFIFO));
}

/*!
  Returns true if the event fifo has any event descriptions in 
  it.

  \return bool
  \retval true   - There's at least one entry in the event fifo.,
  \retval false  - There are no entries in the event fifo.
 */
bool
CCAENV1x90::isEventFIFOReady()
{
  
  return ((m_pRegisters.peekw(OffsetW(WEventFIFOStatus)) &
	   FIFOStatus::EVFIFODATA_READY) != 0);
}

/*!
  The event FIFO is 1024 events deep.  This function returns 
  true if all of the slots in the event fifo are filled with 
  event descriptions.
  \return bool
  \retval true     - True if the event fifo is completely full.
  \retval false    - False if the event fifo isn't completely
                     full.
 */
bool
CCAENV1x90::isEventFIFOFull()
{
  return ((m_pRegisters.peekw(OffsetW(WEventFIFOStatus)) &
	   FIFOStatus::EVFIFO_FULL) != 0);
}


///////////////////////// Sequencer functions: ///////////////////

/*!
Enables the module to run in trigger matching mode.  
In trigger matching mode, the TDC chips will group hits that 
occur within a programmable matching window of the trigger 
(software or hardware input) into an event. 
 */
void 
CCAENV1x90::TriggerMatchMode()
{
  m_fTriggerMatching = true;
  WriteMicro(CCAENV1x90Opcodes::TRG_MATCH);
  WaitMicroWrite();
}

/*!
Enables the module to run in continuous storage mode.  In continous storage mode, the TDC chips emit hits continuously regardless of the trigger signal.  The timing of hits in this mode is relative to the last module reset.  Since the TDC values are only 20 bits (roughly 1E06), in order to time over long periods it will be necessary to arrange to have either some other fiducial against which these are compared or alternatively some periodic source of resets (note however that resets will also reset the TDC programming).
 */
void
CCAENV1x90::ContinuousStorageMode()
{
  m_fTriggerMatching = false;
  WriteMicro(CCAENV1x90Opcodes::CONT_STOR);
  WaitMicroWrite();

}
/*!
  The TDC chips on the board are read by passing a readout token 
  to the first TDC chip. When that chip has finished 
  transferring data it passes the token on to the next, until 
  the token is passed from the last TDC chip back to the board 
  logic indicating all chips have been read.  TDC chips can  
  operate in two modes: 
  - Hold on to the read token until the end of event (trigger 
    match mode) or no more data present (continuous storage 
    mode).
  - Release the token after transferring a single word.
    This function enables the first of these readout transfer 
    modes.Note that if the TDC is in trigger matching mode then 
    the TDC chips will TransferUntilDone regardless of what you 
   tell them to do.
 */
void
CCAENV1x90::TransferUntilDone()
{
  WriteMicro(CCAENV1x90Opcodes::SET_KEEP_TOKEN);
  WaitMicroWrite();

}
/*!
  See TransferUntilDone for details about the chip readout 
  scheme.  This function tells the TDC chips to release the 
  readout token after each word has been transferred. 
 */
void
CCAENV1x90::TransferOneAtATime()
{
  WriteMicro(CCAENV1x90Opcodes::CLEAR_KEEP_TOKEN);
  WaitMicroWrite();
  
}

/*!
 Loads the default TDC chip configuration. 
 */
void
CCAENV1x90::LoadDefaultConfig()
{
  WriteMicro(CCAENV1x90Opcodes::LOAD_DEF_CONFIG);
  WaitMicroWrite();

}
/*!
  Saves the current TDC chip programming to on board 
  Non-volatile ram.  Once saved, the configuration can be bulk 
  loaded via LoadUserConfig or even enabled to load on power up 
  via AutoLoadUserConfig.  Use the latter with care on pool 
  modules as the person using the module after you will very 
  likely make assumptions about the power up state of the board 
  which are not correct otherwise. Conversely, with pool modules 
  you may want to call AutoLoadDefaultConfig and 
  LoadDefaultConfig early in your initialization to remove the 
  effect of any prior AutoLoadUserConfig  

  The items saved are:
  - The Acquisition mode (trigger match or continuous storage)
  - The window width
  - The window offset
  - The reject margin
  - The Extra search margin
  - The enabled channel mask.
 */
void
CCAENV1x90::SaveUserConfig()
{
  WriteMicro(CCAENV1x90Opcodes::SAVE_USER_CONFIG);
  WaitMicroWrite();
  Os::usleep(10000);		// Carlo says wait 10ms after this opcode.
}
/*!
Reloads the last saved user configuration into the TDC.  
This can save a bit of time if you have initially set up the 
TDC and have saved the settings, and wish to re-establish them. 
See SaveUserConfig for more information about what is restored.
 */
void
CCAENV1x90::LoadUserConfig()
{
  WriteMicro(CCAENV1x90Opcodes::LOAD_USER_CONFIG);
  WaitMicroWrite();
}
/*!
 Enables the module to load the last saved user settings on power 
up or global reset.   Refer to SaveUserConfig for information 
about what is actually loaded.  Use this with caution on pool 
electronics as this will cause the module to power up in an 
unexpected mode for later users.
*/
void
CCAENV1x90::AutoLoadUserConfig()
{
  WriteMicro(CCAENV1x90Opcodes::AUTOLOAD_USER_CONFIG);
  WaitMicroWrite();
  Os::usleep(10000);		// Carlo says wait 10ms after this opcode.

}
/*!

Enables the module to load the default settings on power up.  
If you are not sure of the last user's intent, you will probably 
want to execute this followed by a global reset rather than 
assuming the module will power up in the default state (See 
AutoLoadUserConfig). See SaveUserConfig for information about 
exactly what gets saved.

 */
void
CCAENV1x90::AutoLoadDefaultConfig()
{
  WriteMicro(CCAENV1x90Opcodes::AUTOLOAD_DEF_CONFIG);
  WaitMicroWrite();
  Os::usleep(10000);		// Carlo says wait 10ms after this opcode.

}
/*!
Sets the trigger matching mode window width.   The trigger 
matching window width effectively determines the full scale range 
of the TDC in trigger matching mode.  There are relationships 
(constraints) that must be maintained between the width and the 
offset.  See the hardware manual 2.4.1 for information about this.


  \pre Width is in the range [0, 0xfff].

 */
void
CCAENV1x90::SetWindowWidth(uint32_t nWidth)
{
  REQUIRE(nWidth >= MINMATCHWIDTH, 
	  FormatValue("Window width must be > %x",
				    MINMATCHWIDTH).c_str());
  REQUIRE(nWidth <= MAXMATCHWIDTH, 
	  FormatValue(" Window width must be <= 0x%x",
				    MAXMATCHWIDTH).c_str());

  WriteMicro(CCAENV1x90Opcodes::SET_WIN_WIDTH);
  WriteMicro(nWidth);
  WaitMicroWrite();

}
/*!
In trigger matching mode, sets the time offset from the trigger to 
start of the matching window.  The window can start anywhere 
from -2048*25n relative to the trigger to 40*25ns.

        \param nOffset	(int	in)
	New offset in 25 ns units.

\pre PreconditionException if the offset is not valid.

 */
void
CCAENV1x90::SetWindowOffset(int nOffset)
{
  REQUIRE(nOffset >= MINWINDOWOFFSET, 
	  FormatValue("Window offset must be at least %d",
			   MINWINDOWOFFSET).c_str());
  REQUIRE(nOffset <= MAXWINDOWOFFSET,    
	  FormatValue("Window offset must be no greater than %d",
			   MAXWINDOWOFFSET).c_str());
  WriteMicro(CCAENV1x90Opcodes::SET_WIN_OFFS);
  WriteMicro(nOffset);
  WaitMicroWrite();

}

/*!
Since trigger matching is done on digitized hits stored in the 
Chip level 1 buffer, it is possible that when the search window is 
complete, there will be hits that are detected but not yet 
transferred to the buffer.   The extra search margin allows the 
user to specify a 'slop' on the end of the matching window to 
allow these his to be detected as well.  This member allows you to 
adjust the width of this margin.

    \param nMargin	(uint32_t	in)
	New margin  in 25ns units.


    \pre nMargin in the interval [0, 2047]

 */
void
CCAENV1x90::SetExtraSearchMargin(uint32_t nMargin)
{
  REQUIRE(nMargin >= 0, 
	  FormatValue(" nMargin must be >=%d",
				     MINEXTRAMARGIN).c_str());
  REQUIRE(nMargin <= 2047, 
	  FormatValue("nMargin must be <= %d",
				     MAXEXTRAMARGIN).c_str());

  WriteMicro(CCAENV1x90Opcodes::SET_SW_MARGIN);
  WriteMicro(nMargin);
  WaitMicroWrite();


}
/*!

The reject margin indicates a time period in which hits in the 
module L1 buffer at the start of the match window are assumed to 
be associated with times prior to the match window, and can be 
purged from the L1 buffer.  This function allows the user to 
select the reject margin.

   \param nMargin	(uint32_t	IN)
   reject margin in 25ns units.

\pre Reject margin must be in the interval [0, 2047].

 */
void
CCAENV1x90::SetRejectMargin(uint32_t nMargin)
{
  REQUIRE(nMargin >= 0, 
	  FormatValue("nMargin must be >= %d",
				    MINREJECTMARGIN).c_str());
  REQUIRE(nMargin <= 2047, 
	  FormatValue("nMargin must be <= %d",
				    MAXREJECTMARGIN).c_str());

  WriteMicro(CCAENV1x90Opcodes::SET_REJ_MARGIN);
  WriteMicro(nMargin);
  WaitMicroWrite();
}
/*!
  When enable trigger time subtraction is turned on, all of the 
times are expressed in terms of offsets from the beginning of the 
match window (note that this is not the same as relative to the 
trigger!!!).  This is only effective in trigger matching mode.

 */
void
CCAENV1x90::EnableTriggerTimeSubtraction()
{
  WriteMicro(CCAENV1x90Opcodes::EN_SUB_TRG);
  WaitMicroWrite();
}
/*!
  When enable trigger time subtraction is turned off, all of the 
times are expressed as offsets from the last bunch reset (CRST 
input I believe). 
 */
void 
CCAENV1x90::DisableTriggerTimeSubtraction()
{
  WriteMicro(CCAENV1x90Opcodes::DIS_SUB_TRG);
  WaitMicroWrite();

}

/*!
Retrieves the current trigger configuration and returns it the 
function return value. The Trigger configuration is a block of 
data that can be queried to return information about the trigger 
setup.  

See:
-	GetMatchWindow - to retrieve the current matching window 
        width.·	
-       GetWindowOffset -  to retrieve the offset of the matching 
                           window relative to the trigger.
-	GetExtraSearchMargin - to retrieve information about the 
                                extra search margin timing.
-	GetRejectMargin - to retrieve information about the reject 
                          margin.
-	IsTriggerTimeSubtracted - to determine if the time is 
                  window relative or bunch-count-reset relative.

  \return TriggerConfiguration 
  \retval a block that can be passed to the get functions that
      decode the trigger configuration.

 */
CCAENV1x90::TriggerConfiguration
CCAENV1x90::GetTriggerConfiguration()
{
  
  TriggerConfiguration config;
  MicroTransaction(CCAENV1x90Opcodes::READ_TRG_CONF,
		   &config, sizeof(config)/sizeof(uint16_t));
  return config;
}
/*!
This utility function allows you to decode the match window from a 
trigger configuration block that was returned from the 
GetTriggerConfiguration function.

   \param Config	(const TriggerConfiguration	IN)
	Trigger configuration returned from GetTriggerConfiguration

   \return uint32_t 
   \retval the trigger match window widthin 25ns units.


 */
uint32_t
CCAENV1x90::GetMatchWindow(TriggerConfiguration Config)
{
  return (Config.s_MatchWidth & MATCHMASK);
}
/*!

Utility function to decode the trigger window offset from a 
trigger configuration block returned from GetTriggerConfiguration.

  \param Config	(TriggerConfiguration 	IN)
  Trigger configuration returned from GetTriggerConfiguration

  \return uint32_t 
  \retval the trigger offset, a value in the range [-2048 - 40]*25ns

 */
int 
CCAENV1x90::GetWindowOffset(TriggerConfiguration Config)
{
  int nOffset = (Config.s_MatchOffset & WINDOWOFFSETMASK);

  // nOffset is a signed 12 bit number... if necessary extend the  
  // sign; a & ~(a >> 1) is the highest set bit in a, if a is a mask
  // of contiguous 1's.


  if(nOffset & (WINDOWOFFSETMASK & ~(WINDOWOFFSETMASK >> 1))) {
    nOffset |= ~(WINDOWOFFSETMASK);
  }
  return nOffset;
}
/*!
  Utility function that decodes the Extra search margin from a 
trigger configuration block returned by a call to 
GetTriggerInformation.

   \param Config	(TriggerConfiguration	IN)
	Trigger configuration returned from GetTriggerConfiguration

    \return uint32_t 
    \retval the Extra search margin in theragne [0,2047]*25ns.


 */
uint32_t
CCAENV1x90::GetExtraSearchMargin(TriggerConfiguration Config)
{
  return Config.s_MatchExtra & EXTRAMARGINMASK;
}

/*!
  Get extra rejection time margin froma  trigger configuration
  block returned by a call to GetTriggerInformation.

   \param Config	(TriggerConfiguration	IN)
	Trigger configuration returned from GetTriggerConfiguration

    \return uint32_t 
    \retval the Extra rejection time margin.

 */
uint32_t
CCAENV1x90::GetRejectMargin(TriggerConfiguration Config)
{
  return Config.s_RejectMargin & REJECTMARGINMASK;
}
/*!
  Determine if the trigger time subtraction is turned on in a
  Trigger configuration read from the device via 
  GetTriggerConfiguration

   \param Config	(TriggerConfiguration	IN)
	Trigger configuration returned by a call to 
	GetTriggerConfiguration.

    \return bool
    \retval true - trigger time subtraction is enabled.
    \retval false- trigger time subtraction is disabled.


 */
bool
CCAENV1x90::isTriggerTimeSubtracted(TriggerConfiguration Config)
{
  return ((Config.s_Subtracting & TRUEBIT) != 0);
}
/*!
  Sets the TDC resolution.   Note that only the 1290  is capable 
  of 25ps resolution and that this resolution is gained at the 
  cost of not being able to run in edge pair detetion mode.
  The m_fIsHiResMode member data is updated accordingly.

    \param nResolution	(Resolution	In)
	The new resolution.  This can be one of:
	- CCAENV1x90::Res_25ps 25 ps (only for the 1290).
	- CCAENV1x90::Res_100ps - 100ps
	- CCAENV1x90::Res_200ps - 200 ps.
	- CCAENV1x90::Res_800ps - 800 ps.

    \pre The resolution is valid.
    \pre If the resolution is Res_25ps, the module is
         Hires capable (m_fCanHiRes is true).


 */
void
CCAENV1x90::SetIndividualLSB(Resolution nResolution)
{

  // Check the preconditions:

  REQUIRE(((nResolution == Res_25ps)     ||
	   (nResolution == Res_100ps)    ||
	   (nResolution == Res_200ps)    ||
	   (nResolution == Res_800ps)), 
	  "Resolution is not a legal value");
  if(nResolution == Res_25ps) {
    REQUIRE(m_fCanHiRes, "Only 1290 can be put in 25ps resolution");
  }

  WriteMicro(CCAENV1x90Opcodes::SET_TR_LEAD_LSB);
  WriteMicro(nResolution);
  WaitMicroWrite();

  // Only the 1290 can have 25ps resolution.. the flag variable
  // m_fIsHiResMode is used to keep track of this as there are things
  // you give up to get that resolution (most notably pair mode).

  if(nResolution == Res_25ps) {
    m_fIsHiResMode = true;
  } else {
    m_fIsHiResMode = false;
  }

}


/*!
Sets the module edge detection mode.  Note that a 1290 in high
 resolution (25ps) mode cannot do EdgeMode_Pair and an exception 
is thrown if you attempt to turn on edge detection on  a module 
that is in that mode.

     \param nEdgeMode	(EdgeMode	in)
      The edge detection mode.  This can be one of:
      - CCAENV1x90::EdgeMode_Pair The width of a  pulse (rising/trailing edge) 
                      is measured.
      - CCAENV1x90::EdgeMode_Trailing Trailing edge detection enabled
      - CCAENV1x90::EdgeMode_Leading Leading edge detection enabled.
      - CCAENV1x90::EdgeMode_Both The time of both the leading and trailing 
                 edge of a pulse are digitized.


      \pre The nEdgeMode parameter is a valid member of the
          EdgeMode enum.
      \pre If the nEdgeMode parameter is EdgeMode_Pair, the module
           is not in hires mode.

 */
void
CCAENV1x90::SetEdgeDetectMode(EdgeMode nEdgeMode)
{
  // Check preconditions.

  REQUIRE( ((nEdgeMode == EdgeMode_Pair)           ||
	  (nEdgeMode == EdgeMode_Trailing)         ||
	  (nEdgeMode == EdgeMode_Leading)          ||
	  (nEdgeMode == EdgeMode_Both)),
	 "Invalid edge mode");
  if(m_fCanHiRes & m_fIsHiResMode) {
    REQUIRE(nEdgeMode != EdgeMode_Pair,
	  "Pair mode is not available in High resolution mode");
  }

  // Set the mode.

  WriteMicro(CCAENV1x90Opcodes::SET_DETECTION);
  WriteMicro(nEdgeMode);
  WaitMicroWrite();
}

/*!
  Returns the current edge detect trigger mode.

  \return EdgeMode
  \retval CCAENV1x90::EdgeMode_Pair -     Time between lead/trail edge
                             digitized.
  \retval CCAENV1x90::EdgeMode_Leading - Leading edge is detected/timed.
  \retval CCAENV1x90::EdgeMode_Trailing - Trailing edge is detected/timed.
  \retval CCAENV1x90::EdgeMode_Both     - Both leading and trailing edge detected
                              and timed.
 */
CCAENV1x90::EdgeMode
CCAENV1x90::GetEdgeDetectMode()
{
  WriteMicro(CCAENV1x90Opcodes::READ_DETECTION);
  uint16_t n = ReadMicro();
  n   &= EDGEDETECTIONMASK;			//  Only the bottom two bits have meaning
  return static_cast<EdgeMode>(n);

}
/*!
This function sets the pair mode resoulution of the device. 
Pair mode resolution consists of two values, the leading edge 
arrival time resolution and the pulse width resolution.  
It is not possible to set one without setting the other.


   \param nLEResolution	(LEResolution	IN)
   The desired leading edge resolution, one of :
   - CCAENV1x90::LE_100ps - 100ps 
   - CCAENV1x90::LE_200ps - 200ps
   - CCAENV1x90::LE_400ps - 400 ps.
   - CCAENV1x90::LE_800ps - 800 ps.
   - CCAENV1x90::LE_1600ps - 1.6ns.
   - CCAENV1x90::LE_3120ps - 3.12ns
   - CCAENV1x90::LE_6250ps - 6.25ns
   - CCAENV1x90::LE_12500ps - 12.5ns.

   \param nPWResolution	(PWResolution	IN)
   The desired pulse width resolution. 
   One of:
   - CCAENV1x90::PW_100ps - 100ps
   - CCAENV1x90::PW_200ps - 200ps
   - CCAENV1x90::PW_400ps - 400ps
   - CCAENV1x90::PW_800ps - 800ps
   - CCAENV1x90::PW_1600ps - 1.6ns
   - CCAENV1x90::PW_3200ps - 3.2ns
   - CCAENV1x90::PW_6250ps - 6.25ns
   - CCAENV1x90::PW_12500ps - 12.5ns
   - CCAENV1x90::PW_25ns   - 25ns
   - CCAENV1x90::PW_50ns  - 50 ns.
   - CCAENV1x90::PW_100ns - 100ns.
   - CCAENV1x90::PW_200ns - 200ns.
   - CCAENV1x90::PW_400ns - 400ns
   - CCAENV1x90::PW_800ns - 800ns.

\pre   - Module is pair mode.
\pre   - nLEResolution is a valid member of the LEResolution enum.
\pre   - nPWResolution is a valid member of the PWResolution enum.


*/
void
CCAENV1x90::SetPairResolutions(LEResolution nLEResolution,
			       PWResolution nPWResolution)
{
  // Check preconditions.

  REQUIRE((GetEdgeDetectMode() == EdgeMode_Pair),
	"Edge detection mode is not pair");
  REQUIRE( ((nLEResolution   == LE_100ps)     ||
	  (nLEResolution   == LE_200ps)     ||
	  (nLEResolution   == LE_400ps)     ||
	  (nLEResolution   == LE_800ps)     ||
	  (nLEResolution   == LE_1600ps)    ||
	  (nLEResolution   == LE_3120ps)    ||
	  (nLEResolution   == LE_6250ps)    ||
	  (nLEResolution   == LE_12500ps)),
	 "Leading edge resolution invalid");

  REQUIRE( ((nPWResolution  == PW_100ps)      ||
	  (nPWResolution  == PW_200ps)      ||
	  (nPWResolution  == PW_400ps)      ||
	  (nPWResolution  == PW_800ps)      ||
	  (nPWResolution  == PW_1600ps)     ||
	  (nPWResolution  == PW_3200ps)     ||
	  (nPWResolution  == PW_6250ps)     ||
	  (nPWResolution  == PW_12500ps)    ||
	  (nPWResolution  == PW_25ns)       ||
	  (nPWResolution  == PW_50ns)       ||
	  (nPWResolution  == PW_100ns)      ||
	  (nPWResolution  == PW_200ns)      ||
	  (nPWResolution  == PW_400ns)      ||
	  (nPWResolution  == PW_800ns)),
	 "Pair width resolution is invalid.");

  // Construct the settings word.

  uint16_t settings = nLEResolution | 
                            (nPWResolution << PWRESOLUTIONRSHIFT);


  // set the pair resolutions.
  WriteMicro(CCAENV1x90Opcodes::SET_PAIR_RES);
  WriteMicro(settings);
  WaitMicroWrite();

}
/*!
  Returns a word that describes the resolution of the device.  This word 
  can only be correctly interpreted knowing the trigger mode.  
  The resolution word can be interpreted via:
  - InterpretEdgeResolution - Interprets the resolution for the edge 
                              trigger modes.
  - InterpretLEResolution - Interpretes the leading edge resolution for
                            pair mode.
  - InterpretWidthResolution - Interprets the word as a width 
                            resolution for pair mode.

  \return uint16_t
  \retval the resolution information that can be passed to 
          interpretation functions.


 */
uint16_t
CCAENV1x90::GetResolution()
{
  WriteMicro(CCAENV1x90Opcodes::READ_RES);
  return ReadMicro();

}
/*!
  Returns an encoded resolution. The module must be in one of the edge 
  triggering modes to call this function.

 
   \param nResolution	(uint16_t	IN)
   The resolution word returned from GetResolution

   \return Resolution
   \retval CCAENV1x90::Res_25ps  - Module is in 25ps resolution (1290 only).
   \retval CCAENV1x90::Res_100ps - Module is at 100ps resolution.
   \retval CCAENV1x90::Res_200ps - Module is at 200ps resolution.
   \retval CCAENV1x90::Res_800ps - Module is at 800ps resolution.

   \pre Module may not be in pair mode.

*/
CCAENV1x90::Resolution
CCAENV1x90::InterpretEdgeResolution(uint16_t nResolution)
{
  // Check preconditions:

  REQUIRE(GetEdgeDetectMode() != EdgeMode_Pair,
	"Edge resolution asked for when module in pair mode");

  //
  
  uint16_t nMask = nResolution & 
                         EDGERESOLUTIONMASK; // Mask off resolution and
  return static_cast<Resolution>(nMask); // Return as resolution 
}
/*!
  Returns the encoded representation of the leading edge resolution 
  decoded when the module is in pair mode.  The module must still be 
  in pair mode to call this function.

     \param nResolution	(uint16_t	IN)
     The resolution word returned from GetResolution

     \return LEResolution
     \retval CCAENV1x90::LE_100ps    Leading edge resolution is 100ps
     \retval CCAENV1x90::LE_200ps    Leading edge resolution is 200ps
     \retval CCAENV1x90::LE_400ps    Leading edge resolution is 400ps
     \retval CCAENV1x90::LE_800ps    Leading edge resolution is 800ps
     \retval CCAENV1x90::LE_1600ps    Leading edge resolution is 1600ps
     \retval CCAENV1x90::LE_3120ps    Leading edge resolution is 3120ps
     \retval CCAENV1x90::LE_6250ps    Leading edge resolution is 6250ps
     \retval CCAENV1x90::LE_12500ps    Leading edge resolution is 12500ps

     \pre Module must be in pair mode.

 */
CCAENV1x90::LEResolution
CCAENV1x90::InterpretLEResolution(uint16_t nResolution)
{
  // Check preconditions:

  REQUIRE(GetEdgeDetectMode() == EdgeMode_Pair,
	"LE resolution requested with module not in pair mode"); 

  // 

  uint16_t nMask = nResolution & LERESOLUTIONMASK;
  return static_cast<LEResolution>(nMask);
    
}
/*!
  When the module is in pair mode, this function can be called to 
  return the pulse width resolution encoded as a PWResolution value.   
  If the module is not in pair mode, the function throws an exception.

    \param nResolution	(uint16_t	IN)
	The resolution word returned from GetResolution

    \return PWResolution
    \retval CCAENV1x90::PW_100ps   - 100ps  pulse width resolution.
    \retval CCAENV1x90::PW_200ps   - 200ps  pulse width resolution.
    \retval CCAENV1x90::PW_400ps   - 400ps  pulse width resolution.
    \retval CCAENV1x90::PW_800ps   - 800ps  pulse width resolution.
    \retval CCAENV1x90::PW_1600ps  - 1600ps  pulse width resolution.
    \retval CCAENV1x90::PW_3200ps  - 3200ps  pulse width resolution.
    \retval CCAENV1x90::PW_6250ps  - 6250ps  pulse width resolution.
    \retval CCAENV1x90::PW_12500ps - 12500ps  pulse width resolution.
    \retval CCAENV1x90::PW_25ns    - 25ns  pulse width resolution.
    \retval CCAENV1x90::PW_50ns    - 50ns  pulse width resolution.
    \retval CCAENV1x90::PW_100ns   - 100ns  pulse width resolution.
    \retval CCAENV1x90::PW_200ns   - 200ns  pulse width resolution.
    \retval CCAENV1x90::PW_400ns   - 400ns  pulse width resolution.
    \retval CCAENV1x90::PW_800ns   - 800ns  pulse width resolution.

    \pre    Module must be in pair mode.


 */
CCAENV1x90::PWResolution
CCAENV1x90::InterpretWidthResolution(uint16_t nResolution)
{
  // Check the precondition:

  REQUIRE(GetEdgeDetectMode() == EdgeMode_Pair,
	"PW resolution requested when not in pair mode");

  //
  uint16_t nMask = (nResolution & PWRESOLUTIONMASK) >> 
                          PWRESOLUTIONRSHIFT;
  return static_cast<PWResolution>(nMask);
}
/*!
  Sets the dead time between hits.  This is the effective hit-to-hit 
  resolution of the device.

   \param nDeadTime	(DeadTime	IN)
   Dead time between hits.  This can be one of:
   - CCAENV1x90::DT_5ns   - 5 ns.
   - CCAENV1x90::DT_10ns  - 10ns
   - CCAENV1x90::DT_30ns  - 30 ns
   - CCAENV1x90::DT_100ns - 100 ns.

   \pre nDeadTime parameter is a legal member of the DeadTime
        enum.

 */
void
CCAENV1x90::SetDoubleHitResolution(DeadTime nDeadTime) 
{ 
  // Check the preconditions

  REQUIRE((nDeadTime == DT_5ns)         ||
	(nDeadTime == DT_10ns)        ||
	(nDeadTime == DT_30ns)        ||
	(nDeadTime == DT_100ns),
	"Invalid dead time value");
  //
  WriteMicro(CCAENV1x90Opcodes::SET_DEAD_TIME);
  WriteMicro(nDeadTime);
  WaitMicroWrite();

}
/*!
  Returns the hit-to-hit dead time for the module.


  \return DeadTime 
  \retval CCAENV1x90::DT_5ns   -   5ns hit-to-hit resolution.
  \retval CCAENV1x90::DT_10ns  -  10ns hit-to-hit resolution.
  \retval CCAENV1x90::DT_30ns  -  30ns hit-to-hit resolution.
  \retval CCAENV1x90::DT_100ns - 100ns hit-to-hit resolution.

 */
CCAENV1x90::DeadTime
CCAENV1x90::GetDoubleHitResolution()
{
  WriteMicro(CCAENV1x90Opcodes::READ_DEAD_TIME);
  uint16_t nMask = ReadMicro();
  nMask &= 3;
  return static_cast<DeadTime>(nMask);

}
/*!
  Enables the module to encapsulate the data from a single tdc between
  a header and trailer word produced by the TDC chip.
*/
void
CCAENV1x90::EnableTDCEncapsulation()
{
  WriteMicro(CCAENV1x90Opcodes::EN_HEADER_TRAILER);
  WaitMicroWrite();
}

/*!
  Disables the encapsulation of data from a single TDC chip within 
  headers and trailers.
 */
void
CCAENV1x90::DisableTDCEncapsulation()
{
  WriteMicro(CCAENV1x90Opcodes::DIS_HEADER_TRAILER);
  WaitMicroWrite();
}
/*!
  Determines if the device has TDC data encapsulation enabled.
  \return bool
  \retval If the device is encapsulating the data from a single 
          TDC in chip header/trailer pairs, returns true.
  \retval Otherwise, returns false.

 */
bool
CCAENV1x90::isTDCEncapsulationOn()
{
  WriteMicro(CCAENV1x90Opcodes::READ_HEADER_TRAILER);
  return (ReadMicro() & TRUEBIT) != 0;

}
/*!
  Sets the maximum number of hits a tdc chip will contribute to an 
  event (Trigger match mode?).  If the number of hits is exceeded, the 
  TDC will assert a size limit exceeded condition which, if error 
  flags are enabled, will result in an error flag word being placed in 
  the output event.

   \param nHits	(HitMax	IN)	
   This can be one of:
   -	CCAENV1x90::HITS_0 - no hits.
   -	CCAENV1x90::HITS_1 - 1 hit.
   -	CCAENV1x90::HITS_2 - 2 hits
   -	CCAENV1x90::HITS_4 - 4 hits
   -	CCAENV1x90::HITS_8 - 8 hits
   -	CCAENV1x90::HITS_16 - 16 hits
   -	CCAENV1x90::HITS_32 - 32 hits
   -	CCAENV1x90::HITS_64 - 64 hits
   -	CCAENV1x90::HITS_128 - 128 hits
   -	CCAENV1x90::HITS_UNLIMITED - no  limit

   \pre nHits parameter is a valid member of the HitMax enum.
*/
void
CCAENV1x90::SetMaxHitsPerEvent(HitMax nHits)
{
  // Check validity of nHits parameter (precondition)

  REQUIRE((nHits == HITS_0)              ||
	(nHits == HITS_1)              ||
	(nHits == HITS_2)              ||
	(nHits == HITS_4)              ||
	(nHits == HITS_8)              ||
	(nHits == HITS_16)             ||
	(nHits == HITS_32)             ||
	(nHits == HITS_64)             ||
	(nHits == HITS_128)            ||
	(nHits == HITS_UNLIMITED),
	"Invalid nHits parameter value");

  //  Set the device:

  WriteMicro(CCAENV1x90Opcodes::SET_EVENT_SIZE);
  WriteMicro(static_cast<uint16_t>(nHits));
  WaitMicroWrite();
}
/*!
  Return the maximum hit count set via SetMaxHitsPerEvent.

  \return HitMax
  \retval	CCAENV1x90::HITS_0 - no hits.
  \retval 	CCAENV1x90::HITS_1 - 1 hit.
  \retval 	CCAENV1x90::HITS_2 - 2 hits
  \retval	CCAENV1x90::HITS_4 - 4 hits
  \retval	CCAENV1x90::HITS_8 - 8 hits
  \retval	CCAENV1x90::HITS_16 - 16 hits
  \retval	CCAENV1x90::HITS_32 - 32 hits
  \retval	CCAENV1x90::HITS_64 - 64 hits
  \retval	CCAENV1x90::HITS_128 - 128 hits
  \retval	CCAENV1x90::HITS_UNLIMITED - no  limit

 */
CCAENV1x90::HitMax
CCAENV1x90::GetMaxHitsPerEvent()
{
  WriteMicro(CCAENV1x90Opcodes::READ_EVENT_SIZE);
  uint16_t nMask = ReadMicro();
  return static_cast<HitMax>(nMask);
}

/*!
  Enables the production of error mark words in the event whenever a 
  global error occurs on an event.
 
*/
void
CCAENV1x90::EnableErrorMark()
{
  WriteMicro(CCAENV1x90Opcodes::EN_ERROR_MARK);
  WaitMicroWrite();
}
/*!
  Disables the production of error mark words in the event when a 
  global error occurs.
 */
void
CCAENV1x90::DisableErrorMark()
{
  WriteMicro(CCAENV1x90Opcodes::DIS_ERROR_MARK);
  WaitMicroWrite();
}
/*!
  Allows the TDC to disable its readout on the existence of a global 
  error condition.
 */
void
CCAENV1x90::EnableBypassOnError()
{
  WriteMicro(CCAENV1x90Opcodes::EN_ERROR_BYPASS);
  WaitMicroWrite();
}
/*!
  Allows the TDC to be readout in the presence of a global error 
  condition.
 */
void
CCAENV1x90::DisableBypassOnError()
{
  WriteMicro(CCAENV1x90Opcodes::DIS_ERROR_BYPASS);
  WaitMicroWrite();
}
/*!
  Determines the set of conditions detected in side the TDC that 
  result in a global error.  The parameter is a bitwise or of the 
  legal values.  Note that in addition to the errors that can be 
  controlled, there's also the size error bit which cannot be 
  controlled, and indicates the TDC has more hits than allowed.

   \param nErrors	(uint16_t	IN)	
   A bitwise or of the conditions that you want to produce a global 
   error condition.  The legal bits are:
   -	CCAENV1x90::ERR_VERNIER - vernier error, DLL unlocked or excessive jitter 
                      detected).
   -	CCAENV1x90::ERR_COARSE - Parity error on coarse count readout
   -	CCAENV1x90::ERR_SELECT - Channel select (Synchronization) error.
   -	CCAENV1x90::ERR_L1PARITY - Level 1 buffer parity error.
   -	CCAENV1x90::ERR_TFIFOPARITY - Trigger fifo parity error.
   -	CCAENV1x90::ERR_MATCHERROR - Trigger matching errro (state error)
   -	CCAENV1x90::ERR_RFIFOPARITY - Readou t fifo parity error.
   -	CCAENV1x90::ERR_RDOSTATE - Readout state error.
   -	CCAENV1x90::ERR_SUPPARITY - Setup parity error.
   -	CCAENV1x90::ERR_CTLPARITY - control parity error.
   -	CCAENV1x90::ERR_JTAGPARITY - jtag parity error.

   \pre No extraneous bits may be set in the nErrors mask.

 */
void
CCAENV1x90::SetErrorEnables(uint16_t nErrors)
{
  // Check the preconditions..

  REQUIRE((nErrors & ~( ERR_VERNIER         &
		      ERR_SELECT          &
		      ERR_L1PARITY        &
		      ERR_TFIFOPARITY     &
		      ERR_MATCHERROR      &
		      ERR_RFIFOPARITY     &
		      ERR_RDOSTATE        &
		      ERR_SUPPARITY       &
		      ERR_CTLPARITY       &
		      ERR_JTAGPARITY)),
	"Error mask contains extra set bits.");

  // Set the device.

  WriteMicro(CCAENV1x90Opcodes::SET_ERROR_TYPES);
  WriteMicro(nErrors);
  WaitMicroWrite();
}

/*!
Returns the mask of TDC error bits that are enabled..

See CCAENV1x90::SetErrorEnables for more information about the
valid bits that may be set in the mask.
 */
uint16_t
CCAENV1x90::GetErrorEnables()
{
  WriteMicro(CCAENV1x90Opcodes::READ_ERROR_TYPES);
  return ReadMicro();
}


/*!
  Sets the level 1 buffer fifo size. in words.


    \param nL1Size	(L1Size	IN)
	the level 1 buffer size.  This can be one of:
	-	CCAENV1x90::L1_2wds   - 2 words
	-	CCAENV1x90::L1_4 wds  - 4 words
	-	CCAENV1x90::L1_8wds   - 8 words
	-	CCAENV1x90::L1_16wds  - 16 words
	-	CCAENV1x90::L1_32wds  - 32 words.
	-	CCAENV1x90::L1_64wds  - 64 words.
	-	CCAENV1x90::L1_128wds - 128 words.
	-	CCAENV1x90::L1_256wds -256 words.

    \pre nL1Size i a valid member of L1Size.

*/
void
CCAENV1x90::SetL1Size(L1Size nL1Size)
{
  // Check precondition:

  REQUIRE((nL1Size   == L1_2wds)       ||
	(nL1Size   == L1_4wds)       ||
	(nL1Size   == L1_8wds)       ||
	(nL1Size   == L1_16wds)      ||
	(nL1Size   == L1_32wds)      ||
	(nL1Size   == L1_64wds)      ||
	(nL1Size   == L1_128wds)     ||
	(nL1Size   == L1_256wds),
	"L1 size selector is invalid");

  //
  
  WriteMicro(CCAENV1x90Opcodes::SET_FIFO_SIZE);
  WriteMicro(nL1Size);
  WaitMicroWrite();
  

}
/*!
 Returns the size of the L1 buffer fifo.  See CCAENV1x90::SetL1size 
 for more information about the L1Size enumerator and its legal 
 values.
 */
CCAENV1x90::L1Size
CCAENV1x90::GetL1Size()
{
  WriteMicro(CCAENV1x90Opcodes::READ_FIFO_SIZE);
  return static_cast<L1Size>(ReadMicro());
}

/*!
  Enables a single TDC channel.   The actual number of channels 
  supported by a TDC Chip, is determined by the model number of the 
  device.  If an invalid chip or channel is selected, an exception 
  will be thrown.


      \param nChannel	(uint32_t	IN)
	Number of the channel to enable.

      \pre nChannel >= 0
      \pre nChannel <= m_nChannels



 */
void
CCAENV1x90::EnableChannel(uint16_t nChannel)
{
  REQUIRE(nChannel < m_nChannels, "Channel number out of range");

  //
  WriteMicro(CCAENV1x90Opcodes::EN_CHANNEL | nChannel);
  WaitMicroWrite();
}
/*!
  Disables a single TDC channel.   The actual number of channels 
  supported by a TDC Chip, and the number of chips is determined by 
  the model number of the device.  If an invalid chip or channel is 
  selected, an exception will be thrown.

    \param nChannel	(uint32_t	IN)
    Number of the channel to enable.

    \pre  nChannel >= 0.
    \pre  nChannel < m_nChannels.

 */
void
CCAENV1x90::DisableChannel(uint16_t nChannel)
{
  REQUIRE(nChannel < m_nChannels, "Channel Number out of range");

  //

  WriteMicro(CCAENV1x90Opcodes::DIS_CHANNEL | nChannel);
  WaitMicroWrite();

}

/*!
  Enables all of the channels in all of the chips on the module.
 */
void
CCAENV1x90::EnableAllChannels()
{
  WriteMicro(CCAENV1x90Opcodes::EN_ALL_CH);
  WaitMicroWrite();
}
/*!
  Dissables all the channels in all the chips on the module.
 */
void
CCAENV1x90::DisableAllChannels()
{
  WriteMicro(CCAENV1x90Opcodes::DIS_ALL_CH);
  WaitMicroWrite();
}
/*!
  This function enables all channels that have corresponding bits set 
  in nMask.The actual number of bits in the mask will depend on the 
  model and variant.  


      \param nMask	(vector<uint16_t>	IN)
      The mask of channels to enable.

   \pre nMask.size() == m_nChannels/(sizeof(uint16_t)* 8)
  
 */
void
CCAENV1x90::SetChannelEnables(vector<uint16_t> nMask)
{
  // Check the mask is the proper size:

  REQUIRE(nMask.size() == m_nChannels/(sizeof(uint16_t)*8),
	"Enables mask is not the right size.");

  // Marshall the vector into an array so we can call WriteMicroBlock

  uint16_t* pWords = new uint16_t[nMask.size() + 1];

  pWords[0] = CCAENV1x90Opcodes::WRITE_EN_PATTERN;
  for(int i =0; i < nMask.size(); i++) {
    pWords[i+1] = nMask[i];
  }

  WriteMicroBlock(pWords, nMask.size()+1);
  WaitMicroWrite();

  delete []pWords;

}
/*!
  This function reads the current channel enable mask.    
  The actual number of bits that will be transferred depends on 
  the module type and variant.


   \param nMask	(vector<uint16_t>& OUT)
   	Mask of enable bits.  Note that the bits are appended
        to the vector so it is best to be sure the vector is empty
	before passing it into this function.  The vector is not
	cleared by this function.



 */
void
CCAENV1x90::GetChannelEnables(vector<uint16_t>& masks)
{
  uint32_t nMasks = m_nChannels/(sizeof(uint16_t)*8);

  uint16_t* localMasks = new uint16_t[nMasks];

  MicroTransaction(CCAENV1x90Opcodes::READ_EN_PATTERN,
		   localMasks, nMasks);

  for(int i =0; i < nMasks; i++) {
    masks.push_back(localMasks[i]);
  }
  delete []localMasks;

}

/*!
  Sets the enables for a single chip of the module as designated 
  by the mask longword.  Each channel that has a corresponding 
  bit set is enabled, the others disabled. 

   \param nChip	(uint32_t	IN)
   Chip number.
   \param nMasks (uint32_t 	IN)
   Mask of enable bits.

   \pre nChip must be < m_nChipCount (chips are numbered from 0).


 
*/
void
CCAENV1x90::SetChipEnables(uint16_t nChip,
			   uint32_t nMask)
{
  // Check the preconditions:

  REQUIRE(nChip < m_nChipCount,
	"Chip number is not valid for this model");
  
  //  Issue the opcode.  Note that nMask must be busted
  //  apart into a low and high word that get written to the
  //  micro.
  
  uint16_t wordMask;

  WriteMicro(CCAENV1x90Opcodes::WRITE_EN_PATTERN32 | nChip);

  wordMask = LO(nMask);
  WriteMicro(wordMask);

  wordMask = HI(nMask);
  WriteMicro(wordMask);
  WaitMicroWrite();

}

/*!
  Returns the channel enable mask for a TDC Chip. The mask will 
  have a bit set for each enabled channel.  

   \param nChip	(uint32_t	IN)
   Chip number.

   \return uint32_t
   \retval The 32 bit mask of channel enables.  Channel 0 of the
   chip will be the LSBit. while channel 31 the MSBit.

   \pre nChip < m_nChipCount


*/
uint32_t
CCAENV1x90::GetChipEnables(uint16_t nChip)
{
  // Check preconditions:

  REQUIRE(nChip < m_nChipCount,
	  FormatValue("Invalid chip number %d",
				    nChip).c_str());

  // Fetch the mask out in two chunks.  The first
  // word has the low order channels, the second, the high:

  uint32_t mask;

  WriteMicro(CCAENV1x90Opcodes::READ_EN_PATTERN32 | nChip);

  mask = (static_cast<uint32_t>(ReadMicro())) & 0xffff;
  uint32_t top = ReadMicro();
  mask |= (top << 16);


  return mask;

}
/*!
  This function sets a global offset on the coarse and fine 
  counter values.  The coarse counter offset range is 0-2047, 
  while the fine offset is 0-31.

   \param nCoarse  (uint16_t	IN)
   Coarse offset.
   \param nVernier (uint16_t	IN)
	Fine (vernier) offset

   \pre nCoarse  < 0x1000
   \pre nVernier < 0x20



 */
void
CCAENV1x90::SetGlobalOffset(uint16_t nCoarse,
			    uint16_t nVernier)
{
  // Check the preconditions:

  REQUIRE(nCoarse <= COARSEMAX, 
	  FormatValue("Global offset coarse value %d",
				    nCoarse).c_str());
  REQUIRE(nVernier <= VERNIERMAX,
	  FormatValue("Vernier offset is invalid: %d",
				    nVernier).c_str());

  // Set the values:

  WriteMicro(CCAENV1x90Opcodes::SET_GLOB_OFFS);
  WriteMicro(nCoarse);
  WriteMicro(nVernier);
  WaitMicroWrite();

}

/*!
  Reads the coarse and fine offsets from the module.  These 
  offsets are masked so that there will be no bits set other than 
  those set in the offset fields.

   \param nCoarse (uint32_t&	OUT)
   The coarse offset read from the chip.
   \param nVernier (uint32_t&	OUT)
   The fine offset read from the chip.


*/
void
CCAENV1x90::ReadGlobalOffset(uint16_t& nCoarse,
			     uint16_t& nVernier)
{
  WriteMicro(CCAENV1x90Opcodes::READ_GLOB_OFFS);
  nCoarse = (ReadMicro()  & COARSEMASK);
  nVernier= (ReadMicro()  & VERNIERMASK);

}

/*!
  Each TDC channel can have an offset applied to it.  This 
  function sets the offset for an individual TDC channel.

    \param nChannel (uint32_t	IN)
    channel number
    \param nOffset (uint16_t	IN)
    Offset in the range [0-255].

    \pre nChannel is a legal channel number for this module type.
    \pre nOffset is i < 0x100.



 */
void 
CCAENV1x90::SetChannelOffset(uint16_t nChannel,
			     uint16_t nOffset)
{
  // Check the preconditions:

  REQUIRE(nChannel < m_nChannels,
	" Channel number out of range");
  REQUIRE(nOffset  <= CHANOFFSETMAX,
	  FormatValue("Offset value out of range: %d",
				    nOffset).c_str());

  // Set the micro:

  WriteMicro(CCAENV1x90Opcodes::SET_ADJUST_CH | nChannel);
  WriteMicro(nOffset);
  WaitMicroWrite();
}
/*!
  Returns the offset value that was set for a channel of the TDC 
  via SetChannelOffset.

    \param nChannel (uint32_t	IN)
    Channel Number.

    \return uint16_t
    \retval The channel offset in the range [0,0xff].

    \pre nChannel < m_nChannels;


 */
uint16_t
CCAENV1x90::GetChannelOffset(uint32_t nChannel)
{
  // check the preconditions:

  REQUIRE(nChannel < m_nChannels,
	"Channel number out of range");

  // Get and return the value:

  WriteMicro(CCAENV1x90Opcodes::READ_ADJUST_CH | nChannel);
  return (ReadMicro() & CHANOFFSETMASK);
}
/*!
  Each tap on the delay line has a set of three additional 
  capacitors that can be gated into the circuit.  These 
  capacitors affect the exact timing of the delay line tap and 
  can therefore be used to calibrate each tap.  This function 
  sets the contact masks for all 12 capacitors (3 contacts per 4 
  delay line taps) in the delay line.


    \param nChip	(Uint16_T	In)
    TDC chip number.
    \param Tap1Contact	(uint16_t	IN)
    Contact mask for tap 1.  For each bit in the contact mask, an 
    additional capacitance is added to the Tap1Contact RC delay.  
    Valid mask bits are:
    - CCAENV1x90::TAP_CONTACT1
    - CCAENV1x90::TAP_CONTACT2
    - CCAENV1x90::TAP_CONTACT3
    \param Tap2Contact	Uint16_T	IN	
    Contact mask for tap2
    \param Tap3Contact	Uint16_T	IN	
    Contact mask for tap 3
    \param Tap4Contact	Uint16_T	IN	
    contact mask for tap 4.

    \pre Tap or contact mask only has the bits sest described
         above.
    \pre nChip < m_nChipCount



 */
void
CCAENV1x90::CalibrateDelayLine(uint16_t nChip,
			       uint16_t Tap1Contact,
			       uint16_t Tap2Contact,
			       uint16_t Tap3Contact,
			       uint16_t Tap4Contact)
{
  // check the preconditions:

  REQUIRE(nChip < m_nChipCount,
	"Chip number is not in range");

  REQUIRE((Tap1Contact & ~(TAP_CONTACT1 | TAP_CONTACT2 | TAP_CONTACT3)) == 0,
	" Tap 1 contact has extra bits"); 
  REQUIRE((Tap2Contact & ~(TAP_CONTACT1 | TAP_CONTACT2 | TAP_CONTACT3)) == 0,
	" Tap 1 contact has extra bits");
  REQUIRE((Tap3Contact & ~(TAP_CONTACT1 | TAP_CONTACT2 | TAP_CONTACT3)) == 0,
	" Tap 1 contact has extra bits");
  REQUIRE((Tap4Contact & ~(TAP_CONTACT1 | TAP_CONTACT2 | TAP_CONTACT3)) == 0,
	" Tap 1 contact has extra bits");

  // Set the delay line calibration.
  
  uint16_t TapMask = (Tap1Contact) |
                           (Tap2Contact << 3) |
                           (Tap3Contact << 6) |
                           (Tap4Contact << 9);


  WriteMicro(CCAENV1x90Opcodes::SET_RC_ADJ | nChip);
  WriteMicro(TapMask);
  WaitMicroWrite();
}
/*!
     Returns the contact masks for the delay line calibrations 
     for a single TDC on the board.

   \param nChip	(uint16_t	IN)
   The TDC chip number.
   \param Tap1Contact	(uint16_t &	OUT)
   Tap 1 contact mask See SetDelayLineCalibration for information 
   about the bits in this mask
   \param Tap2Contact	(uint16_t &	OUT)
   Tap 2 contact mask.
   \param Tap3Contact	(uint16_t &	OUT)
   Tap 3 contact mask
   \param Tap4Contact	(uint16_t & 	OUT)
   Tap 4 contact mask.

   \pre nChip  < m_nChipCount


 */
void
CCAENV1x90::GetDelayLineCalibration(uint16_t  nChip,
				    uint16_t& Tap1Contact,
				    uint16_t& Tap2Contact,
				    uint16_t& Tap3Contact,
				    uint16_t& Tap4Contact)
{
  WriteMicro(CCAENV1x90Opcodes::READ_RC_ADJ | nChip);
  uint16_t TapMask = ReadMicro();

  Tap1Contact = (TapMask & TAPCONTACTMASK);
  Tap2Contact = (TapMask >> TAPCONTACTRSHIFT) & TAPCONTACTMASK;
  Tap3Contact = (TapMask >> (2*TAPCONTACTRSHIFT)) & TAPCONTACTMASK;
  Tap4Contact = (TapMask >> (3*TAPCONTACTRSHIFT)) & TAPCONTACTMASK;

}
/*!
  The current set of delay line calibration contact masks are 
  saved to EEPROM on the module.  On power up, these settings 
  will be restored to all TDC chips.

 */
void
CCAENV1x90::SaveDelayLineCalibrations()
{
  WriteMicro(CCAENV1x90Opcodes::SAVE_RC_ADJ);
  WaitMicroWrite();

}
/*!
  Returns the chip ID and stepping leve of the specified 
  TDC chip.   This should be a hex number like: 0x8470dacx  
  where x is the stepping number.  The stepping number is not 
  encoded in any way I can understand.  The manual claims that 
  0xe is revision 1.3 (e = 14 so I don't get it).  

    \param nChip	(uint16_t	IN)
    Chip number.

    \return uint32_t
    \retval the chip id. See the description.

    \pre nChip < m_nChipCount


*/
uint32_t
CCAENV1x90::GetChipId(uint16_t nChip)
{
  REQUIRE(nChip < m_nChipCount,
	"Chip number invalid for this board");


  WriteMicro(CCAENV1x90Opcodes::READ_TDC_ID | nChip);

  // 16 Is not a magic number.. It's the number of bits in a word.
  
  uint32_t nId = ReadMicro();
  nId             |= (static_cast<uint32_t>(ReadMicro())) << 16;
  return nId;
}


/*!
  Retrieves information about the firmware revision and when it 
  was comitted from the board.

  \param nRevision (uint32_t&	OUT)
  The firmware revision level
  \param nDay	(uint32_t&	OUT)
  Day of the month on which the firmware was comitted.
  \param (nMonth	uint32_t&	OUT)
  Month of the year on which the firmware was comitted.
  \param nYear	(uint32_t &	OUT)
  Year on which the firmware was committed.

 */
void
CCAENV1x90::GetuCFirmwareInfo(uint16_t& nRevision,
			      uint16_t& nDay,
			      uint16_t& nMonth,
			      uint16_t& nYear)
{
  WriteMicro(CCAENV1x90Opcodes::REV_DATE_MICRO_FW);
  nRevision = ReadMicro();
  nDay      = ReadMicro();
  nMonth    = ReadMicro();
  nYear     = ReadMicro();
}

/*!
  Returns the error mask from the specified TDC chip.


   \param nChip	(uint32_t	IN)
   The chip to read.

 
   \pre nChip is invalid for the board.


 */
uint16_t
CCAENV1x90::GetChipErrors(uint16_t nChip)
{
  REQUIRE(nChip < m_nChipCount,
	"Chip number is out of range for this module");

  WriteMicro(CCAENV1x90Opcodes::READ_ERROR_STATUS | nChip);

  uint16_t nErrors = ReadMicro();

  nErrors &= ~(ERR_VERNIER     |
	       ERR_COARSE      |
	       ERR_SELECT      |
	       ERR_L1PARITY    |
	       ERR_TFIFOPARITY |
	       ERR_MATCHERROR  |
	       ERR_RFIFOPARITY |
	       ERR_RDOSTATE    |
	       ERR_SUPPARITY   |
	       ERR_CTLPARITY   |
	       ERR_JTAGPARITY);
  return nErrors;
}
/*!
  Requests that the board  run in test mode.  This is 
  only useful in trigger matching mode.  For each gate the tdc 
  will emit (between headers and trailers if enabled) n test words 
  as described in figure 5.2 of the hardware manual.   n is the 
  maximum event size programmed for the tdc.  I'm not sure what 
  it'll do if that's set to unlimited :-).

    \param nValue (uint32_t	IN)
    The 24 bits of data value to set for this TDC chip's data.

    \pre nValue & ~(0xffffff)  == 0 (Data word fits in 24 bits).

 
*/
void
CCAENV1x90::EnableTestMode(uint32_t  nValue)
{
  // Validate preconditions:

  REQUIRE((nValue & ~(MAXTESTVALUE)) == 0,
	"The data value has extraneous bits");

  // Break up thed data longword into hi lo words:
  //  0xffff is just a word with all bits set., 16 the # bits in a word

  uint16_t nLow   = LO(nValue);
  uint16_t nHigh  = HI(nValue);

  WriteMicro(CCAENV1x90Opcodes::ENABLE_TEST_MODE);
  WriteMicro(nLow);
  WriteMicro(nHigh);
  WaitMicroWrite();

}

/*!
  Takes the board  out of test mode.  Test mode is enabled for a 
  chip via a call to EnableTestMode.
 */
void
CCAENV1x90::DisableTestMode()
{
  WriteMicro(CCAENV1x90Opcodes::DISABLE_TEST_MODE);
  WaitMicroWrite();
}

////////////////////////// Event readout functions ////////////////

/*!
  The action depends on the TDC module trigger mode:
  - Trigger Matching mode: calls ReadPacket to process an entire 
    data packet from the TDC.
  - Continuous Storage mode: calls ReadValid

  Please see the description of both of these functions for more information.

    \param pBuffer	(void*	OUT)
    Where to put the data.
    \param nMaxLongs	(uint32_t	IN)
    Maximum number of Longs to read. The pBuffer parameter must point 
    to storage of at least nMaxLongs*sizeof(long)  bytes of data.

    \return uint32_t 
    \retval actual number of Longs


 */
uint32_t
CCAENV1x90::ReadData(void* pBuffer,
		     uint32_t nMaxLongs)
{
  if(m_fTriggerMatching) {
    return ReadPacket(pBuffer, nMaxLongs);
  } else {
    return ReadValid(pBuffer, nMaxLongs);
  }
}
/*!
  Reads a data packet that represents an event from the TDC. 
  This is done by reading the TDC word by word and looking for an
  Event trailer.  I suggest that the caller use the Event FIFO to 
  determine how big the event is and allocate an appropriately sized 
  buffer for it.  If the event is larger than nMaxLongs, longs will
  be read from the TDC until the end of event trailer is located.  
  The user buffer will consist of the first nMaxLong-1 data items, 
  a bunch of skipped longs and a final long that will be the trailer 
  long.  In that case, the return value of ReadPacket will be 
  nMaxLongs as well.  The caller code can detect this by a mismatch 
  between nMaxLongs (the return value) and the size of he event as 
  described in the trailer longword and take appropriate action.  
  This should, of course never happen if you paid attention and 
  used the event FIFO to set the buffersize and nMaxLongs.


   \param pBuffer    (void*	OUT)
   Pointer to the buffer into which the data are read.
   \param nMaxLongs  (uint32_t	IN)
   Maximum number of longs to read into the buffer.

   \return int 
   \retval Number of longs actually read. 

*/
uint32_t
CCAENV1x90::ReadPacket(void* pBuffer,
		       uint32_t nMaxLongs)
{
  uint32_t* pPacket = (uint32_t*)pBuffer;
  uint32_t   nRead   = 0;

  // Loop over the read until we get a data word that has
  // either the TDC_TRAILER or FILLER_LONG type  mask value.
  // Note that if we run out of user buffer we just stop incrementing
  // the counter/pointer.

  bool done(false);
  while(!done) {
    uint32_t datum = m_pRegisters.peekl(OffsetL(OutputBuffer));
    *pPacket            = datum;
    if(isGlobalTrailer(datum) || isFiller(datum)) {
      done = true;		// Last read.
    }
    if(nRead < nMaxLongs) {	// overwrite last word if output full.
      pPacket++;
      nRead++;
    }
  }
  
  return nRead;
}
/*!
  LongWords are read from the TDC data buffer until either nMaxLongs 
  longwords have been read, or alternatively, until the first pad 
  longword is encountered.  Pad longs are not put in the buffer.

      \param pBuffer	(void*	OUT)
      Pointer to the buffer into which the TDC data will be read.
      \param nMaxLongs	(uint32_t	IN)
      Number of longwords in the buffer.

      \return uint32_t 
      \retval The number of longwords placed into the buffer.

 */
uint32_t 
CCAENV1x90::ReadValid(void* pBuffer, uint32_t nMaxLongs)
{
  uint32_t* pLongs = (uint32_t*)pBuffer;
  uint32_t   nRead  = 0;
  bool           done   = false;

  while(!done) {
    uint32_t datum = m_pRegisters.peekl(OffsetL(OutputBuffer));
    *pLongs++          = datum;
    nRead++;

    if(isGlobalTrailer(datum) ||
       isFiller(datum)    ||
       (nRead == nMaxLongs)) {
      done = true;
    }
  }

  return nRead;
}

////////////////////////// Private utilities //////////////////////

/*!

Determines if the module is a module supported by this class.
- Read the board type and ensure that it is a 1190 or a 1290.
- Look at the version.  If the board is a 1190, this must be 
  A or B if the board is a 1290 it must be A or N.
- The manufacturer code must be 0x0040e6 (CAEN).


  \param pROM	(CVMEptr	IN)	
     pointer like object to the module's configuration rom.

   \return bool
   \retval	true -indicates the module is a V1x90 tdc
   \retval	false - indicates the module is not a V1x90 tdc.

*/
bool
CCAENV1x90::ValidBoard(CVmeModule& pRom)
{
  // Check the manufacturer's code (OUI):
  
  uint32_t oui = ReadPromLong(pRom, WOUI2); // Read oui from prom.
  if(oui != 0x0040e6) {
    return false;
  }

  // Check the board version:

  uint32_t nModel = ReadPromLong(pRom, WModelNumber2);
  if((nModel != 1190) && (nModel != 1290)) {
    return false;
  }

  // 1190's can be either type a or b, 1290's must be a or n

  unsigned char nType = 
    pRom.peekw(OffsetW(WBoardVersion)) & 0xff;

  if(nModel == 1190) {
    if((nType != ModuleVersion::A) && 
       (nType != ModuleVersion::B)) {
      return false;
    }
  }
  else {			// Already know from above it's a 1290
    if((nType != ModuleVersion::A) && 
       (nType != ModuleVersion::N)) {
      return false;
    }
  }
  // Ok board.

  return true;
}

/*!
Fetch the properties of a CAEN V 1x90 board into member data.
Stores the following:
-	m_nModel - from the configuration prom fields.
-	m_cVersion - from the module's prom fields (this is the 
                     submodel of the board actually).
-	m_nSerialNumber - from the module's prom fields.
-	m_nBoardRevision - from the module's prom fields.
-	m_nChannels - computed from the model and version.
-	m_nChipCount -computed from the model and version
-	m_fCanHiRes  - computed from the model.
-	m_fIsHiresMode - read from the module's status register if 
                         it is hires capable.
-	m_fTriggerMatchMode - read from the module's status 
                              register.



    \param pRom	        (CVmeModule&	IN)
       pointer like object to the module's configuration rom.



 */
void
CCAENV1x90::BoardProperties(CVmeModule& pROM)
{
  m_nModel = ReadPromLong(pROM,WModelNumber2);

  // Sub module gets encoded as a letter:

  uint16_t ntype= 
    pROM.peekw(OffsetW(WBoardVersion)) & 0xff;
  if(ntype == ModuleVersion::A) {
    m_cVersion = 'A';
   }
  else if (ntype == ModuleVersion::B) {
    m_cVersion = 'B';
  }
  else if (ntype ==  ModuleVersion::N) {
    m_cVersion = 'N';
  }
  else {			// Not allowed.
    throw string("Unrecognized module subtype, not A,B, nor N");
  }

  // Serial number is a prom word:

  m_nSerialNumber = 
    (pROM.peekw(OffsetW(WSerialNumber1)) & 0xff) << 8;
  m_nSerialNumber |=
    (pROM.peekw(OffsetW(WSerialNumber0)) & 0xff);

  // Board revision is 4 bytes (a true long word).
  // this will be built up by trickery from a 3 byte long:

  m_nBoardRevision = ReadPromLong(pROM, WRevision3); // top 24 bits
  m_nBoardRevision = (m_nBoardRevision << 8) |
    (pROM.peekw(OffsetW(WRevision0)) & 0xff); // bottom 8.

  // Module specific info is as follows:
  //           chans   chips  CanHires
  //   1190 A:  128      4     no
  //   1190 B:   64      2     no
  //   1290 A:   32      4     yes
  //   1290 N:   16      2     yes

  if(m_nModel == 1190) {
    m_fCanHiRes  = false;
    if(m_cVersion == 'A') {
      m_nChannels = 128;
      m_nChipCount=   4;
    } 
    else {
      m_nChannels =  64;
      m_nChipCount=   2;
    }
  } 
  else {			// 1290
    m_fCanHiRes = true;
    if(m_cVersion == 'A') {
      m_nChannels  = 32;
      m_nChipCount =  4;
    } 
    else {
      m_nChannels  = 16;
      m_nChipCount =  2;
    }
  }

  // Now use the SR to figure out if the module is in trigger
  // match mode.  We don't need to conditionalize the
  // resolution check for hires as the hi-res mode is not
  // an allowed resolution combination for the 1190:
  
  uint16_t sr = SR();
  m_fTriggerMatching = (sr & StatusRegister::TRG_MATCH) == 
                                        StatusRegister::TRG_MATCH;
  m_fIsHiResMode      = (sr & StatusRegister::RESOLUTIONMASK) ==
                                        StatusRegister::RES_25ps;

}

/*!
  Return the value of the board's status register. Reads the 
  board's status register using the m_pRegisters pseudo pointer 
  and returns its value to the caller.
  
  \return uint16_t
  \retval  The value of the status register.

 */
uint16_t
CCAENV1x90::SR()
{
  return m_pRegisters.peekw(OffsetW(WStatusRegister));
}

/*!
Checks the state of a bit in the status register.
-	The status register is read. temp = SR()
-	isset = temp & (1 << bitnum)
-	return isset != 0


\param Bitnum	(uint16_t	IN)
   Number of a bit to check numbered from 0.

 \return bool
 \retval true -  if the specified bit is set in the module's 
                 status register.
 \retval false - if the specified bit is not set in the module's 
                 status register.


 */
bool
CCAENV1x90::isSetSR(uint16_t bitnum)
{
  return (SR() & (1 << bitnum)) != 0;
}

/*!
Writes the mask to the module's control register.  
The control register is accessed via the m_pRegisters pseudo 
pointer.


\param mask	(uint16_t	in)
	New value to write.

 */
void
CCAENV1x90::WriteCR(uint16_t mask)
{
  m_pRegisters.pokew(mask, OffsetW(WControlRegister));
}

/*!
   Read the current value of the control register.

   \return uint16_t
   \retval  The current contents of the control register.


 */
uint16_t
CCAENV1x90::ReadCR()
{
  return m_pRegisters.peekw(OffsetW(WControlRegister));
}
/*!
  Sets a specific single bit in the module's control register.

  \param bit	(uint16_t	in)
       Number (from 0) of the bit to set.

   \return void

 */
void 
CCAENV1x90::BitSetCR(uint16_t bit)
{
  uint16_t cr = ReadCR();
  cr |= (1 << bit);
  WriteCR(cr);
}

/*!
  Clears a specific bit in the module's control register.

  \param bit	(uint16_t	 in)
     Number (from 0) of the bit to clear.


 */
void
CCAENV1x90::BitClearCR(uint16_t bit)
{
  uint16_t cr = ReadCR();
  cr &= ~(1 << bit);
  WriteCR(cr);
}


/*!
  Read a longword from the configuration prom.  In the
  config prom, longwords are stored high endian, one byte per long.
  The valid byte is in the low order byte of each word.
  Access to the prom is also limited to D16 and the first 16 words
  of each longword offset is what is significant.

  \param prombase (CVmeModule& in) 
     reference to the module 'map'.
  \param ByteOffset (uint32_t in )  
     Offset in bytes to the first word to read.

  \return long
  \retval the value of the longword.
*/
uint32_t
CCAENV1x90::ReadPromLong(CVmeModule& prombase,
			 uint32_t ByteOffset)
{
  uint32_t n;		// Result long.
  uint32_t WordOffset = OffsetW(ByteOffset);

  n  = (prombase.peekw(WordOffset)   & 0xff) << 16;
  n |= (prombase.peekw(WordOffset+2) & 0xff) <<  8;
  n |= (prombase.peekw(WordOffset+4) & 0xff);

  return n;

}
/*!
   Wait for the micro to be writable.
*/
void
CCAENV1x90::WaitMicroWrite()
{

  while((m_pRegisters.peekw(OffsetW(WMicroHandshake)) & 
	 MicroHandshake::WRITE_OK)  == 0) 
    ;				// Spin on the register...

}
  
/*!
  Writes a single word to the module's micro controller with 
  appropriate handshake.  This is a utility function used by 
  the member functions that must interact with the micro 
  controller.

  \param int nWord	(uint16_t	In)
    Word to write.

 */
void 
CCAENV1x90::WriteMicro(uint16_t nWord)
{
  // Wait for the micro to be writable...

  WaitMicroWrite();

  // Write the word.
  
  m_pRegisters.pokew(nWord, OffsetW(WMicroData));

}
/*!
  Reads a single word from the micro controller waiting if 
  necessary for the word to become available.  Note that it is 
  the caller's responsibility to be sure that the module will 
  have a data word available (one reason this is private), otherwise
  this function will spin endlessly waiting for the
  MicroHanshake::READ_OK bit.

  \return uint16_t
  \retval the word returned from the micro-controller.


 */
uint16_t
CCAENV1x90::ReadMicro()
{
  // Wait for data to be presented:

  while((m_pRegisters.peekw(OffsetW(WMicroHandshake)) & 
	 MicroHandshake::READ_OK)  == 0)
    ;				// Spin on the bit.

  // Return the data:

  return m_pRegisters.peekw(OffsetW(WMicroData));
}
/*!
  Calls WriteMicro nWords times to write consecutive words pointed 
  to by pWords.   This function is used by members that need to 
  write blocks of several words to the microcontroller.


  \param pWords	(void*	in)
  Pointer to words to write.
 
  \param nWords	(uint32_t	in)
  Number of words to write.
 
*/
void 
CCAENV1x90::WriteMicroBlock(void* pWords,
			    uint32_t nWords)
{
  uint16_t* pW = (uint16_t*) pWords;

  for(int i = 0; i < nWords; i++) {
    WriteMicro(*pW++);
  }
}
/*!
  Reads Calls ReadMicro nWords times, storing the results in 
  consecutive memory locations pointed to by pWords.  
  This function is used by members that must transfer blocks of 
  data from the microcontroller.

  \param pWords	(void*	out)
	Pointer to a buffer to hold the words.
  \param nWords	(uint32_t	in)
         Number of words to read

 */
void
CCAENV1x90::ReadMicroBlock(void* pWords, 
			   uint32_t nWords)
{
  uint16_t* pW = (uint16_t*) pWords;

  for(int i =0; i < nWords; i++) {
    *pW++ = ReadMicro();
  }
}
/*!
  Writes opcode to the micro controller followed by a read of 
  nWords from the microcontroller into the buffer pointed to by 
  pWords. This performs a  full operation with the 
  microcontroller that requests and transfers data from the 
  micro controller.

  \param opcode	(uint16_t	IN)
  The operation code to write to the micro
  \param pWords	(uint16_t	OUT)
	Pointer to the buffer to hold the words retreived from the micro
  \param nWords	(uint32_t	in)
	Number of words to read from the micro

*/
void 
CCAENV1x90::MicroTransaction(uint16_t opcode,
			     void*          pWords,
			     uint32_t   nWords)
{
  WriteMicro(opcode);
  ReadMicroBlock(pWords, nWords);
}
