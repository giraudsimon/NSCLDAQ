/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  utils.cpp
 *  @brief: Implement utils.h 
 */
#include "utils.h"
extern "C" {
/**
 * swal
 *   @param l - longword to swap.
 *   @return uint32_t byte swapped longword.
 */
uint32_t swal(uint32_t l)
{
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (l & 0xff);
        l = l >> 8;
    }
    return result;
}

/** 
 * swap a quadword (64 bit) data item.  This follows the
 * algorithm in the final implementation of BSWAP_64 from 
 * http:/blogs.oracle.com/DanXX/entry/optimizing_byte_swapping_for_fun
 * By Dan Anderson (Oct 31, 2008)
 *
 * @param aquad - the quadword to swap.
 * 
 * @return uint64_t
 * @retval the swapped quadword.
 */
uint64_t 
swaq(uint64_t aquad)
{
  return (((uint64_t)(aquad) << 56) | 
	  (((uint64_t)(aquad) << 40) & 0xff000000000000ULL) | 
	  (((uint64_t)(aquad) << 24) & 0xff0000000000ULL) | 
	  (((uint64_t)(aquad) << 8)  & 0xff00000000ULL) | 
	  (((uint64_t)(aquad) >> 8)  & 0xff000000ULL) | 
	  (((uint64_t)(aquad) >> 24) & 0xff0000ULL) | 
	  (((uint64_t)(aquad) >> 40) & 0xff00ULL) | 
	  ((uint64_t)(aquad)  >> 56));

}


/**
 ** Reverses the bytes in a word.
 * @param aword - the word to reverse.
 * @return uint16_t 
 * @retval input word with opposite byte ordering.
 */
uint16_t
swaw(uint16_t aword)
{
  return (aword >> 8) |
         (aword << 8);
}
}