#ifndef CESCBD8210_H
#define CESCBD8210_H



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
   The CES CBD 8210 is a parallel branch highway driver.
   Each module can support up to 7 crates.  Up to 8 of them
   can be in a VME crate at a time.

   All operations are memory mapped, and therefore this is a subclass
   of the CMemoryMappedCAMACBranch class.
*/
class CESCBD8210 : public CMemoryMappedCAMACBranch
{
private:
  uint8_t*   m_pBranch;		// Base address of the branch memory map.

  static const unsigned BRANCHES = 8;
  static const unsigned VMECRATES= 16;
  static uint8_t* m_pBranches[BRANCHES][VMECRATES];


public:
  CESCBD8210(unsigned branch, unsigned vmeCrate = 0);

public:
  // Need to be able to supply the pointer for a cnaf within our branch:

  virtual void* pointer(unsigned crate, unsigned slot,
		       unsigned subaddress, unsigned function);

  // Only controllers know how to get the q/x response back:

  virtual bool qtest();
  virtual bool xtest();

  // Utilities:
private:
  void mapBranch(unsigned branch, unsigned crate);
  
};
#endif