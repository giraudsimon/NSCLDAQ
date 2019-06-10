/*=========================================================================*\
| Copyright (C) 2008 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

/*
  Change Log:
*/

static const char* Copyright= "(C) Copyright Michigan State University 2008, All rights reserved";

#include <config.h>

#include <CADC2530.h>
#include <string>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <assert.h>

using namespace std;


//   Convenience function for longword swapping.
#define swaplong(n)  (((n >> 16) & 0xffff) | ((n & 0xffff) << 16))

// Byte offset associated with a structure/field.
#define Offset(structname, field) ((unsigned int)&(((structname*)0x0)->field))

// Short and Long noffset associated with a struct/field.
#define ShortOffset(structname, field) (Offset(structname,field)/sizeof(uint16_5))
#define LongOffset(structname, field)  (Offset(structname,field)/sizeof(uint32_t))

// VME crate definitions
#define VME_CRATE_SIZE      24	// Slots in a VME crate.

// ADC2530 register and memory sizes and offsets
#define CADC2530_REGSIZE       0xC0      // Size of register set.
#define CADC2530_MEMSIZE       0x200000  // Size of all histo/list memory.
#define CADC2530_HISTOMEMSIZE  0x40000   // Size of histogram memory.
#define CADC2530_HISTOCHANSIZE 0x2000    // Size of histogram channel in longs.

// This value masks out (logical AND) the bits returned from a
// CSR read that should not be retained during a CSR set operation.
#define CADC2530_CSRMASK 0x00FF8A

// This value masks out (logical AND) the bits returned from a
// CTR read that should not be retained during a CTR set operation.
#define CADC2530_CTRMASK 0x00810F

// Defines for parsing list mode data blocks
#define IS_LIST_HEADER(x) (((((x>>24)&0x07) == 0x2)) ? true : false) 
#define IS_LIST_EOB(x) (((((x>>24)&0x07) == 0x4)) ? true : false) 
#define IS_LIST_CHANNEL(x) ((((x>>24)&0x07) == 0x0)) ? true : false) 
#define GET_LIST_CHANCOUNT(x) (((x>>8)&0x07)+1)
#define GET_LIST_CHANNUM(x) (((x>>16)&0x07)+1)
#define GET_LIST_CHANDATA(x) (x&0x1FFF)
#define GET_LIST_EVENTCOUNT(x) (x&0x00FFFFFF)

// Definition of the register and configuration memory.
typedef struct ADCregisters_struct {
  uint16_t  ManufacturerId;              // 0x00      (R)
  uint16_t  DeviceType;	               // 0x02      (R)
  uint16_t  CSR;                         // 0x04      (RW)
  uint16_t  MemoryOffset;                // 0x06      (RW)
  uint16_t  ListAddressLS;               // 0x08      (RW)
  uint16_t  ListAddressMS;               // 0x0A      (RW)
  uint16_t  InterruptVector;             // 0x0C      (RW)
  uint16_t  CTR;    	               // 0x0E      (RW)
  uint16_t  FullnessFlag;                // 0x10      (RW)
  uint16_t  InterruptMask;               // 0x12      (RW)
  uint16_t  LowerLevelDisc;              // 0x14      (RW)
  uint16_t  UpperLevelDisc;              // 0x16      (RW)
  uint16_t  EventCounterLS;              // 0x18      (RW)
  uint16_t  EventCounterMS;              // 0x1A      (RW)
  uint16_t  Conversion;                  // 0x1C      (R)
  uint16_t  SlidingScaleTest;            // 0x1E      (RW)
  uint16_t  NotUsed2[16]; 	       // 0x20-0x3F  
  uint16_t  SerialNumber;                // 0x40      (R) 
  uint16_t  DataInNonVolatileMem[63];    // 0x42-0xBF
} ADCregisters_t;

// Bit definition for the control and status register (CSR)
typedef union ADCcsr_union {
  uint16_t csrval;
  struct bit_struct {
    uint8_t RSTBSY : 1; // Read->Busy, Write->Reset (RW)
    uint8_t SS     : 1; // Enable sliding scale (0 is ON) (RW)
    uint8_t DR     : 1; // Data ready (R)
    uint8_t SC     : 1; // Set calibration on (RW)
    uint8_t FFCLR  : 1; // Force fast clear on all channesl (W)
    uint8_t ACM    : 1; // Auto memory clear (RW) (not completed)
    uint8_t IS     : 1; // Interrupt status (R)
    uint8_t IE     : 1; // Interrupt enable (RW)
    uint8_t ARM    : 1; // Start acquisition (RW)
    uint8_t IPL    : 3; // Set interrupt priority level (RW)
    uint8_t FHE    : 1; // Full/Half full or Event interrupt (RW)
    uint8_t ZE     : 1; // Enable zero conversion (RW)
    uint8_t GE     : 1; // Enable gate mode (RW)
    uint8_t HE     : 1; // Enable histogram mode (RW)
  } bits;
} ADCcsr_t;

// Bit definition for the calibration and test register (CTR)
typedef union ADCctr_union {
  uint16_t ctrval;
  struct bit_struct {
    uint8_t M012   : 3; // Channel select bits 0-2 (RW)
    uint8_t MEN    : 1; // Enable the MUX switch (RW)
    uint8_t D04    : 1; // Unused
    uint8_t D05    : 1; // Unused
    uint8_t D06    : 1; // Unused
    uint8_t D07    : 1; // Unused
    uint8_t DISC   : 1; // Disable compensation (0 -> comp. applied) (RW)
    uint8_t D09    : 1; // Unused
    uint8_t D10    : 1; // Unused
    uint8_t D11    : 1; // Unused
    uint8_t D12    : 1; // Unused
    uint8_t D13    : 1; // Unused
    uint8_t D14    : 1; // Unused
    uint8_t DEAC   : 1; // Disable auto fast clear
  } bits;
} ADCctr_t;

// Bit definition for the fullness flag register
typedef union ADCfullnessflag_union {
  uint16_t flags;

  struct byte_struct {
    uint8_t HALFFULL : 8; // Half full flags
    uint8_t FULL     : 8; // Full flags
  } bytes;

  struct bit_struct {
    uint8_t F1     : 1; 
    uint8_t F2     : 1; 
    uint8_t F3     : 1; 
    uint8_t F4     : 1; 
    uint8_t F5     : 1; 
    uint8_t F6     : 1; 
    uint8_t F7     : 1; 
    uint8_t F8     : 1; 
    uint8_t HF1    : 1; 
    uint8_t HF2    : 1; 
    uint8_t HF3    : 1; 
    uint8_t HF4    : 1; 
    uint8_t HF5    : 1; 
    uint8_t HF6    : 1; 
    uint8_t HF7    : 1; 
    uint8_t HF8    : 1; 
  } bits;
} ADCfullnessflag_t;

/*==============================================================*/
/** @fn CADC2530::CADC2530(int slotNum,int crateNum,long nBase)
* @brief Constructor.
*
* NOTE: geographic addressing is currently unsupported.
*
* Constructs a Hytec 2530 ADC card. The card can be addressed either
* geographically or via base addres switches.  If geographical 
* addressing is used, the slot value comes from the card position 
* in the crate.  Otherwise it is programmed by the slotNum parameter.
* 
* @param crateNum The crate number in which the module lives.
* This selects which of the VME crates the module is in.  VME
* crate numbers can be determined using the cratelocator utility.
* The crate number is also programmed into the crate register so
* that the module crate field in the data stream is correct.
* 
* @param nBase long [in] The base address of the module.  
*
* @return this
*/                                                             
CADC2530::CADC2530(int crateNum,uint32_t nBase) :
  my_nCrate(crateNum),
  my_nBase(nBase),
  my_nMemOffset(nBase),
  my_pModule(NULL),
  my_pMemory(NULL),
  my_nCardId(0),
  my_nCardType(0),
  my_nModFd(0),
  my_nMemFd(0),
  my_eventmode(false),
  my_cureventpos(0)
{
  assert(sizeof(ADCregisters_t) == CADC2530_REGSIZE); // Else struct misdefined.
  slotInit();
}

