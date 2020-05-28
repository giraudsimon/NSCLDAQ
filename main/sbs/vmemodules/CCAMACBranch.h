#ifndef CCAMACBRANCH_H
#define CCAMACBRANCH_H
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

#include <stdint.h>


/*!
   This is an abstract base class for a VME module
   that manages at least one CAMAC crate.
   The concept is that this is the device dependent part
   of a CAMAC subsystem.  CAMAC modules delegate operations
   on their modules to some instance of a concrete class derived
   from this and therefore can be independent of the actual
   controller (e.g. CES or Wiener VC/CC32).

   \note This is a pure abstract base class.
*/
class CCAMACBranch
{
public:
  virtual void control(unsigned  crate, unsigned slot, unsigned subaddress,
		       unsigned function) = 0;
  virtual void write(unsigned crate, unsigned slot, unsigned subaddress, 
		     unsigned function, int32_t data) = 0;
  virtual void write16(unsigned crate, unsigned slot, unsigned subaddress,
		       unsigned function, int16_t data) = 0;
  virtual int32_t read(unsigned crate, unsigned slot, unsigned subaddress,
		       unsigned function) = 0;
  virtual int16_t read16(unsigned crate, unsigned slot, unsigned subaddress,
			 unsigned function) = 0;
  virtual bool qtest() = 0;
  virtual bool xtest() = 0;
};

#endif
