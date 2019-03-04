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
#include "CMemoryMappedCAMACBranch.h"


/*!
  Construction just has to save the data path width flag:
  \param 
  \param is32bitDataPath - true if the data path to the controller
                           module is 32 bits (default true).
*/
CMemoryMappedCAMACBranch::CMemoryMappedCAMACBranch(bool is32bitDataPath) :
  m_is32(is32bitDataPath)
{}


/*!  
  Control operations are implemented as a 16 bit read that
  discards the data as no data is transferred by control ops.

  \param crate      - the crate within the branch
  \param slot       - The slot within the crate.
  \param subaddress - the module subaddress within the slot.
  \param function   - the function to execute.
*/
void
CMemoryMappedCAMACBranch::control(unsigned crate, unsigned slot, unsigned subaddress,
				  unsigned function)
{
  read16(crate, slot, subaddress, function);
}

/*!
   Do a CAMAC write. this is a 24 bit (32 on the VME backplane) write 
   to a camac module.

  \param crate      - the crate within the branch
  \param slot       - The slot within the crate.
  \param subaddress - the module subaddress within the slot.
  \param function   - the function to execute.
  \param data       - the data to write.
*/
void
CMemoryMappedCAMACBranch::write(unsigned crate, unsigned slot, unsigned subaddress, 
				unsigned function, int32_t data)
{
  void* p = pointer(crate, slot, subaddress, function);

  if (m_is32) {
    int32_t* p32 = reinterpret_cast<int32_t*>(p);
    *p32 = data;
  }
  else {
    // VME is big endian so:

    int16_t* p16 = reinterpret_cast<int16_t*>(p);

    *p16++ = (data >> 16) & 0xffff; // high half.
    *p16   = (data & 0xffff);	// low half.
  }
}
/*!
   Do a 16 bit camac write.. if the width to the controller is 32 bits, it is assumed 
ther's no penalty doing a 32 bit write


  \param crate      - the crate within the branch
  \param slot       - The slot within the crate.
  \param subaddress - the module subaddress within the slot.
  \param function   - the function to execute.
  \param data       - the data to write.
*/
void
CMemoryMappedCAMACBranch::write16(unsigned crate, unsigned slot, unsigned subaddress, 
				unsigned function, int16_t data)
{
  if (m_is32) {
    write(crate, slot, subaddress, function, data);
  }
  else {
    int16_t* p = reinterpret_cast<int16_t*>(pointer(crate, slot, subaddress, function));
    *p++;			// bottom 16 bits in big endian VME space.
    *p = data;
  }
}

/*!
   Read a 24 bit (32 bit) value from the data way. The top 8 bits will always
   be set to zero.

  \param crate      - the crate within the branch
  \param slot       - The slot within the crate.
  \param subaddress - the module subaddress within the slot.
  \param function   - the function to execute.
  \return int32_t 
  \retval the value read from the module.
*/
int32_t
CMemoryMappedCAMACBranch::read(unsigned crate, unsigned slot, unsigned subaddress,
			       unsigned function)
{
  int32_t result = 0;
  void*   p = pointer(crate, slot, subaddress, function);
  if (m_is32) {
    int32_t* p32 = reinterpret_cast<int32_t*>(p);
    result = *p32;
  }
  else {
    int16_t* p16 = reinterpret_cast<int16_t*>(p);
    result       = (*p16++) << 16;
    result      |= *p16;
  }
  result &= 0xffffff;
  return result;
}

/*!
   Read a 16 bit item from the data way.  If the controller has a 32 bit
   data path leading to it, the assumption is there's no performance penalty
   to just do the 32 bit read and discard the top bits.
  \param crate      - the crate within the branch
  \param slot       - The slot within the crate.
  \param subaddress - the module subaddress within the slot.
  \param function   - the function to execute.
  \return int166_t 
  \retval the value read from the module.
*/
int16_t
CMemoryMappedCAMACBranch::read16(unsigned crate, unsigned slot, unsigned subaddress,
			       unsigned function)
{
  if (m_is32) {
    return read(crate, slot, subaddress, function);
  } 
  else {
    uint16_t* p = reinterpret_cast<uint16_t*>(pointer(crate, slot, subaddress, function));
    p++;
    return *p;
  }
}