/*==============================================================*/
/** @fn CADC2530::CADC2530(const CADC2530& card)
* @brief Copy constructor.
*
*  Copy constructor: Constructs a new card from an existing card
*  object.  A new map is established to the card for this object.
*  The card is not re-initialized, but remains in the state defined
*  by the existing card.
*
* @param card The card to copy.
* @return this
*/                                                             
CADC2530::CADC2530(const CADC2530& card) :
  my_nCrate(card.my_nCrate),
  my_nBase(card.my_nBase),
  my_nMemOffset(card.my_nMemOffset),
  my_nCardId(card.my_nCardId),
  my_nCardType(card.my_nCardType),
  my_pModule(NULL),
  my_pMemory(NULL),
  my_nModFd(0),
  my_nMemFd(0),
  my_eventmode(false),
  my_cureventpos(0)
{
  mapCard();			// Map the card.
}

/*==============================================================*/
/** @fn CADC2530::CADC2530(const CADC2530& card)
* @brief Copy constructor.
*
* Assignment operator.  This card is destroyed, and replaced with
* a copy of the card described by <em>card</em>.  this is not
* initialized, but remains in the state defined by the rhs of the
* assignment.
*
* @param card The card to assign to this.  
* @return Dereferenced version of this.
*/                                                             
CADC2530& CADC2530::operator=(const CADC2530& card) {
  if (this != (&card)) {
    if (this->my_pModule != NULL) destroyCard(); // Destroy old memory map.
    my_nCrate = card.my_nCrate;
    my_nBase  = card.my_nBase;
    my_nMemOffset = card.my_nMemOffset;
    my_eventmode = card.my_eventmode;
    my_cureventpos= card.my_cureventpos;
    my_nMemFd  = 0;
    my_nModFd  = 0;
    my_pModule = NULL;
    my_pMemory = NULL;
    mapCard();
  }
  return *this;
}

/*==============================================================*/
/** @fn CADC2530::~CADC2530()
* @brief Unmap and destruct.       
*
* Unmap and destruct this card object.
*
* @param None
* @return None
*/                                                             
CADC2530::~CADC2530() {
  destroyCard();
}

/*==============================================================*/
/** @fn CADC2530::checkCard(int nCrate,long nBase,uint16_t &rType,uint16_t &rManId)
* @brief Static method to check if a card is a Hyted 2530 ADC.
*
* Static method to check if a card is a Hytec 2530 ADC.  Simply maps
* the register set and retrieves the manufacturer id and device type 
* from the locations where they are stored for the ADC2530.  These
* values are returned in rManId and rType.
*
* Note, this is a static method that does not require the
* instantiation of this class to be called.  Also, it should be
* called as: CADC2530::checkCard().
*
* @param nCrate The crate number to check.
* @param nBase The crate number to check.
* @param rType The device type (output).
* @param rManId The manufactuer id (output).
* @return true If this card appears to be a ADC2530, else false.
*/                                                             
bool CADC2530::checkCard(int nCrate,uint32_t nBase,uint16_t &rType, uint16_t &rManId) {
  // Ensure that the slot and crate specified stay within bounds.
  int crate = nCrate & 0xff;  // Not important enough to give an error,
                                // just discard the extra.

  // The card is not mapped yet, so do it.
  void *fd = NULL;
  rType = 0;
  rManId = 0;


   fd = CVMEInterface::Open(CVMEInterface::A24,crate);
   volatile uint16_t *adcmod = (volatile uint16_t*)CVMEInterface::Map(fd,nBase,CADC2530_REGSIZE);


   rType = ((volatile ADCregisters_t*)adcmod)->DeviceType;
   rManId = ((volatile ADCregisters_t*)adcmod)->ManufacturerId;
   CVMEInterface::Unmap(fd,(void*)adcmod,CADC2530_REGSIZE);
   CVMEInterface::Close(fd);


   adcmod = NULL;
   fd = NULL;

   if ((rManId != 0x8063)||(rType != 2530)) return false;
   else return true;
}

/*==============================================================*/
/** @fn CADC2530::volt2lld(double volt)
* @brief Static method to convert a voltage to an LLD setting.
*
* Static method to convert a voltage in the range [0-0.8191] volts
* to a lower level discriminator (LLD) value.  All specified values 
* greater than 0.8191 will return 0x3FFC (maximum setting), and
* all values less than zero will return 0x0 (minimu setting).
*
* Note, this is a static method that does not require the
* instantiation of this class to be called.  Also, it should be
* called as: CADC2530::volt2lld().
*
* @param volt The voltage to convert.
* @return An LLD setting for the specified voltage.
*/                                                             
uint16_t CADC2530::volt2lld(double volt) {
  uint16_t sval = 0; 
  if (volt < 0) {
    sval = 0x0;
  } else if (volt > 0.8191) {
    sval = 0x3FFC;
  } else {
    int val = (int)((volt/3.2764)/(0.25/4095));
    sval = (uint16_t)((val << 2)&0x00003FFC);
  }

  return sval;
}

/*==============================================================*/
/** @fn CADC2530::volt2uld(double volt)
* @brief Static method to convert a voltage to an ULD setting.
*
* Static method to convert a voltage in the range [0-8.191] volts.
* to a upper level discriminator (ULD) value. All values
* greater than 8.191 will be set to 8.191 and all values less than
* zero will be set to 0.
*
* Note, this is a static method that does not require the
* instantiation of this class to be called.  Also, it should be
* called as: CADC2530::volt2uld().
*
* @throw A string exeception on error.
* @param volt The voltage to convert.
* @return An ULD setting for the specified voltage.
*/                                                             
uint16_t CADC2530::volt2uld(double volt) {
  uint16_t sval = 0;

  if (volt < 0) {
    sval = 0x0;
  } else if (volt > 8.191) {
    sval = 0x3FFC;
  } else {
    int val = (unsigned int)trunc(((volt/3.2764)-2)/(0.5/4095));
    sval = (uint16_t)((val << 2)&0x00003FFC);
  }

  return sval;
}

/*==============================================================*/
/** @fn CADC2530::toString()
* @brief Stringify this card.
*
* Stringify this card as a std::string that describes this
* module.
*
* @throw A string execption if there were any problems.
* @param None
* @return A std::string representation of this module.
*/                                                             
const std::string& CADC2530::toString() {
  static string namstr;
  static char buffer[256];
  sprintf(buffer,"ADC2530 in crate %d at base 0x%lx with memory offset 0x%lx",my_nCrate,
	  (long unsigned int)my_nBase, (long unsigned int)my_nMemOffset);
  namstr = buffer;
  return namstr;
}



