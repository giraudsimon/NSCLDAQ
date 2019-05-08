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


#define DMA_THRESHOLD 34*20*4	// Don't allow it to go DMA.

#include <config.h>
#include "CCAENChain.h"
#include "CAENcard.h"
#include <CVMEInterface.h>

#include <SBSBit3API.h>


#include <stdio.h>




using namespace std;



static const int QualifyingFirmware(0x0904);


static int fwMajor(int firmware) 
{
  return (firmware >> 8) & 0xff;
}
static int fwMinor(int firmware) 
{
  return firmware & 0xff;
}
/*!
   Construct a chain.  The chain can consist either of a
   set of slots defined by [nFirstSlot, nLastSlot] or it can
   consist of a set of cards described by a vector of base addresses
   Note that chains must either be entirely geographically addressed
   or physically addressed.  This is a software restriction.
   
   \param nFirstSlot (int in):
      The first geographical address in the chain. If the
      geo is true, this represents a physical slot, otherwise a
      virtual slot that is programmed into the chain members.
   \param nLastSlot (int in):
      The last geographical address in the chain.  If the geo
      parameter is true, this represents a physical slot, otherwise
      a virtual slot that is programmed into the chain members.
   \param vBases (vector<uint32_t>& in):
      A set of base addresses for the modules in the chain.
      This is only important in the geo parameter is false.
      In that case, there must be nLastSlot - nFirstSlot + 1 entries
      in the vector that represent the base addresses at which
      the non-geographical modules live.
   \param nCrate (int in default = 0):
      Identifies the vme crate in which the chain lives.  Chains
      may not span VME crates.  This is a hardware restriction.
   \param geo (bool [in]):
      - true  - The modules are adressed geographicall, and the 
                nBases parameter is effectively ignored.
     -  false - The modules are addressed by the base addresses
                in vBases and the geographical addresses are just
		programmed into the modules as virtual slots.
   \throw string
      A descriptive error message should there be any problems 
      creating the chain.  The CAENmodules created up to that 
      point will be properly destroyed if an exception is thrown.
*/
CCAENChain::CCAENChain(int nFirstSlot, int nLastSlot,
		       vector<uint32_t>& vBases,
		       int nCrate, bool geo) 
      : m_nCBLTAddress(0),
	m_nCrate(nCrate),
	m_pHandle(0),
	m_nMaxBytes(0)
{

  // Require legitimate length of chain:

  int nModules = (nLastSlot - nFirstSlot + 1);
  if(nModules < 2) {
    throw string("Chains must have at least two modules!!");
  }
  // If not geo, require sufficient bases:

  if(!geo & (vBases.size() != nModules)) {
    throw string("Insufficient module base addresses in CAENChain");
  }
  

  // Create the modules in the chain.
  // 
  int i = 0;
  try {
    for(int slot = nFirstSlot; slot <= nLastSlot; slot++, i++) {
      m_vCards.push_back(new CAENcard(slot, nCrate, geo,
				      geo ?  0 : vBases[i]));
    }
  } 
  catch(...) {			// On any exception:
    FreeModules();		// Free the modules already created
    throw;			// and rethrow.
  }

  // All of the modules in the chain must be at firmware 9.04 or higher
  // at least.  This corresponds to a firmware value of
  // 0x0904 or larger.

  try {
    for (int slot = 0; slot < m_vCards.size(); slot++) {
      if (m_vCards[slot]->getFirmware() < QualifyingFirmware) {
	uint32_t base    = m_vCards[slot]->getBase();
	int           fw      = m_vCards[slot]->getFirmware();
	int           major   = fwMajor(fw);
	int           minor   = fwMinor(fw);
	int           qmajor  = fwMajor(QualifyingFirmware);
	int           qminor  = fwMinor(QualifyingFirmware);
	char errorMessage[1000];
	sprintf(errorMessage,
		"All modules in the chain must have firmware rev at least %d.%d, the module base address 0x%08lx (slot %d) has firmware rev %d.%d",
		qmajor, qminor,
		(long unsigned int)base, m_vCards[slot]->getSlot(), major, minor);
	throw(string(errorMessage));

      }
    }
  }
  catch (...) {
    FreeModules();
    throw;
  }

  //  The base address of the chain is essentially the first
  //  slot number (shifted left by 16 bits to fall into the 
  //  range of address bits covered by the MCST/CBLT bits in the
  //  module's MCST/CBLT register.
  //

  m_nCBLTAddress = nFirstSlot << 24;
  for(i = 0; i < nModules; i++) {
    CAENcard::ChainMember where(CAENcard::IntermediateInChain);
    if(i == 0) where = CAENcard::FirstInChain;
    if(i == (nModules - 1))  where = CAENcard::LastInChain;
    m_vCards[i]->SetCBLTChainMembership(nFirstSlot, where);
  }

  // Calculate the  number of bytes that can be read:
  // In cblt seems like a module returns
  //  header data trailer invalid 
  //   Data can be 32 x 32bit values.,
  //   header, trailer and invalid are both a single long.

  m_nMaxBytes = (nModules * 35 + 1) * sizeof(uint32_t);

  // The very last thing to do is open the VME crate on
  // CBLT addressing so that we can do a read(2) to read an event:

  try {
    m_pHandle = CVMEInterface::Open(CVMEInterface::CBLT, nCrate);

    CSBSBit3VmeInterface::SetDMABlockTransfer(m_pHandle, true);
    CSBSBit3VmeInterface::SetDMAThreshold(m_pHandle, DMA_THRESHOLD);

  }
  catch (...) {
    FreeModules();
    throw;
  }
				   
  
}
/*!
   Destroy the  chain.  This involves:
   - Freeing all the modules.
   - closing the vme interface handle:
*/
CCAENChain::~CCAENChain() 
{
  FreeModules();
  CVMEInterface::Close(m_pHandle);
}
/*!
    Select a module from the chain.
    \param index (int in):
      Index of the module to select. 
    \return CAENCard* pointer to the module selected.
      The intent is that the user can then configure the module
      appropriately.
    \throw CRangeError
      If the index is out of range.
*/
CAENcard*
CCAENChain::operator[](int index) 
{
  if( (index >= 0) && (index < m_vCards.size())) {
    return m_vCards[index];
  }
  else {
    throw CRangeError(0, m_vCards.size(), index,
		      "Selecting a CAENcard from a chain");
  }

}
/*!
  Read an event from the chain.  This is done by issuing a
  read of m_nMaxBytes long.  The cards set themselves up in a 
  chain to Buserr on end of data so the actual number of bytes
  read will, in general, be fewer.

  \param pBuffer (void* out):
     User buffer into which the chain is read.
  \return int
     Number of words actually read.

   \note   This function does not detect buffer overruns.
           A good way to declare the user buffer is either:
	   \verbatim

	   CCAENChain  chain(...);
	   ...
	   uint32_t buffer[chain.getMaxBytes()/sizeof(uint32_t)];
	   ...
	   ReadEvent(buffer);

	   \endverbatim
	   
	   Or of course:
	   \verbatim

	   ...
	   uint32_t* pBuffer = new int32_t[chain.getMaxByte()/sizeof(uint32_t)];
	   ...
	   ReadEvent(pBuffer);

	   \endverbatim

 */
