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
#include "CESCBD8210.h"
#include <CVMEInterface.h>

// Static data:

uint8_t* CESCBD8210::m_pBranches[CESCBD8210::BRANCHES][CESCBD8210::VMECRATES];


static const unsigned    CAMF =  2;		    /* Shift count to F field. */
static const unsigned    CAMA =  7;		    /* Shift count to A field  */
static const unsigned    CAMN =  11;		    /* Shift count to N field  */
static const unsigned    CAMC =  16;		    /* Shift count to C field  */

static const unsigned  CSRCRATE =   0;	    /* 'crate' for Branch CSR */
static const unsigned  CSRSLOT  =  29;	    /* 'slot' for branch CSR  */
static const unsigned  CSRSUBAD =   0;	    /* 'subaddress' for branch CSR */
static const unsigned  CSRFUN   =   0;

static const unsigned    IT4  =  1;                 /* Interrupt 4 pendng.           */
static const unsigned    IT2  =  2;                 /* Interrupt 2 pending           */
static const unsigned    MIT4 =  0x4;		    /* Mask interrupt 4.	     */
static const unsigned    MIT2 =  0x8;		    /* Mask interrupt 2.	     */
static const unsigned    MLAM =  0x10;		    /* Mask LAM interrupts.	     */
static const unsigned    MTO  =  0x20;		    /* MASK Branch timeout.	     */
static const unsigned    MNOX =	  0x800;	    /* Mask BERR on no x.	     */
static const unsigned    QMASK=  0x8000;	    /* Q bit in branch CSR.	    */
static const unsigned    XMASK=  0x4000;	    /* X bit in branch CSR.	     */

static const unsigned    BRANCH_SHIFT = 19;


/*!
   Construction requires us to get a shared memory region that corresponds to this
   branch.  Since there are several crates per branch, and since there may be several
   instances of the same branch, we maintain a cache of the branches that have
   already been mapped and only map branches which are not. 
   At this point we don't reference
   count, so we don't ever attempt to unmap a branch.

   \param branch - Branch number this object represents.  Note that this
                   is the value of the rotary buttons on the front panel of the
		   CES module.
   \param vmeCrate - Number of the VME crates this is in. This defaults to zero
                     which issuitable for single VME crate systems.
*/
CESCBD8210::CESCBD8210(unsigned branch, unsigned vmeCrate)
{
  if (!(m_pBranches[branch][vmeCrate])) {
    mapBranch(branch, vmeCrate);
  }
  m_pBranch = m_pBranches[branch][vmeCrate];

}

/*!
    Given the base pointer in m_pBranch and a cnaf, determine the
    address that can be dereferenced to do a 32 bit transfer for that cnaf.
    \param crate - the crate the pointer should reference.
    \param slot  - the slot the pointer should reference.
    \param subaddress - the subaddress the pointer should reference.
    \param function   - the function code the pointer should reference.

    \return void*
    \return Pointer that when dereferenced 32 bits wide will produce
            the desired cnaf on this branch.

    Note that if a 16bit transfer is needed, then the word following this
    pointer should be dereferenced. as the VME bus is big endian.

*/
void*
CESCBD8210::pointer(unsigned crate, unsigned slot, unsigned subaddress,
		    unsigned function)
{
  return m_pBranch + ((crate << CAMC) | (slot << CAMN) | (subaddress << CAMA) 
		      |(function << CAMF));
}

/*!
   Determine if the last operation on this branch returned an X response.
*/
bool
CESCBD8210::xtest()
{
  uint16_t csr = read16(CSRCRATE, CSRSLOT, CSRSUBAD, CSRFUN);
  return (csr & XMASK) != 0;
  
}
/*!
  Determine if the last operation on the branch returned a Q response
*/
bool
CESCBD8210::qtest()
{
  uint16_t csr = read16(CSRCRATE, CSRSLOT, CSRSUBAD, CSRFUN);
  return (csr & QMASK) != 0;
}
///////////////////////////////////////////////////////////////////////////////

/*
** Map a branch and put a pointer to its mapping in m_pBranches
** There is an assumption that a branch is aligned on page boundaries, and
** that it is an even number of pages long.  This is forced by the
** addressing scheme, and by the size of a branch for pretty much all reasonable
** architectures.
**
**  Parameters:
**      branch      - Number in the front panel branch selector on the physical module.
**      crate       - >VME< crate in which the branch driver module is installed.
*/
void
CESCBD8210::mapBranch(unsigned branch, unsigned crate)
{
  void* fd    = CVMEInterface::Open(CVMEInterface::Standard, crate);
  long base = 0x800000 + (branch  << BRANCH_SHIFT);
  try {
    m_pBranches[branch][crate] = 
      reinterpret_cast<uint8_t*>(CVMEInterface::Map(fd, base, 1 << BRANCH_SHIFT));
  }
  catch (...) {
    CVMEInterface::Close(fd);
    throw;
  }
  CVMEInterface::Close(fd);
}