/*==============================================================*/
/** @fn CADC2530::readListEvent(void *buf,int lngsLeft)
* @brief Read a single event into a user buffer.
*
* Reads a single list memory event into a user buffer (pointer to
* local memory).  The buf parameter is a pointer to local memory
* that has already been alocated.  
*
* This method is meant for internal use and assumes that
* list memory boundary checking has already been completed before
* this method was called.
*
* @param buf A pointer to local memory that has already been allocated.
* @param lngsLeft Number of longs left in list memory.
* @return An integer indicating the number of BYTES of data placed in buf.
*   @retval >0 indicates the number of BYTES of data placed in buf.
*   @retval 0 indicates that no data was placed in the buffer.
*/                                                             
int CADC2530::readListEvent(void* buf,int lngsLeft) {
  int lngcnt = 0;
  unsigned int *lptr = (unsigned int*)buf; 

  // Need at least a header and EOB
  if (lngsLeft < 2) return(0);


  lptr[lngcnt] = ((volatile uint32_t*)my_pMemory)[my_cureventpos];

  int chans = GET_LIST_CHANCOUNT(lptr[0]);

  // Only read out the last event if there is enough memory
  // to contain it.
  if ((chans+2) < lngsLeft) {
    lngcnt++;
    my_cureventpos++; 

#ifdef ADC2530_LOOP_COPY

    for (int i = 0; i < (chans+1); i++) {
      lptr[lngcnt] = ((volatile uint32_t*)my_pMemory)[my_cureventpos];
      my_cureventpos++;  
      lngcnt++; 
    } 
#else
    memcpy(lptr, const_cast<const unsigned int*>(my_pMemory+my_cureventpos), 
	   (chans+1)*sizeof(unsigned int));
#endif
  } 

  return lngcnt*sizeof(unsigned int);
}

/*==============================================================*/
/** @fn CADC2530::readListEvents(void *buf,int& nEvents)
* @brief Read multiple events into a user buffer.
*
* Reads multiple list memory events into a user buffer (pointer to
* local memory).  The buf parameter is a pointer to local memory
* that has already been alocated.  The size of buf should be
* at least 40*nEvents in size to guarantee sufficient space
* for reading the request number of events.
*
* @param buf A pointer to local memory that has already been allocated.
* @param nEvents (IN) The number of events to read, (OUT) the number of
*                     events actually read.
* @return An integer indicating the number of BYTES of data placed in buf.
*   @retval >0 indicates the number of BYTES of data placed in buf.
*   @retval 0 indicates that no data was placed in the buffer.
*/                                                             
int CADC2530::readListEvents(void* buf,int& nEvents) {
  int lngcnt = 0;
  unsigned int *lptr = (unsigned int*)buf; 

  // First check list memory boundaries
  unsigned int listaddr = getListAddress();
  if (listaddr == 0) {
    if (dataReady()) listaddr = (CADC2530_MEMSIZE/sizeof(unsigned int));
  }

  // Now read the events
  int eventsread = 0;
  for (int i = 0; i < nEvents; i++) {
    int bytes = readListEvent((void*)lptr,listaddr-my_cureventpos);
    if (bytes <= 0) break;  // No more events
    eventsread++;

    lptr += (bytes/sizeof(uint32_t));
    lngcnt += (bytes/sizeof(uint32_t));
  } 

  nEvents = eventsread;
  return lngcnt*sizeof(uint32_t);
}

/*==============================================================*/
/** @fn CADC2530::readHistogramChannel(void *buf,int channum) 
* @brief Read a single event into a user buffer. 
* 
* Reads a single histogram channel into a user buffer (pointer to
* local memory).  The buf parameter is a pointer to local memory
* that has already been alocated.  This user allocated buffer must
* be sized to contain at least 8192 (0x2000) 32-bit long words
* (32768 bytes).
*
* @throw String exception on error.
* @param buf A pointer to local memory that has already been allocated.
* @param channum The channel number to read in the range [1-8].
* @return An integer indicating the number of BYTES of data placed in buf.
*   @retval >0 indicates the number of BYTES of data placed in buf.
*   @retval 0 indicates that no data was placed in the buffer.
*/                                                             
int CADC2530::readHistogramChannel(void* buf,int channum) {
  if ((channum < 1)||(channum > 8)) {
    char buffer[256];
    sprintf(buffer, "CADC2530::readHistogramChannel(): the channel number specified must be in the range [1-8].  A channel number of %d was specified\n",channum);
    throw string(buffer);
  } 

  int chstart = (channum - 1) * CADC2530_HISTOCHANSIZE;
  unsigned int *lptr = (unsigned int*)buf; 


  for (int i = 0; i < CADC2530_HISTOCHANSIZE; i++) {
    lptr[i] = ((volatile uint32_t*)my_pMemory)[chstart+i];
  }

  return (CADC2530_HISTOCHANSIZE * sizeof(unsigned int));
}

/*==============================================================*/
/** @fn CADC2530::slotInit()
* @brief Initialize a card to a standard, well defined state.
*
* Maps the card and initializes it to a well defined state.
* The card location is assumed to already have been set up 
* as defined by the member variables (except for my_pModule and
* my_pMemory which are filled in by mapCard()).
*
* @throw A string execption if there were any problems.
* @param None
* @return None
*/                                                             
void CADC2530::slotInit() {
  mapCard();
  resetCard();
  clearHistogramMemory();
}

/*==============================================================*/
/** @fn CADC2530::cardType()
* @brief Return the device type of this card.
*
* Return the device type of this card.  It should 2530(dec) for
* the Hytec 2530 ADC as read from location base+0x02.
*
* @param None
* @return The device type as an uint16_t.
*/                                                             
uint16_t CADC2530::cardType() {
  if (!my_nCardType) {
    my_nCardType = ((volatile ADCregisters_t*)my_pModule)->DeviceType;
  }
  return my_nCardType;
}

/*==============================================================*/
/** @fn CADC2530::manufacturerId()
* @brief Return the manufacturer id for this card.
*
* Return the manufacturer id for this card.  It should 0x8063 for
* the Hytec 2530 ADC as read from location base+0x0.
*
* @param None
* @return The manufacturer id as an uint16_t.
*/                                                             
uint16_t CADC2530::manufacturerId() {
  if (!my_nCardId) {
    my_nCardId = ((volatile ADCregisters_t*)my_pModule)->ManufacturerId;
  }
  return my_nCardId;
}

/*==============================================================*/
/** @fn CADC2530::setMemoryOffset(uint32_t memoset))
* @brief Set the memory offset to the specified address.
*
* Set the memory offset to the specified A32 address.
* Note, memoff is right shifted 16 bits and logically ANDed 
* with 0xFFE0 before it is written to the register (the A32 return 
* value reflects this).
*
* @param memoset The A32 memory offset base.
* @return The value read back from the register as an A32 address.
*/                                                             
uint32_t CADC2530::setMemoryOffset(uint32_t memoset) {
  uint32_t lval = (memoset >> 16);
  uint16_t sval = (lval & 0xFFE0);  

  ((volatile ADCregisters_t*)my_pModule)->MemoryOffset = sval;
  lval = ((volatile ADCregisters_t*)my_pModule)->MemoryOffset;

  return (lval << 16);
}

/*==============================================================*/
/** @fn CADC2530::calcMemoryOffset(uint32_t base))
* @brief Calculate the memory offset register setting.
*
* Calculate the memory offset register setting given
* the current A24 base address for this card's registers.
*
* @param base The base address for this card.
* @return A computed memory offset A32 base value.
*/                                                             
uint32_t CADC2530::calcMemoryOffset(uint32_t base) {
  uint32_t regset = ((base >> 8) & 0xFFE0);
  return (regset << 16); 
}

