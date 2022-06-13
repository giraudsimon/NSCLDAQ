#ifndef CVC32CC32_H
#define CVC32CC32_H
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

#include "CMemoryMappedCAMACBranch.h"

/*!
   The Wiener VC32/CC32 is a board set that allows
   a single CAMAC crate to be attached to a single
   VME module.  It's advantages are that it runs quite a bit faster than
   the CES CBD 8210, the cable is much less delicate, each crate uses
   a smaller address space, it has a 32 bit data path, and finally,
   it's being made, while the CES modules are out of production for quite
   some time.

   The controller is a memory mapped controller, so this is a concrete subclass
   of CMemoryMappedCAMACBranch
*/
class CVC32CC32 : public CMemoryMappedCAMACBranch
{
private:
  uint8_t*   m_pBranch;		// Base address of the branch memory map.

  static const unsigned BRANCHES = 8;
  static const unsigned VMECRATES= 16;
  static uint8_t* m_pBranches[BRANCHES][VMECRATES];

public:
  CVC32CC32(unsigned branch, unsigned vmeCrate = 0);

  // Need to be able to supply the pointer for a cnaf within our branch:

  virtual void* pointer(unsigned crate, unsigned slot,
		       unsigned subaddress, unsigned function);

  // Control operations are a bit horky; some are reads, some writes.

  virtual void control(unsigned crate, unsigned slot, unsigned subaddress,
		       unsigned function);

  // Only controllers know how to get the q/x response back:

  virtual bool qtest();
  virtual bool xtest();


  // Utilities:
private:
  void mapBranch(unsigned branch, unsigned crate);
};


#endif
