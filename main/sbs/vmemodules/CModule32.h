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

#ifndef CMODULE32_H
#define CMODULE32_H

#include "VmeModule.h"
#include <stdint.h>


/*!
   Base class for modules that have 32 bit wide register sets.
   This module is especially useful as a base class for several
   Struck (SIS) modules as their register sets have some
   common characteristics:
   - Full 32 bit.
   - Existence of 'key' registers.

*/

class CModule32
{
  // Data:
private:
  mutable CVmeModule  m_Module;

  // Constructors and other canonicals
public:
  CModule32(uint32_t base, size_t size,int crate = 0);
  ~CModule32();

  // Various copy operations are not now supported...although
  // in retrospect the CVmeModule does support this.
private:
  CModule32(const CModule32&);
  CModule32& operator=(const CModule32&);
  int       operator==(const CModule32&);
  int       operator!=(const CModule32&);
public:

  // Selectors

  CVmeModule& getModule() {
    return m_Module;
  }


  // Class functions - not intended for public use, but
  // for the derived actual device classes.

protected:
  uint32_t peek(uint32_t byteoff) const {
    return m_Module.peekl(Offset(byteoff));
  }
  void poke(uint32_t data, 
		uint32_t byteoff) const {
    m_Module.pokel(data, Offset(byteoff));
  }
  void Key(uint32_t offset) const; //!< write a key register.

  static uint32_t Offset(uint32_t off)  {
    return off/sizeof(uint32_t);
  }
  static void ThrowString(const char*  pLeader,
			  const char*  pMessage) ;

};
#endif