/*==============================================================*/
/** @fn CADC2530::mapModule()
* @brief Map this cards registers into memory.
*
* Given the values in 
*   - my_nCrate - Physical crate number.
*   - my_nBase  - Base address.
*
*   - Creates a map to the registers of the module.
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::mapModule() {
   // Donot map the register memory more than once.
   if (my_pModule != NULL) {
     char buffer[256];
     sprintf(buffer, "CADC2530::mapModule(): The Card in crate %d at base 0x%lx already has a register memory map.\n",
	     my_nCrate, (long unsigned int)my_nBase);
     throw string(buffer);
   }

   // Ensure that the slot and crate specified stay within bounds.
   my_nCrate = my_nCrate & 0xff;  // Not important enough to give an error, 
                                // just discard the extra.

   // The card is not mapped yet, so do it.
   void *fd = NULL;

   fd = CVMEInterface::Open(CVMEInterface::A24, my_nCrate);
   my_pModule = (volatile uint16_t*)CVMEInterface::Map(fd,my_nBase,
						 CADC2530_REGSIZE);

   // Check if the module type is a 2530(dec) and that the manufacturer is
   // 0x8063.  If not, then issue and error message.
   my_nCardId = manufacturerId();
   my_nCardType = cardType();

   if ((my_nCardId != 0x8063)||(my_nCardType != 2530)) {   

     CVMEInterface::Unmap(fd,(void*)my_pModule,CADC2530_REGSIZE);
     CVMEInterface::Close(fd);
     my_pModule = NULL;
     char buffer[256];
     sprintf(buffer, "CADC2530::mapModule(): Card in crate %d at base 0x%lx is not a Hytec 2530 ADC or is missing type=%d id=0x%0x\n",
	     my_nCrate, (long unsigned int)my_nBase ,my_nCardType,
	     static_cast<unsigned int>(my_nCardId));
     throw string(buffer);
   }

   my_nModFd = fd;			// Save for destruction.
}

/*==============================================================*/
/** @fn CADC2530::mapMemory()
* @brief Map this card's list/histogram memory.
*
* Given the values in 
*   - my_nCrate - Physical crate number.
*   - my_nBase  - Base address.
*
*   - Creates a map to the list/histogram memory of the module.
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::mapMemory() {
   // We must have a register memory map first.
   if (my_pModule == NULL) {
     char buffer[256];
     sprintf(buffer, "CADC2530::mapMemory(): Cannot map list/histogram memory without first mapping the ADC2530 registers for card in crate %d at base 0x%lx.\n",
	     my_nCrate, (long unsigned int)my_nBase);
     throw string(buffer);
   } 

   // Donot map the list/histogram memory more than once.
   if (my_pMemory != NULL) {
     char buffer[256];
     sprintf(buffer, "CADC2530::mapMemory(): The Card in crate %d at base 0x%lx already has a list/histogram memory map.\n",
	     my_nCrate, (long unsigned int)my_nBase);
     throw string(buffer);
   }

   // Ensure that the slot and crate specified stay within bounds.
   my_nCrate = my_nCrate & 0xff;  // Not important enough to give an error, 
                                  // just discard the extra.

   // The card is not mapped yet, so do it.
   void *fd = NULL;

   // Now can map the list/histogram memory
   uint32_t nmemoset = calcMemoryOffset(my_nBase); 
   my_nMemOffset = setMemoryOffset(nmemoset);

   // If we did this correctly and the card is working correctly,
   // we should read back the setting we calculated.
   if (nmemoset != my_nMemOffset) {
     char buffer[256];
     sprintf(buffer, "CADC2530::mapMemory(): Error setting list/histogram memory offset for ADC2530 card in crate %d at base 0x%lx (written=0x%lx, read=0x%lx)\n",my_nCrate,
	     (long unsigned int)my_nBase, (unsigned long int)nmemoset, (unsigned long int)my_nMemOffset);
     throw string(buffer);
   } 

   // Now map the list/histogram memory.

   fd = CVMEInterface::Open(CVMEInterface::A32, my_nCrate);
   my_pMemory = (volatile unsigned int*)CVMEInterface::Map(fd,my_nMemOffset,
						 CADC2530_MEMSIZE);
   
   my_nMemFd = fd;			// Save for destruction.
}

/*==============================================================*/
/** @fn CADC2530::destroyMemory()
* @brief Unmap this card's list/histogram memory.
*
* Destroy the list/histogram memory map for this card.
*
* @param None
* @return None
*/                                                             
void CADC2530::destroyMemory() {
  if (my_pMemory == NULL) return;


  CVMEInterface::Unmap(my_nMemFd,(void*)my_pMemory,CADC2530_MEMSIZE);
  CVMEInterface::Close(my_nMemFd);

  my_pMemory = NULL;
  my_nMemFd = NULL;
}

/*==============================================================*/
/** @fn CADC2530::clearMemory()
* @brief Zero out the histogram/list memory.
*
* Write zeroes to all locations in the histogram/list memory.
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::clearMemory() {
  // We must have a memory map first.
  if (my_pMemory == NULL) {
    char buffer[256];
    sprintf(buffer, "CADC2530::clearMemory(): Cannot clear list/histogram memory without first having a memory map for card in crate %d at base 0x%lx.\n",my_nCrate,
	    (long unsigned int)my_nBase);
    throw string(buffer);
  } 

  for (int i = 0x0; i < (CADC2530_MEMSIZE/4); i++) {

    ((volatile unsigned int*)my_pMemory)[i] = (unsigned int)0x0;
  }
}

/*==============================================================*/
/** @fn CADC2530::clearHistogramMemory()
* @brief Zero out only histogram memory.
*
* Write zeroes to all locations in only the histogram memory.
* Since the memory used for histogramming is significantly
* smaller that that used in list mode, clearing only histogram
* memory is faster than clearing all of the card's memory.
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::clearHistogramMemory() {
  // We must have a memory map first.
  if (my_pMemory == NULL) {
    char buffer[256];
    sprintf(buffer, "CADC2530::clearHistogramMemory(): Cannot clear histogram memory without first having a memory map for card in crate %d at base 0x%lx.\n",
	    my_nCrate, (long unsigned int)my_nBase);
    throw string(buffer);
  } 

  for (int i = 0x0; i < (CADC2530_HISTOMEMSIZE/4); i++) {
    ((volatile unsigned int*)my_pMemory)[i] = (unsigned int)0x0;
  }
}

/*==============================================================*/
/** @fn CADC2530::resetCard()
* @brief Reset card to a set of initial settings.
*
* Reset this card to a set of initial settings.  This includes
* issuing a CSR clear and FFCLR and setting the CSR to an
* all zero state.  This does not clear the list/histogram
* memory (call clearMemory() or clearHistogramMemory() to zero 
* all list/histogram memory or just histogram memory respectively).
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::resetCard() {
  // We must have a memory map first.
  if (my_pModule == NULL) {
    char buffer[256];
    sprintf(buffer, "CADC2530::resetCard(): Cannot reset this card without first having a memory map for card in crate %d at base 0x%lx.\n",
	    my_nCrate, (long unsigned int)my_nBase);
    throw string(buffer);
  } 

  disarm();
  resetCTR();
  setLLD(0x0);
  setULD(0x0);
  setInterruptVector(0x0);
  setInterruptMask(0x0);
  fastClear();
  resetCSR();
  clearEventCounter();
  clearListAddress();
  clearFullnessFlags();
  modeHistogram(false);
}

/*==============================================================*/
/** @fn CADC2530::destroyModule()
* @brief Unmap this card's register memory.
*
* Destroy the register memory map for this card.  The card
* is also reset (resetCard()).
*
* @param None
* @return None
*/                                                             
void CADC2530::destroyModule() {
  if (my_pModule == NULL) return;

  resetCard();

  CVMEInterface::Unmap(my_nModFd,(void*)my_pModule,CADC2530_REGSIZE);
  CVMEInterface::Close(my_nModFd);

  my_pModule = NULL;
  my_nModFd = NULL;
}

