#ifndef CCAENV830MODULE_H
#define CCAENV830MODULE_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#include "CDigitizerModule.h"

using namespace std;



// Forward Class Definitions:

class CCAENV830;

/*!
  This class is a TCL Configuration manager for a CAEN v830
  scaler (CCAENV830).  The configuration commands currently 
  understood are:

  - base  (integer) Base address of the module if it is not setup
          via geographical addressing.  (geo false).
  - slot  (integer) The slot number of the module if it is being
          setup via geographical addressing (geo true).
  - geo   (boolean) true if the module is being setup geographically,
          false if the module is being setup using base-addressing.
  - crate (integer) The crate number the module is in (defaults 0) 
  - enables (integer[32]) an array of flags for each channel,
           each flag is 0 to disable a channel and 1 to enable it.
  - trigger (integer) an integer encoding of the trigger mode
          as follows. 0 Disable, 1 Random 2 Periodic.
  - wide  (boolean) If true, the module is read in 32 bit wide mode
         otherwise in 26 bit mode.
  - header (boolean) If true, the module will have a header,
         otherwise not.
  - autoreset (boolean) If true, the scalers reset after being
         latched via a trigger.
  - fpclearmeb (boolean) If true, the scaler MEB is cleared on 
         a front panel clear (false, only the counters are cleared).
  - id   (integer)  The id of the packet created by the module
         (see below).
  - manualclear (boolean) If true, the scalers are manually reset 
         via the clear function after the MEB is read out.
  - packetize (boolean)  If true, read inserts additional packetization
         information in the readout stream. This is intended for use when
	 the module is being used to supply event parameters (e.g. timing).
  - vmetrigger (boolean) If true, read does a vme trigger of the modules and then
         does the read.  Requires trigger to be set to random.
  - parameters (string[32]) Set of parameter names associated with each channel
         of the module.  This is used to setup the scaler display etc.



   Since the module has modes in which it is not possible to 
   mechanically decode it, if packetize is true,
   Read will insert additional information
   into the event stream:
   - The module will be an identified packet.. It will have a 
     word count and an id (set by the id config parameter).
   - The module will include a word of  mode information as 
     as follows:
\verbatim
     +--------------------------------+
     |H|W|  Slot (5) |X|  Crate (8)   | 
     +-+-+-----------+-+--------------+
      ^ ^     ^       ^     ^
      | |     |       |     +--------- 8 bits of module crate num.
      | |     |       +--------------- 1 unused bit.
      | |     +----------------------- 5 bits of Geographical addr.
      | +----------------------------- 1 bit set if wide readout.
      +------------------------------- 1 bit set if header enabled.

\endverbatim
    - If the W bit is set, an additional long word channel mask
      will be inserted in the data stream.  This long word will
      be in little endian order (low order word first).  Each
      bit of the longword represents a scaler channel and will be
      set if the channel is present. Note that the channels are 
      then placed in the buffer from low channel number to high.
      In little endian word order.

*/
class CCAENV830Module : public  CDigitizerModule
{
  // Data:

  CCAENV830* m_pModule;		//!< The module hardware driver.
  int        m_nId;		//!< Packet id.
  bool       m_fHeader;		//!< True if headers enabled.
  bool       m_fWide;		//!< True if module reads are wide.
  bool       m_fManualClear;	//!< True if manual clear needed.
  bool       m_fPacketize;	//!< True if should add packet info.
  bool       m_fVmeTrigger;	//!< True if trigger via VME.
  unsigned long m_lEnables;	//!< Channel enables mask.
  int        m_nCrate;		//!< Crate of module.
  int        m_nSlot;		//!< Slot of module.

public:
  // Constructors and other canonicals:

  CCAENV830Module(const std::string& rName, CTCLInterpreter& rInterp);
  virtual ~CCAENV830Module();
private:
  CCAENV830Module(const CCAENV830Module& rhs);
  CCAENV830Module& operator= (const CCAENV830Module& rhs);
  int        operator!=(const CCAENV830Module& rhs);
  int        operator==(const CCAENV830Module& rhs);
public:

  // class operations:

public:

  virtual void Initialize();
  virtual void Prepare();
  virtual int  Read(void* pBuffer);
  virtual void Clear();
  virtual std::string getType() const {
    return std::string("caenv830");
  }

};



#endif
