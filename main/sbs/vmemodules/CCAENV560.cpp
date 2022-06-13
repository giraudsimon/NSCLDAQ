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
#include "CCAENV560.h"
#include "CCAENV560Registers.h"
#include <CVMEInterface.h>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

using namespace std;



// The 'macros' below are used to do structure like offsets
// using the register structure through VmeModule:
//

#define GetShort(field) (m_pModule->peekw(SOffset(CAENV560Registers,field)))
#define GetLong(field)  (m_pModule->peekl(LOffset(CAENV560Registers,field)))
#define PutShort(field,value) (m_pModule->pokew((value),  \
                                  SOffset(CAENV560Registers,field)))
#define PutLong(field,value)  (m_pModule->pokel((value), \
                                  LOffset(CAENV560Registers,field)))


static const uint16_t Manufacturer(2);
static const uint16_t ModelNumber(0x18);
static const uint16_t FixedCode(0xfaf5);

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
    "CCAENV560::%s Invalid channel must be in [0,15] %d";
  if( (nChannel < 0) || (nChannel > 15)) {
    char msg[100 + strlen(pFunction) + strlen(pFmtString)];
    sprintf(msg, pFmtString, pFunction, nChannel);
    throw string(msg);
  }
}
static inline void ThrowIfBadSection(const char* pFunction,
	       			   int nSection) 
{
   const char* pFmtString=
     "CCAENV560::%s Invalid section must be in [0,7] %d";
   if( (nSection < 0) || (nSection > 7)) {
     char msg[100 + strlen(pFunction) + strlen(pFmtString)];
     sprintf(msg, pFmtString, pFunction, nSection);
     throw string(msg);
   }
}
/*!
    Make a new CCAENV560 module.  This involves:
    - Mapping the proposed module.
    - Ensuring the module is a V560.
    - Initializing the module to a known initial state.
   \param base (in):
      The module base address as set in it's rotary switches.
   \param crate (in):
      The VME crate the module lives in.  This defaults to zero.

\note  - The parameter order changed from the original KSU
         contribution because you ought to be able to 
         construct without a crate number if you are in crate 0...
	 and the base should not be optional.

*/
CCAENV560::CCAENV560(uint32_t base, int crate) 
  :
  m_nCrate(crate),
  m_nBase(base),
  m_pModule(0)
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
   Destroy a VME module; the address map created is deleted.
*/
CCAENV560::~CCAENV560()
{
  UnmapModule();		// Destroy mapping info.
}

// Functions that operate on the module:



// Utility functions:

/*!
   Get the scale status (bit=0, two 32-bit scales, bit=1, one 64 bit) of 
   one of the 8 sections of the module; each representing a pair of scalers.
   \param nSection (in):
      Number of section to check status of [0-7].
*/
bool
CCAENV560::GetSectStat(int nSection) 
{
  ThrowIfBadSection("GetSectStat", nSection);
  short nStat  = GetShort(SSsr);
  return ((nStat & SSRMask[nSection]) != 0);
}

/*!  
  Increment all counters by 1. Simple VME access (r or w) to address.
 */
void
CCAENV560::ScalInc()
{
  short nDummy = GetShort(SInc);
}

/*!
  Set the VME VETO, preventing counters from counting. Simple VME access.
*/
void
CCAENV560::SetVeto()
{
  short nDummy = GetShort(VMEVS);
}

/*!
  Reset (clear) the VME VETO, allowing the counters to count.  Simple VME
  access.
*/
void
CCAENV560::ResetVeto() 
{
  short nDummy = GetShort(VMEVR);
}

/*!
   Clear all scalers, remove VME interrupt request if asserted, disable VME
   interrupt generation.  Simple VME access.
*/
void
CCAENV560::Clear()
{
  short nDummy = GetShort(SClear);
}


/*!
   Get latched status of VETO from last counter read
*/
bool
CCAENV560::GetVetoStat()
{
  return (Vetostat(GetShort(Ilvr)) != 0);
}

/*!
   Map the vme module.  This involves:
   - Creating a CVmeModule object after validating the crate number.
   - Retrieving the following information from the module:
     #  Module type
     #  Module serial number
     #  Module version

*/
void
CCAENV560::MapModule() 
{
  // Validate the member data:

  if(m_nCrate < 0) {
    ThrowIntString("CCAENV560::MapCard - invalid crate %d",
		   m_nCrate);
  }
  

  // Now create the 'permanent' register map.
  
  m_pModule = new CVmeModule(CVmeModule::a32d32,
			     m_nBase,
			     sizeof(CAENV560Registers),
			     m_nCrate);

  // Ensure that this module is really a V560:
  // by requiring the fixed code and correct mfrmod register
  //
  m_nModuleType = MIModTyp(GetShort(Mfrmod));
  uint16_t mfgr  = MIMfrNo(GetShort(Mfrmod));
  uint16_t fCode = GetShort(FCode);

  if ((m_nModuleType != ModelNumber)   ||
      (mfgr          != Manufacturer)  ||
      (fCode         != FixedCode)) {
    delete m_pModule;
    ThrowIntString("CCAENV560::MapCard - Module at base %x not a CAEN V560",
		   m_nBase);
  }
  // Store the rest of the stuff away.. we can no longer fail.

  m_nSerialno = MISN(GetShort(VerSN));
  m_nVersion  = MIVer(GetShort(VerSN));

}
/*!
   Setup the initial module state.  The default setup is:
   - Veto is set (counting is disabled).
   - Clear all counters, clear and disable interrupt generation.
*/
void
CCAENV560::InitModule()
{
  // Set module to power up settings (disable counting), and clear the data

  SetVeto();
  Clear();

}
/*!
  Destroy any memory map held on the module:
*/
void
CCAENV560::UnmapModule()
{
  delete m_pModule;
  m_pModule = 0;
}
/*!
   Read an individual instantaneous scaler channel.
   \param nChannel (int in):
      Number of channel to read must be in the range [0,15].
   \return int
      Instantaneous value of the counter for the selected channel.
   \throw string
      If nChannel is invalid, an explanatory string exception
      is thrown.
*/
int
CCAENV560::ReadCounter(int nChannel) 
{
  if((nChannel < 0) || (nChannel > 15)) {
    char text[1000];
    string msg;
    sprintf(text, "Invalid channel %d (must be in [0,15] in %s\n",
	    nChannel, "CCAENV560::ReadCounter");
    msg = text;
    throw msg;
  }
  return GetLong(Counters[nChannel]);
}