/*==============================================================*/
/** @fn CADC2530::mapCard()
* @brief Map the ADC card into memory.
*
* Given the values in 
*   - my_nCrate - Physical crate number.
*   - my_nBase  - Base address.
*
*   - Creates a map to the registers of the module.
*   - Creates a map to the list/histogram memory of the module.
* 
* @throw string describing why the method failed.
* @param None
* @return None
*/                                                             
void CADC2530::mapCard() {
  mapModule();
  mapMemory();
}

/*==============================================================*/
/** @fn CADC2530::destroyCard()
* @brief Unmap this card.
*
* Destroy the memory maps for this card.
*
* @param None
* @return None
*/                                                             
void CADC2530::destroyCard() {
  destroyMemory();
  destroyModule();
}

/*==============================================================*/
/** @fn CADC2530::arm()
* @brief Start acquisition according to the current mode setting.
*
* Start acquistion according to the current mode (list or 
* histogram) setting.
*
* @param None
* @return None
*/                                                             
void CADC2530::arm() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.ARM = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disarm()
* @brief Stop acquisition.
*
* Stop either histogram or list mode acquistion depending on 
* setting.
*
* @param None
* @return None
*/                                                             
void CADC2530::disarm() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.ARM = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::dataReadyOnEvent()
* @brief Data ready when there's at least one event.
*
* The data ready flag will be set when there's at least one
* event.  This is the default for gate mode (list memory) 
* operation.
*
* @see dataReadOnFullness()
* @param None
* @return None
*/                                                             
void CADC2530::dataReadyOnEvent() {
  my_eventmode = true;
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.DR = 1;
  csr.bits.FHE = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::dataReadyOnFullness()
* @brief Data ready determined by fullness flags.
*
* The data ready flag will be set based on fullness flag
* register bits.  This is the default for histogram mode.
*
* @see dataReadOnEvent()
* @param None
* @return None
*/                                                             
void CADC2530::dataReadyOnFullness() {
  my_eventmode = false;
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.DR = 1;
  csr.bits.FHE = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::enableGate()
* @brief Enable gate mode.
*
* Enable mode that uses front panel master gate.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableGate() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.GE = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disableGate()
* @brief Disable gate mode.
*
* Disable mode that uses front panel master gate.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableGate() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.GE = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::enableZeroCnv()
* @brief Enable zero conversion.
*
* Enable zero conversion.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableZeroCnv() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.ZE = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disableZeroCnv()
* @brief Disable zero conversion.
*
* Disable zero conversion.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableZeroCnv() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.ZE = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::enableCalibration()
* @brief Enable calibration.
*
* Enable calibration.  When enabled the calibration and
* test register (CTR) functions are enabled.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableCalibration() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.SC = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disableCalibration()
* @brief Disable calibration.  
*
* Disable calibrarion.  When disabled the calibration and
* test register (CTR) functions are disabled.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableCalibration() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.SC = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::enableSlidingScale()
* @brief Enable sliding scale.
*
* Enable sliding scale.  Note, when this CSR bit is 0, sliding
* scale is enabled.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableSlidingScale() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.SS = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disableSlidingScale()
* @brief Disable sliding scale. 
*
* Disable sliding scale.  Note, when this CSR bit is 1, sliding
* scale is disabled.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableSlidingScale() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.SS = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::enableInterrupt()
* @brief Enable interrupt.
*
* Enable interrupt.  An IRQ is generated if the CSR interrupt
* status bit is set and interrupts are enabled.  The IRQ number
* is determined by the CSR interrupt priority level.  This bit
* is cleared during the interrupt cycle ROAK.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableInterrupt() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.IE = 1;
  setCSRbits(csr.csrval,csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::disableInterrupt()
* @brief Disable interrupt.
*
* Disable interrupt.  An IRQ is generated if the CSR interrupt
* status bit is set and interrupts are enabled.  The IRQ number
* is determined by the CSR interrupt priority level.  This bit
* is cleared during the interrupt cycle ROAK.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableInterrupt() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.IE = 1;
  resetCSRbits(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::isArmed()
* @brief Check if acquisition has been started.
*
* Check whether acquisition has been started.
*
* @param None
* @return None
*/                                                             
bool CADC2530::isArmed() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  return (csr.bits.ARM == 1); 
}

/*==============================================================*/
/** @fn CADC2530::modeHistogram(bool indgates)
* @brief Set acquisition to histogram mode.
*
* Set acquisition to histogram mode.  If indgates is false,
* then use self gating.  If indgates is true, use individual
* gates on front panel.
*
* @param indgates True if individual front panel gates are to be used.
* @return None
*/                                                             
void CADC2530::modeHistogram(bool indgates) {
  ADCcsr_t csr;

  csr.csrval = 0;
  csr.bits.HE = 1;
  setCSRbits(csr.csrval,csr.csrval);

  if (indgates) {
    csr.csrval = 0;
    csr.bits.GE = 1;
    setCSRbits(csr.csrval,csr.csrval);
  } else {
    csr.csrval = 0;
    csr.bits.GE = 1;
    resetCSRbits(csr.csrval);
  }

  dataReadyOnFullness();
}

/*==============================================================*/
/** @fn CADC2530::modeGate()
* @brief Set acquisition to gate mode.
*
* Set acquisition to gate mode. Gate mode using
* front panel master gate with data stored in list mode.
*
* @param None
* @return None
*/                                                             
void CADC2530::modeGate() {
  ADCcsr_t csr;
  csr.csrval = 0;
  csr.bits.HE = 1;
  resetCSRbits(csr.csrval);
  csr.csrval = 0;
  csr.bits.GE = 1;
  setCSRbits(csr.csrval,csr.csrval);
  dataReadyOnEvent();
}

/*==============================================================*/
/** @fn CADC2530::getEventCounter()
* @brief Return the event counter.
*
* Return the number of events currently recorded by the
* ADC event counter.
*
* @param None
* @return The number of events.
*/                                                             
unsigned int CADC2530::getEventCounter() {
  uint16_t ecls = ((volatile ADCregisters_t*)my_pModule)->EventCounterLS;
  uint16_t ecms = ((volatile ADCregisters_t*)my_pModule)->EventCounterMS;

  unsigned int evtcnt = ((ecms&0x00FF)<<16)|(ecls&0x0000FFFF);
  return evtcnt;
}

/*==============================================================*/
/** @fn CADC2530::clearEventCounter()
* @brief Clear the ADC event counter.
*
* Set the ADC event counter to zero.
*
* @param None
* @return None
*/                                                             
void CADC2530::clearEventCounter() {
  my_cureventpos= 0;

  ((volatile ADCregisters_t*)my_pModule)->EventCounterLS = 0x0;
  ((volatile ADCregisters_t*)my_pModule)->EventCounterMS = 0x0;

}

/*==============================================================*/
/** @fn CADC2530::getListAddress()
* @brief Return the list address counter.
*
* Return the list address counter.  Can be used to determine
* the amount of data has in list memory when in gate mode.
*
* @see clearListAddress()
* @param None
* @return The list address counter.
*/                                                             
unsigned int CADC2530::getListAddress() {
  uint16_t lals = ((volatile ADCregisters_t*)my_pModule)->ListAddressLS;
  uint16_t lams = ((volatile ADCregisters_t*)my_pModule)->ListAddressMS;

  unsigned int lstaddr = ((lams&0x0000FFFF)<<16)|(lals&0x0000FFFF);
  return lstaddr;
}

/*==============================================================*/
/** @fn CADC2530::clearListAddress()
* @brief Clear the list address counter.
*
* Set the list address counter to zero.
*
* @see getListAddress()
* @param None
* @return None
*/                                                             
void CADC2530::clearListAddress() {

  ((volatile ADCregisters_t*)my_pModule)->ListAddressLS = 0x0;
  ((volatile ADCregisters_t*)my_pModule)->ListAddressMS = 0x0;
}

/*==============================================================*/
/** @fn CADC2530::isBusy()
* @brief Check if the module is busy.
*
* Check is the module has accepted input pulses and that they
* are being converted.
*
* @param None
* @return True if the module is busy.
*/                                                             
bool CADC2530::isBusy() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  return (csr.bits.RSTBSY == 1); 
}

/*==============================================================*/
/** @fn CADC2530::clearFullnessFlags()
* @brief Clear the fullness flags to zero.
*
* Clear the fullness flags to zero.
*
* @param None
* @return None
*/                                                             
void CADC2530::clearFullnessFlags() {

  ((volatile ADCregisters_t*)my_pModule)->FullnessFlag = 0x0;

}

/*==============================================================*/
/** @fn CADC2530::getFullnessFlags()
* @brief Get the fullness flags.
*
* Get the fullness flags.  Note, the meaning of the
* individual bits is dependent on whether the module is in
* list or histogram mode.
*
* @param None
* @return The current fullness flags.
*/                                                             
uint16_t CADC2530::getFullnessFlags() {
  uint16_t flags = 0;
  flags = ((volatile ADCregisters_t*)my_pModule)->FullnessFlag;

  return flags;
}

/*==============================================================*/
/** @fn CADC2530::isChannelFull(uint16_t aChan)
* @brief Check if a histogram channel is full.
*
* Check if the specified histogram channel is full (overflow). Note, 
* no determination is made as whether the module is actually
* operating in histogram mode.  Also, histogram mode flags are
* shared with list mode flags.
*
* @throw String exception on error. 
* @param aChan The channel to check in the range [1-8].
* @return If the fullness flag of the specified channel is set.
*/                                                             
bool CADC2530::isChannelFull(uint16_t aChan) {
  if ((aChan < 1)||(aChan > 8)) {
    char buffer[256];
    sprintf(buffer, "CADC2530::isChannelFull(): When checking channel fullness, the channel specified must be in the range [1-8] (channel == %d).\n",aChan);
    throw string(buffer);
  }

  ADCfullnessflag_t fflags;
  fflags.flags = getFullnessFlags();
  uint8_t mask = 0x1;
  mask = (mask << (aChan - 1));
  return ((fflags.bytes.FULL & mask) > 0);
}

/*==============================================================*/
/** @fn CADC2530::channelHasData(uint16_t aChan)
* @brief Check if a histogram channel has data.
*
* Check if the specified histogram channel has data. Note, 
* no determination is made as whether the module is actually
* operating in histogram mode. Also, histogram mode flags are
* shared with list mode flags.
*
* @throw String exception on error. 
* @param aChan The channel to check in the range [1-8].
* @return If the half-fullness flag of the specified channel is set.
*/                                                             
bool CADC2530::channelHasData(uint16_t aChan) {
  if ((aChan < 1)||(aChan > 8)) {
    char buffer[256];
    sprintf(buffer, "CADC2530::channelHasData(): When checking channel fullness, the channel specified must be in the range [1-8] (channel == %d).\n",aChan);
    throw string(buffer);
  }

  ADCfullnessflag_t fflags;
  fflags.flags = getFullnessFlags();
  uint8_t mask = 0x1;
  mask = (mask << (aChan - 1));
  return ((fflags.bytes.HALFFULL & mask) > 0);
}

/*==============================================================*/
/** @fn CADC2530::isListFull()
* @brief Check if list mode memory is full.
*
* Check if list mode memory is full. Note, 
* no determination is made as whether the module is actually
* operating in list mode.  Also, List mode flags are shared with
* histogram mode flags.
*
* @param None.
* @return If the list mode fullness flag is set.
*/                                                             
bool CADC2530::isListFull() { 
  ADCfullnessflag_t fflags;
  fflags.flags = getFullnessFlags();
  uint8_t mask = 0x1;
  return ((fflags.bytes.FULL & mask) > 0);
}

/*==============================================================*/
/** @fn CADC2530::isListHalfFull()
* @brief Check if list mode memory is half full.
*
* Check if list mode memory is half full. Note, 
* no determination is made as whether the module is actually
* operating in list mode.  Also, List mode flags are shared with
* histogram mode flags.
*
* @param None.
* @return If the list mode half-fullness flag is set.
*/                                                             
bool CADC2530::isListHalfFull() { 
  ADCfullnessflag_t fflags;
  fflags.flags = getFullnessFlags();
  uint8_t mask = 0x1;
  return ((fflags.bytes.HALFFULL & mask) > 0);
}

/*==============================================================*/
/** @fn CADC2530::setSSTR(uint16_t sstrval)
* @brief Set the sliding scale test register.
*
* Set the sliding scale test register.  The first and last
* two bits are control bits and must be set to zero.  This
* method will a mask of 0x3FFC to the specified value to
* ensure the first and last two bits are set to zero before
* writing to the register.
*
* Note that a bit setting of 0 (default value) indicates that 
* the sliding scale counters for that channel will increment,
* while a setting of 1 set the sliding scale counters to zero.
*
* @param sstrval The new SSTR setting.
* @return None
*/                                                             
void CADC2530::setSSTR(uint16_t sstrval) {
  uint16_t val = (sstrval&0x3FFC);
  ((volatile ADCregisters_t*)my_pModule)->SlidingScaleTest= val;

}

/*==============================================================*/
/** @fn CADC2530::getSSTR()
* @brief Get the sliding scale test register.
*
* Get the sliding scale test register.  The first and last
* two bits are control bits.  
*
* Note that a bit setting of 0 (default value) indicates that 
* the sliding scale counters for that channel will increment,
* while a setting of 1 set the sliding scale counters to zero.
*
* @param None
* @return The current SSTR setting.
*/                                                             
uint16_t CADC2530::getSSTR() {
  uint16_t sstrval = 0;

  sstrval = ((volatile ADCregisters_t*)my_pModule)->SlidingScaleTest;

  return sstrval;
}

/*==============================================================*/
/** @fn CADC2530::hasInterrupt()
* @brief Check the CSR interrupt status.
*
* Check the control and status register interrupt status.
* A return of true indicates that a mask enabled fullness flag
* has been set.
*
* @param None
* @return True if a mask enabled fullness flag has been set.
*/                                                             
bool CADC2530::hasInterrupt() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  return (csr.bits.IS == 1); 
}

/*==============================================================*/
/** @fn CADC2530::dataReady()
* @brief Check if data ready.
*
* Check is any of the channel flags are set (data ready)
* for this module.
*
* @param None
* @return True if data ready.
*/                                                             
bool CADC2530::dataReady() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  return (csr.bits.DR == 1); 
}

