/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/
/******************************************************************************
  Interface for class CBufferTranslator
******************************************************************************/


#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <ByteBuffer.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <typeinfo>
using namespace std;


namespace DAQ 
{
  namespace BO // byte order package
  {

    template<class T> void swapBytes(T& podObj) {

      char* pItem = reinterpret_cast<char*>(&podObj);
      std::reverse(pItem, pItem+sizeof(podObj));

    }


    // Generic implementation for pod types that are not const or pointers.
    // It is not clear how you can convert pointers or non pod types... we do support arrays (see specialization below)
    //
    template<class T, class ByteIter>
    struct swapImpl {
      static ByteIter interpretAs(ByteIter pos, T& type) {
        static_assert(   std::is_pod<T>::value,     "Can only swap pod types");
        static_assert( ! std::is_pointer<T>::value, "Cannot swap pointers, this has no clear meaning");
        static_assert( ! std::is_const<T>::value,   "Target type must be non-const so we can set it");

        std::reverse_copy(pos, pos+sizeof(type), reinterpret_cast<char*>(&type));
        return pos+sizeof(type);
      }
    };

    /// Specialization for arrays (this is good for any type of array, char[N], int[N], etc...)
    template<typename T, std::size_t N, class ByteIter>
    struct swapImpl<T[N], ByteIter> {
      static ByteIter interpretAs(ByteIter pos, T (&type)[N]) {
        T* pArr = type;
        for (std::size_t i=0; i<N; i++) {
          pos = swapImpl<T,ByteIter>::interpretAs(pos, *(pArr+i));
        }
        return pos;
      }
    };

    template<class T, class ByteIter>
    struct noSwapImpl {
      static ByteIter interpretAs(ByteIter pos, T& type) {
        std::copy(pos, pos+sizeof(type), reinterpret_cast<char*>(&type));
        return pos+sizeof(type);
      }

    };

    // BufferTranslator that works on any generic buffer type
    class CByteSwapper
    {
    private:
      bool         m_needsSwap;

    public:
      CByteSwapper(bool needsSwap);

      bool isSwappingBytes() const { return m_needsSwap; }
      void setSwapBytes(bool swap) { m_needsSwap = swap; }


      // convert raw bytes to a properly byte ordered value
      // Partial template specialization is illegal for functions but not for classes, so
      // we delegate the real logic to helper classes. This allows for specialization for
      // arrays.
      template<typename T, typename ByteIter> T copyAs(ByteIter pos) const
      {
        T type;

        if (! m_needsSwap) {
          noSwapImpl<T,ByteIter>::interpretAs(pos, type);
        } else {
          swapImpl<T,ByteIter>::interpretAs(pos, type);
        }

        return type;
      }

      // convert raw bytes to a properly byte ordered value
      template<typename T, typename ByteIter> ByteIter interpretAs(ByteIter pos, T& type) const
      {
        if (! m_needsSwap) {
          return noSwapImpl<T,ByteIter>::interpretAs(pos, type);
        } else {
          return swapImpl<T,ByteIter>::interpretAs(pos, type);
        }
      }

    };

  } // end of BO
} // end of DAQ
#endif
