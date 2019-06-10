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

#include "CVMUSBReadoutList.h"

using namespace std;

// The following are bits in the mode word that leads off each stack line.
//

static const int modeAMMask(0x3f); // Address modifier bits.
static const int modeAMShift(0);

static const int modeDSMask(0xc0);
static const int modeDSShift(6);

static const int modeNW(0x100);
static const int modeNA(0x400);
static const int modeMB(0x800);
static const int modeSLF(0x1000);
static const int modeBE(0x10000);
static const int modeHD(0x20000);
static const int modeND(0x40000);
static const int modeHM(0x80000);
static const int modeNTMask(0xc00000);
static const int modeNTShift(22);
static const int modeBLTMask(0xf0000000);
static const int modeBLTShift(24);

// The following bit must be set in the address stack line for non long
// word transfers:

static const int addrNotLong(1);

/////////////////////////////////////////////////////////////////
//  Constructors and canonicals.

/*!
   Constructing a CVMUSBReadoutList is not really doing anything.
   the vector constructs itself by default.
*/
CVMUSBReadoutList::CVMUSBReadoutList()
{}

/*!
   Copy construction requires a copy construction of the list data.
   without this, the bit by bit copy would be very bad.
*/
CVMUSBReadoutList::CVMUSBReadoutList(const CVMUSBReadoutList& rhs) :
    m_list(rhs.m_list)
{
}
/*!
   Destruction is also a no-op but is provided for the sake of subclasses.
*/
CVMUSBReadoutList::~CVMUSBReadoutList()
{}

/*!
   Comparison is based on  comparison of the list contents.
*/
int
CVMUSBReadoutList::operator==(const CVMUSBReadoutList& rhs) const
{
    return m_list == rhs.m_list;
}
int
CVMUSBReadoutList::operator!=(const CVMUSBReadoutList& rhs) const
{
    return !(*this == rhs);
}

////////////////////////////////////////////////////////////////////
//    Operations on the whole of the list:

/*!
   Clear the list.
*/
void
CVMUSBReadoutList::clear()
{
    m_list.clear();
}
/*!
   return the size of the list in longwords.
*/
size_t
CVMUSBReadoutList::size() const
{
    return m_list.size();
}
/*!
     Return a copy of the list itself:
*/

vector<uint32_t>
CVMUSBReadoutList::get() const
{
   return m_list;
}
/////////////////////////////////////////////////////////////////////
//  register operations.

/*!
    Add a register read to a stack.  Register reads are just like
    single shot 32 bit reads but the SLF bit must be set in the 
    mode line.  I am assuming that we don't need stuff like the 
    DS bits either, as those are significant only for VME bus.
   \param address  : unsigned int
       Register address, see 3.4 of the VM-USB manual for a table
       of addresses.
*/
void
CVMUSBReadoutList::addRegisterRead(unsigned int address)
{
    uint32_t mode = modeNW  | modeSLF;
    m_list.push_back(mode);
    m_list.push_back(address);
	
}
/*!
   Add a register write to a stack. Register writes are the same
   as register reads except:
   - There is a data longword that follows the address.
   - The mode word does not have modeNW set.

   \param address : unsigned int
       Address of the register.  See VM-USB manual section 3.4 for
       a table of addresses.
   \param data    : uint32_t 
      The data to write.  Note that some registers are only 16 bits
      wide.  In these cases, set the low order 16 bits of the data.
*/
void
CVMUSBReadoutList::addRegisterWrite(unsigned int address,
				    uint32_t     data)
{
    uint32_t mode = modeSLF;
    m_list.push_back(mode);
    m_list.push_back(address);
    m_list.push_back(data);

}

////////////////////////////////////////////////////////////////////////////////
// 
// Single shot VME write operations:

/*!
   Add a single 32 bit write to the list.  Any legitimate non-block mode
   address modifier is acceptable.   This version has the following restrictions:
   - No checking for suitability of the address modifier is done.  E.g. doing a
     MBLT64 transfer amod is allowed although will probably not work as expected
     when the list is executed.
   - The address must be longword aligned, however this is not checked.
     Instead, the address will have the bottom 2 bits set to 0.
     The VM-USB does not support marshalling the longword into an appropriate
     multi-transfer UAT, and neither do we.
 
   \param address : uint32_t
       The address to which the data are transferred. It is the caller's
       responsibility to ensure that this is longword aligned.
   \param amod   : uint8_t
       The 6 bit address modifier for the transfer.  Extraneous upper bits
       will be silently masked off.
    \param datum : uint32_t
       The data to transfer to the DTB slave.
*/
void
CVMUSBReadoutList::addWrite32(uint32_t address, uint8_t amod, uint32_t datum)
{
  // First we need to build up the stack transfer mode longword:

  uint32_t mode = (static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask;
  m_list.push_back(mode);

  // Now the address and data.. the LWORD* bit will not be set in the address

  m_list.push_back(address & 0xfffffffc); // The longword aligned address.
  m_list.push_back(datum);	          // data to write.
}
/*!
   Add a single 16 bit word write to the list.  Any legitimate non-block mode
   address modifier is acceptalbe.   This version has the following restrictions:
   - No checking for address modifier suitability is done.
   - The address must be word aligned.  The bottom bit of the address will be
     silently zeroed if this is not the case.  

   \param address : uint32_t
       Address to which the data are transferred.  
   \param amod : uint8_t
       The 6 bit address modifier for the transfer.  Extraneous bits are
       silently masked off.
   \param datum : uint16_t
       The data word to transfer.
*/
void
CVMUSBReadoutList::addWrite16(uint32_t address, uint8_t amod, uint16_t datum)
{
  // Build up the mode word... no need to diddle with DS0/DS1 yet.

  uint32_t mode = (static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask;
  m_list.push_back(mode);

  // Now the address and data.  The thing that characterizes a non-longword
  // transfer is that the LWORD* bit must be set in the address.
  // Both data strobes firing is what makes this a word transfer as opposed
  // to a byte transfer (see addWrite8 below).

  m_list.push_back((address & 0xfffffffe) | addrNotLong);
  m_list.push_back(static_cast<uint32_t>(datum));

}
/*!
   Add a single 8 bit write to the list. 
   - No checking for the address modifier is done, and any extraneous
     bits in it are silently discarded.

  Note that since this is a byte write, no alignment restrictions apply.
  The data strobes that fire reflect the byte number.
  \param address : uint32_t
      The address to which the data will be written.
  \param amod : uint8_t
      The 6 bit address modifier code.
  \param datum : uint8_t
      The byte to write.
*/
void
CVMUSBReadoutList::addWrite8(uint32_t address, uint8_t amod, uint8_t datum)
{
  // The data strobes depend on the bottom 2 bits of the address.
  // for an even address, DS1 is disabled.
  // for an odd address, DS0 is disabled.

  uint32_t laddr = (address & 0xfffffffe) | addrNotLong;
  uint32_t mode  = (static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask;
  mode |= dataStrobes(address);

  m_list.push_back(mode);
  m_list.push_back(laddr);

  // The claim is that the data must be shifted to the correct lane.
  // some old code I have does not do this.. What I'm going to do so I 
  // don't have to think too hard about whether or not this is correct
  // is to put the data byte on both D0-D7 and D8-D15, so it does not
  // matter if 
  // - an even byte is being written, or odd
  // - the data has to or does not have to be in the appropriate data lanes.
  //

  uint32_t datum16 = (static_cast<uint32_t>(datum)); // D0-D7.
  datum16         |= (datum16 << 8);               // D8-D15.

  m_list.push_back(datum16);
  
}

////////////////////////////////////////////////////////////////////////////////
//
// Single shot VME Read operations.

/*!
   Add a 32 bit read to the list.  Pretty much like addWrite32, but there is no
   datum to transfer:
   - Address modifier is not checked for validity and extraneous bits are
     silently removed.
   - The transfer must be longword aligned, but is not error checked for that.
   \param address : uint32_t
      Address to which to transfer the data.
   \param amod   : uint8_t
      The address modifier to be associated with the transfer.
*/
void
CVMUSBReadoutList::addRead32(uint32_t address, uint8_t amod)
{
  uint32_t mode = modeNW | ((static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask);
  m_list.push_back(mode);
  m_list.push_back(address & 0xfffffffc);
}
/*!
   Add a 16 bit read to the list.  Pretty much like addWrite16, but there is no
   datum to transfer:
   - The address modifier is not checked for validity, and extraneous bits are
     silently discarded.
   - Data must be word aligned, but this is not checked for validity. 
   \param address : uint32_t
      The transfer address.
   \param amod : uint8_t
      Address modifier gated on the bus for the transfer.
*/
void 
CVMUSBReadoutList::addRead16(uint32_t address, uint8_t amod)
{
  uint32_t mode = modeNW | ((static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask);
  m_list.push_back(mode);
  m_list.push_back((address & 0xfffffffe) | addrNotLong);
}
/*!
   Add an 8 bit read to the list.  Note that at this time, I am not 100%
   sure which data lanes will hold the result.  The VME spec is 
   contradictory between the spec and the examples, I \em think,
   even bytes get returned in D0-D7, while odd bytes in D8-D15.
   When this is thoroughly tested, this comment must be fixed.
   \param address : uint32_t
      The transfer address
   \param amod : uint8_t 
      The address modifier. No legality checking is done, and any
      extraneous bits are silently discarded.
*/
void
CVMUSBReadoutList::addRead8(uint32_t address, uint8_t amod)
{
  uint32_t mode = modeNW | ((static_cast<uint32_t>(amod) << modeAMShift) & modeAMMask);
  mode         |= dataStrobes(address);
  m_list.push_back(mode);
  m_list.push_back((address & 0xfffffffe) | addrNotLong);
}

//////////////////////////////////////////////////////////////////////////////////
//
// Block transfer operations.
//  

/*!
   Add a 32 bit block read to the list.  There are several requirements
   in this version for simplicity.  None of these requirements are actively
   enforced.
   -  The base address must be longword aligned.
   - The address modifier must be one of the block transfer modes e.g.
     CVMUSBReadoutList::a32UserBlock
  
   This operation may generate more than one stack transaction, an initial
   one of complete 256 byte blocks in MB mode, and a final partial block
   transfer if necessary.
   \param baseAddress : uint32_t
      The addreess from which the first transfer occurs.
   \param amod : uint8_t 
      The address modifier for this transfer.  Should be one of the block
      mode modifiers.
   \param transfers : size_t
      Number of \em longwords to transfer. 
*/
void
CVMUSBReadoutList::addBlockRead32(uint32_t baseAddress, uint8_t amod, 
				  size_t transfers)
{
  addBlockRead(baseAddress,  transfers, 
	       static_cast<uint32_t>(((amod << modeAMShift) & modeAMMask) | modeNW));
}

/*!
   Add a read from a fifo.  This is identical to addBlockRead32, however
   the NA bit is set in the initial mode word to ensure that the actual
   address is not incremented.
*/
void
CVMUSBReadoutList::addFifoRead32(uint32_t address, uint8_t amod, size_t transfers)
{
  addBlockRead(address,  transfers, static_cast<uint32_t>(modeNA | 
	                                ((amod << modeAMShift) & modeAMMask) |
	                                modeNW));
}
///////////////////////////////////////////////////////////////////////////////////
//
// Private utility functions:

// Figure out the data strobes for a byte address:

uint32_t
CVMUSBReadoutList::dataStrobes(uint32_t address)
{
  uint32_t dstrobes;
  if (address & 1) {
    // odd address:

    dstrobes = 1;
  }
  else {
    dstrobes = 2;
  }
  return (dstrobes << modeDSShift);
}
// Add an block transfer.  This is common code for both addBlockRead32
// and addFifoRead32.. The idea is that we get a starting mode word to work
// with, within that mode we will fill in things like the BLT field, and the
// MB bit as needed.
//
void
CVMUSBReadoutList::addBlockRead(uint32_t base, size_t transfers, 
				uint32_t startingMode)
{
  // There are two cases, transfers are larger than a block,
  // or transfers are le a block.
  // The first case requires an MB and possibly a single BLT transfer.
  // the secod just a BLT...
  // The maximum number of transfers in a block is 256/sizeof(uint32_t)
  // Regardless, the base address is block justified.:

  base     &= 0xffffff00;		// Block justify the address.
  size_t  fullBlocks   = transfers/(256/sizeof(uint32_t));
  size_t  partialBlock = transfers % (256/sizeof(uint32_t));

  if (fullBlocks) {
    uint32_t mode = startingMode;
    mode         |= modeMB;	// Multiblock transfer.
    mode         |= (256/sizeof(uint32_t)) << modeBLTShift; // Full block to xfer.
    m_list.push_back(mode);
    m_list.push_back(base);	// Not sure this order is correct...
    m_list.push_back(fullBlocks);
   
    base += fullBlocks * 256;	// Adjust the base address in case there are partials.
    
  }
  if (partialBlock) {
    uint32_t   mode = startingMode;
    mode           |= (partialBlock) << modeBLTShift;
    m_list.push_back(mode);
    m_list.push_back(base);
  }

}