int
CCAENChain::ReadEvent(void* pBuffer)
{
  int nCards = m_vCards.size();
  int nRead = 0;
  try {
    nRead = CVMEInterface::Read(m_pHandle,
			m_nCBLTAddress,
			pBuffer, m_nMaxBytes);
  }
  catch (... ) {		//  Try to reset the adapter on error.
    // See if reopening fixes this (ugghhh).
    CVMEInterface::Close(m_pHandle);
    m_pHandle = CVMEInterface::Open(CVMEInterface::CBLT, m_nCrate);
#ifdef HAVE_SBSVME_INTERFACE
    CSBSBit3VmeInterface::SetDMABlockTransfer(m_pHandle, true);
    CSBSBit3VmeInterface::SetDMAThreshold(m_pHandle, DMA_THRESHOLD);    
#endif
    throw;			// But re-report the error.
  }

  return nRead/sizeof(uint16_t);
}
/*!
   Clear the data in all cards.
*/
void
CCAENChain::ClearData()
{
  int ncards = m_vCards.size();
  for(int i = 0; i < ncards; i++) {
    m_vCards[i]->clearData();
  }
}

/*!
  (utility).

  Frees the set of moudules that exist in the chain.  This
  is done both at destruction and for exceptions that occur in the
  constructor.
*/
void
CCAENChain::FreeModules()
{
  for(int i=0; i < m_vCards.size(); i++ ) {
    delete m_vCards[i];
  }
  m_vCards.erase(m_vCards.begin(), m_vCards.end());
}

/*!
   Turns on the EMPTY_PROG bit of the Bit2 control/status register on the right most module in the chain..
   When enabled, this bit implies that the last module in the chain will
   provide a header/trailer longword if it got a gate, if there is no data.
   This is intended to help unambiguously identify the end of data.

   The SBS PCI/VME interface has a deeply pipelined DMA engine.  It is not able
   to reliably deliver partial transfer counts for DMA that end in a BUSERR e.g.
   However, in most cases, doing a CBLT read will result in a partial transfer terminating 
   in a BUSERR.  

   If the modules in the chain all have  a common GATE, this ensures there are always
   a header/trailer from the last module in the chain, and that data can allow an
   unambiguous determination of the end of the chain data.  Typically one would use this
   by placing the data from the chain read in a packet.  Processing the data in the packet
   until the trailer from the last ADC in the chain, but using the packet word count
   to determine where the next chunk of data is.
*/
void 
CCAENChain::lastModuleEmptyEnable() 
{

  CAENcard* pCard = lastCard();
  pCard->emptyEnable();
}

/*!
   This turns off the EMPTY_PROG bit of the Bit2 stat of the right most module in the chain.
   See lastModuleEmptyEnable for an full discussion of  what that means.
   This is the default state (disabled).

*/
void 
CCAENChain::lastModuleEmptyDisable()
{
  CAENcard*  pCard = lastCard();
  pCard->emptyDisable();
}

/*
  Utility to return the last card in the chain (DRY).
*/
CAENcard*
CCAENChain::lastCard()
{
  // Get the pointer to the last module:

  CAENcard* pCard = m_vCards.back();
  return pCard;
}