/*==============================================================*/
/** @fn CADC2530::resetCSR()
* @brief Reset the CSR to zero.
*
* Reset the status register (CSR) to zero.
*
* @param None
* @return None
*/                                                             
void CADC2530::resetCSR() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  csr.bits.RSTBSY = 1;
  setCSR(csr.csrval);
  setCSR((uint16_t)0x0);
  my_eventmode = false;
}

/*==============================================================*/
/** @fn CADC2530::resetCTR()
* @brief Reset the CTR to zero.
*
* Reset the calibration and test register (CTR) to zero.
*
* @param None
* @return None
*/                                                             
void CADC2530::resetCTR() {
  setCTR((uint16_t)0x0);
}

/*==============================================================*/
/** @fn CADC2530::fastClear()
* @brief Force a fast clear on all channels.
*
* Force a fast clear on all channels.
*
* @param None
* @return None
*/                                                             
void CADC2530::fastClear() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  csr.bits.FFCLR = 1;
  setCSR(csr.csrval);
}

/*==============================================================*/
/** @fn CADC2530::setIPL(uint16_t iplval)
* @brief Set the interrupt priority level.
*
* Set the interrupt priority level.  The value specified
* should be in the range [0-7].  Only the 3 lower bits of
* the specified value are retained.
*
* @param iplval The new interrupt priority level.
* @return None
*/                                                             
void CADC2530::setIPL(uint16_t iplval) {
  ADCcsr_t csr;
  ADCcsr_t mask;

  csr.csrval = 0;
  csr.bits.IPL = (iplval & 0x07);

  mask.csrval = 0;
  mask.bits.IPL = 0x7;

  setCSRbits(csr.csrval,mask.csrval);
}

