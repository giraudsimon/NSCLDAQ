#ifndef CMEMORYMAPPEDCAMACBRANCH_H
#define CMEMORYMAPPEDCAMACBRANCH_H

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


#include "CCAMACBranch.h"

/*!
   The important set of CAMAC interfaces we deal with are memory mapped..
   that is they occupy some space in VME memory, and an operation
   on a B,C,N,A,F (bcnaf) can be reduced to a memory read or write to the
   corresponding memory location.   Sample controllers are the
   CES CBD8210 (being phased out but still in use), and the
   Wiener CC32/VC32 board set.

   This base class implements all the dataway operations (not qtest nor
   xtest) in terms of a pure virtual function that provides the
   address represented by a bcnaf.

   \note that controllers can be either 16 or 32 bit wide.
*/
class CMemoryMappedCAMACBranch : public CCAMACBranch
{
private:  
  bool m_is32;			// True if data path to controller is 32 bits wide.

public:
  CMemoryMappedCAMACBranch(bool is32bitDataPath = true);

  // Here's the function that returns the address for a bcnaf:

  virtual void* pointer(unsigned crate, unsigned slot, 
			unsigned subaddress, unsigned function)  = 0;

  // Here are the functions we implement:

  virtual void control(unsigned  crate, unsigned slot, unsigned subaddress,
		       unsigned function);
  virtual void write(unsigned crate, unsigned slot, unsigned subaddress, 
		     unsigned function, int32_t data);
  virtual void write16(unsigned crate, unsigned slot, unsigned subaddress,
		       unsigned function, int16_t data);
  virtual int32_t read(unsigned crate, unsigned slot, unsigned subaddress,
		       unsigned function);
  virtual int16_t read16(unsigned crate, unsigned slot, unsigned subaddress,
			 unsigned function);
};


#endif 
