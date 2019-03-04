/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef BYTE_CAST_H
#define BYTE_CAST_H

#include <cstring>
#include <type_traits>

// the safe way to convert byte data to a type.
template <class Dest>
inline Dest byte_cast(const char* source) {

  /*
     static_assert(std::is_trivially_copyable<Dest>::value, 
     "Destination type must be trivially copyable");
     */
  Dest dest;
  std::memcpy(&dest, source, sizeof(dest));
  return dest;
}

#endif