/*==============================================================*/
/** @fn CADC2530::getIPL()
* @brief Get the interrupt priority level.
*
* Get the interrupt priority level.
*
* @param None
* @return The interrupt priority level from the CSR.
*/                                                             
uint16_t CADC2530::getIPL() {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  uint16_t iplval = csr.bits.IPL;
  return iplval;
}

/*==============================================================*/
/** @fn CADC2530::setLLD(uint16_t lldval)
* @brief Set the lower level discriminator DAC value.
*
* Set the lower level discriminator (LLD) DAC value.  The value 
* specified is NOT a voltage but the computed DAC value as
* described the ADC manual.  This value can be computed from
* the desired voltage (per the manual) as:
*
*  LDD DAC value = (((Vreq/3.2764)/(0.25/4095))<<2)&0x3FFC 
*
* @param lldval The new LLD value.
* @return None
*/                                                             
void CADC2530::setLLD(uint16_t lldval) {
  ((volatile ADCregisters_t*)my_pModule)->LowerLevelDisc = lldval;

}

/*==============================================================*/
/** @fn CADC2530::getLLD()
* @brief Get the lower level discriminator DAC value.
*
* Get the lower level discriminator (LLD) DAC value.  The value 
* returned is NOT a voltage but the computed DAC value as
* described the ADC manual.  This value can be computed from
* the desired voltage (per the manual) as:
*
*  LLD DAC value = (((Vreq/3.2764)/(0.25/4095))<<2)&0x3FFC 
*
*  The value return by this method is the computed 
*  "LLD DAC value."
*
* @param None
* @return The interrupt priority level from the CSR.
*/                                                             
uint16_t CADC2530::getLLD() {
  uint16_t lldval = 0;
  lldval = ((volatile ADCregisters_t*)my_pModule)->LowerLevelDisc;

  return lldval;
}

/*==============================================================*/
/** @fn CADC2530::setULD(uint16_t uldval)
* @brief Set the upper level discriminator DAC value.
*
* Set the upper level discriminator (ULD) DAC value.  The value 
* specified is NOT a voltage but the computed DAC value as
* described the ADC manual.  This value can be computed from
* the desired voltage (per the manual) as:
*
*  ULD DAC value = ((((Vreq/3.2764)-2)/(0.25/4095))<<2)&0x3FFC 
*
* @param uldval The new ULD value.
* @return None
*/                                                             
void CADC2530::setULD(uint16_t uldval) {

  ((volatile ADCregisters_t*)my_pModule)->UpperLevelDisc = uldval;

}

/*==============================================================*/
/** @fn CADC2530::getULD()
* @brief Get the upper level discriminator DAC value.
*
* Get the upper level discriminator (ULD) DAC value.  The value 
* returned is NOT a voltage but the computed DAC value as
* described the ADC manual.  This value can be computed from
* the desired voltage (per the manual) as:
*
*  ULD DAC value = ((((Vreq/3.2764)-2)/(0.25/4095))<<2)&0x3FFC 
*
*  The value return by this method is the computed
*  "ULD DAC value."
*
* @param None
* @return The interrupt priority level from the CSR.
*/                                                             
uint16_t CADC2530::getULD() {
  uint16_t uldval = 0;

  uldval = ((volatile ADCregisters_t*)my_pModule)->UpperLevelDisc;

  return uldval;
}

/*==============================================================*/
/** @fn CADC2530::setInterruptVector(uint16_t ivect)
* @brief Set the interrupt vector.
*
* Set the interrupt vector to the specified value.  
*
* @param ivect The new interrupt vector setting.
* @return None
*/                                                             
void CADC2530::setInterruptVector(uint16_t ivect) {

  ((volatile ADCregisters_t*)my_pModule)->InterruptVector = ivect;

}

/*==============================================================*/
/** @fn CADC2530::getInterruptVector()
* @brief Get the interrupt vector.
*
* Get the current interrupt vector setting.
*
* @param None
* @return The current interrupt vector setting.
*/                                                             
uint16_t CADC2530::getInterruptVector() {
  uint16_t ivect = 0;

  ivect = ((volatile ADCregisters_t*)my_pModule)->InterruptVector;

  return ivect;
}

/*==============================================================*/
/** @fn CADC2530::setInterruptMask(uint16_t imask)
* @brief Set the interrupt mask.
*
* Set the interrupt mask to the specified value.  
*
* @param imask The new interrupt mask setting.
* @return None
*/                                                             
void CADC2530::setInterruptMask(uint16_t imask) {
  ((volatile ADCregisters_t*)my_pModule)->InterruptMask = imask;

}

/*==============================================================*/
/** @fn CADC2530::getInterruptMask()
* @brief Get the interrupt mask.
*
* Get the current interrupt mask setting.
*
* @param None
* @return The current interrupt mask setting.
*/                                                             
uint16_t CADC2530::getInterruptMask() {
  uint16_t imask = 0;

  imask = ((volatile ADCregisters_t*)my_pModule)->InterruptMask;

  return imask;
}

