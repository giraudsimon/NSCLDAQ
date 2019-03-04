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
#include "CVC32CC32.h"
#include <CVMEInterface.h>

#include <stdint.h>

uint8_t* CVC32CC32::m_pBranches[CVC32CC32::BRANCHES][CVC32CC32::VMECRATES];

static const unsigned WIENER_SIZE   = (1UL << 16);
static const unsigned WIENER_SHIFT  = 16;

static const unsigned    CAMF =  2;              /* Shift count to F field. */
static const unsigned    CAMA =  6;              /* Shift count to A field  */
static const unsigned    CAMN = 10;             /* Shift count to N field  */
static const unsigned    CAM24=  0;              /* unused for wiener interface */


static const unsigned STATUSN =  0;
static const unsigned STATUSA =  0;
static const unsigned STATUSF =  0;

static const unsigned STATUS_Q = 0x08;
static const unsigned STATUS_X = 0x04;



/*!
   Create a 'branch' the branche memory maps are cached.  They are never destroyed
   for the life of the program.  
   \param branch    - Number of the branch to open.  There's a corespondence between
                      branch number and base address you can see in the code/docs.
   \param crate     - VME crate the controller is plugged into.
*/
CVC32CC32::CVC32CC32(unsigned branch, unsigned vmeCrate)
{
  if(!m_pBranches[branch][vmeCrate]) {
    mapBranch(branch, vmeCrate);
  }
  m_pBranch = m_pBranches[branch][vmeCrate];
}

/*!
   Return a pointer for a cnaf within this branch.
   \param crate - the crate number (zero for this, so actually ignored.
   \param slot  - The slot number addressed\
   \param subaddress - the subaddress addressed.
   \param function - operation being performed
   \return void*
   \retval address that when derefered with a 32 bit reference will do the operation.
           note that the 16's bit of the function determines the
	   transfer direction.
*/
void*
CVC32CC32::pointer(unsigned crate, unsigned slot, unsigned subaddreess, unsigned function)
{
  function &= 0xf;		// 16's bit of function is tossed.

  return m_pBranch + ((slot << CAMN) | (slot << CAMA) | (slot << CAMN));
}
/*!
   The control operation has a special case if the function is > 16:
*/
void
CVC32CC32::control(unsigned crate, unsigned slot, unsigned subaddress, unsigned  function)
{
  if (function < 16) {
    CMemoryMappedCAMACBranch::control(crate, slot, subaddress, function);
  }
  else {
    uint32_t* p = reinterpret_cast<uint32_t*>(pointer(crate, slot, subaddress, function));
    *p = 0;
  }
}

/*!
  Return true if the last operation set the controller's Q bit
*/
bool
CVC32CC32::qtest()
{
  uint16_t status = read16(0, STATUSN, STATUSA, STATUSF);
  return (status & STATUS_Q) != 0;
}
/*!
  Return true if the last operation set the controller's X bit:
*/
bool
CVC32CC32::xtest()
{
  uint16_t status = read16(0, STATUSN, STATUSA, STATUSF);
  return (status & STATUS_X) != 0;
}
/*
** Map a branch and put a pointer to its mapping in m_pBranches.
** there is an assumption that the branch will be aligned on page boundaries,
** and that it is an even number of pages long.
**
** Parameters:
**  branch     - Controller number...0-7
**  vmeCrate   - VME crate number 0 - 15
*/
void
CVC32CC32::mapBranch(unsigned branch, unsigned vmeCrate)
{
  void * fd = CVMEInterface::Open(CVMEInterface::Standard, vmeCrate);
  uint32_t base = 0x800000 + (branch << WIENER_SHIFT);
  
  try {
    m_pBranches[branch][vmeCrate] = 
      reinterpret_cast<uint8_t*>(CVMEInterface::Map(fd, base, WIENER_SIZE));
  } 
  catch (...) {
    CVMEInterface::Close(fd);
    throw;
  }
  CVMEInterface::Close(fd);
}