/*==============================================================*/
/** @fn CADC2530::setCTRchannel(uint16_t chanval)
* @brief Set the CTR channel.
*
* Set the test channel in the calibration and test register.
* The value specified should be in the range [1-8].  
* Only the 3 lower bits of the specified value -1 are retained.
*
* @param throw String exception on error.
* @param chanval The new calibration channel.
* @return None
*/                                                             
void CADC2530::setCTRchannel(uint16_t chanval) {
  if ((chanval < 1)||(chanval > 8)) {
    char buffer[256];
    sprintf(buffer, "CADC2530::setCTRchannel(): The specified channel value must be in the range [1-8]\n");
    throw string(buffer);
  }
  
  ADCctr_t ctr;
  ADCctr_t mask;

  ctr.ctrval = 0;
  ctr.bits.M012 = ((chanval-1) & 0x07);

  mask.ctrval = 0;
  mask.bits.M012 = 0x7;

  setCTRbits(ctr.ctrval,mask.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::getCTRchannel()
* @brief Get the CTR channel.
*
* Get the test channel from the calibration and test register 
* (CTR).  The value returned is the CTR setting +1 to give
* a channel value in the range [1-8].
*
* @param None
* @return The calibration channel from the CTR.
*/                                                             
uint16_t CADC2530::getCTRchannel() {
  ADCctr_t ctr;
  ctr.ctrval = getCTR();
  uint16_t chanval = ctr.bits.M012;
  return (chanval+1);
}

/*==============================================================*/
/** @fn CADC2530::enableAutoFastClear()
* @brief Enable auto fast clear.
*
* Enable auto fast clear (DEAC flag in the CTR). 
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableAutoFastClear() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.DEAC = 1;
  resetCTRbits(ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::disableAutoFastClear()
* @brief Disable auto fast clear.
*
* Disable auto fast clear (DEAC flag in the CTR).
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableAutoFastClear() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.DEAC = 1;
  setCTRbits(ctr.ctrval,ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::enableCompensation()
* @brief Enable compensation.
*
* Enable compensation (DISC flag in the CTR). 
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableCompensation() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.DISC = 1;
  resetCTRbits(ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::disableCompensation()
* @brief Disable compensation.
*
* Disable compensation (DISC flag in the CTR).
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableCompensation() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.DISC = 1;
  setCTRbits(ctr.ctrval,ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::enableMUXswitch()
* @brief Enable CTR MUX switch.
*
* Enable CTR MUX switch (MEN flag in the CTR).   This opens the
* channel gate and sets the MUX switch on PCB to allow voltage
* at the channel input to be measured.  Call after setting the
* CTR channel (setCTRchannel());
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::enableMUXswitch() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.MEN = 1;
  setCTRbits(ctr.ctrval,ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::disableMUXswitch()
* @brief Disable CTR MUX switch.
*
* Disable CTR MUX switch (MEN flag in the CTR). This closes the
* channel gate and sets the MUX switch on PCB, preventing voltage
* at the channel input from being measured.
* Intended for calibration and testing.
*
* @param None
* @return None
*/                                                             
void CADC2530::disableMUXswitch() {
  ADCctr_t ctr;
  ctr.ctrval = 0;
  ctr.bits.MEN = 1;
  resetCTRbits(ctr.ctrval);
}

/*==============================================================*/
/** @fn CADC2530::getCSR()
* @brief Return the current CSR setting.
*
* Return the current control and status register (CSR) setting.
*
* @param None
* @return The current CSR setting.
*/                                                             
uint16_t CADC2530::getCSR() {
  uint16_t csrval = 0;

  csrval = ((volatile ADCregisters_t*)my_pModule)->CSR;

  return csrval;
}

/*==============================================================*/
/** @fn CADC2530::setCSR(uint16_t csrval))
* @brief Set the CSR to a specific value.
*
* Set the control and status register (CSR) to a specific value.
* Note that if DR or FHE is set in csrval, then the other bit
* will also be set.
*
* @param csrval The new CSR value.
* @return None
*/                                                             
void CADC2530::setCSR(uint16_t csrval) {
  ADCcsr_t csr;
  csr.csrval = csrval;
  if (csr.bits.DR) my_eventmode = true;
  else my_eventmode = false;


  ((volatile ADCregisters_t*)my_pModule)->CSR = csrval;

}

/*==============================================================*/
/** @fn CADC2530::setCSRbits(uint16_t nBits,uint16_t nMask)
* @brief Set specified bits in the CSR.
*
* Set specified bits in the control status register (CSR).
* The arguments passed to this method are a set of bits that
* should be set in the CSR and a mask (of 1's) indicating
* which bits are affected.  
*
* @param nBits The CSR bits to set.
* @param nMask Mask of the bit set affected (1-> bit affected).
* @return None
*/                                                             
void CADC2530::setCSRbits(uint16_t nBits,uint16_t nMask) {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  csr.csrval = (csr.csrval & CADC2530_CSRMASK);
  if (my_eventmode) csr.bits.DR = 1;
  csr.csrval = (csr.csrval & (~nMask));
  csr.csrval = (csr.csrval | nBits);


  ((volatile ADCregisters_t*)my_pModule)->CSR = csr.csrval;

}

/*==============================================================*/
/** @fn CADC2530::resetCSRbits(uint16_t nBits))
* @brief Reset specified bits in the CSR.
*
* Reset specified bits in the control status register (CSR).
* The argument passed to this method is a set of bits that
* should be reset in the CSR.  This method will not affect
* bits masked by the CADC2530_CSRMASK define (e.g., RST,
* FFCLR and read-only bits).
*
* @param nBits The CSR bits to reset.
* @return None
*/                                                             
void CADC2530::resetCSRbits(uint16_t nBits) {
  ADCcsr_t csr;
  csr.csrval = getCSR();
  csr.csrval = (csr.csrval & CADC2530_CSRMASK);
  if (my_eventmode) csr.bits.DR = 1;
  csr.csrval = (csr.csrval & (~nBits));


  ((volatile ADCregisters_t*)my_pModule)->CSR = csr.csrval;

}

/*==============================================================*/
/** @fn CADC2530::getCTR()
* @brief Return the current CTR setting.
*
* Return the current calibration and test register (CTR) setting.
*
* @param None
* @return The current CTR setting.
*/                                                             
uint16_t CADC2530::getCTR() {
  uint16_t ctrval = 0;

  ctrval = ((volatile ADCregisters_t*)my_pModule)->CTR;

  return ctrval;
}

/*==============================================================*/
/** @fn CADC2530::setCTR(uint16_t ctrval))
* @brief Set the CTR to a specific value.
*
* Set the calibration and test register (CTR) to a specific value.
*
* @param ctrval The new CTR value.
* @return None
*/                                                             
void CADC2530::setCTR(uint16_t ctrval) {

  ((volatile ADCregisters_t*)my_pModule)->CTR = ctrval;

}

/*==============================================================*/
/** @fn CADC2530::setCTRbits(uint16_t nBits,uint16_t nMask)
* @brief Set specified bits in the CTR.
*
* Set specified bits in the calibration and test register (CTR).
* The arguments passed to this method are a set of bits that
* should be set in the CTR and a mask (of 1's) indicating
* which bits are affected.  
*
* @param nBits The CST bits to set.
* @param nMask Mask of the bit set affected (1-> bit affected).
* @return None
*/                                                             
void CADC2530::setCTRbits(uint16_t nBits,uint16_t nMask) {
  uint16_t ctrval = getCTR();
  ctrval = (ctrval & CADC2530_CTRMASK);
  ctrval = (ctrval & (~nMask));
  ctrval = (ctrval | nBits);

  ((volatile ADCregisters_t*)my_pModule)->CTR = ctrval;

}

/*==============================================================*/
/** @fn CADC2530::resetCTRbits(uint16_t nBits))
* @brief Reset specified bits in the CTR.
*
* Reset specified bits in the calibration and test register (CTR).
* The argument passed to this method is a set of bits that
* should be reset in the CTR. 
*
* @param nBits The CTR bits to reset.
* @return None
*/                                                             
void CADC2530::resetCTRbits(uint16_t nBits) {
  uint16_t ctrval = getCTR();
  ctrval = (ctrval & CADC2530_CTRMASK);
  ctrval = (ctrval & (~nBits));

  ((volatile ADCregisters_t*)my_pModule)->CTR = ctrval;

}

