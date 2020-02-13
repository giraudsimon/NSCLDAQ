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

/** @file:  stdintoringUtils.cpp
 *  @brief: implementation  for utilities used by stdintoring.
 */
#include "stdintoringUtils.h"
#include "CRingBuffer.h"
#include "stdintoringsw.h"
#include <iostream>
#include <string.h>

using namespace std;


/**************************************************************
 * integerize:                                                *
 *   Utility function that converts a string to an integer    *
 *   while allowing the suffixes k, m, g to be kilo, mega     *
 *   and giga multipliers respectively.  Computer sizes e.g.  *
 *   k = 1024, m = 1024*1024, g = 1024*1024*1024              *
 *                                                            *
 * On failure, this will print the usage and exit with a fail *
 * code.                                                      *
 *                                                            *
 * Parameters:                                                *
 *   const char* str  - The input string                      *
 * Returns:                                                   *
 *   The integer result                                       *
 *************************************************************/

int 
integerize(const char* str)
{
  char* endptr;

  // Decode the front part:

  int base = strtol(str, &endptr, 0); // Allow hex etc.
  
  // It's an error to not have at least one digit:

  if (endptr == str) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // If the entire string was converted return it:

  if(strlen(endptr) == 0) {
    return base;
  }
  // If there's more than one extra char, that's an error too:

  if (strlen(endptr) > 1) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // One extra character can be the multiplier

  char multiplier = *endptr;
  int value;

  switch (multiplier) {
  case 'k':
    value = base*1024;
    break;
  case 'm':
    value = base*1024*1024;
    break;
  case 'g':
    value = base*1024*1024*1024;
    break;

  default:
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }
  return value;
}

/**
 * Determine the size of an item in the possible presence of a difference
 * in endianness.  This is done by knowing only the bottom 16 bits of type data
 * are used.
 * @param pHeader - pointer to the header.
 * @return uint32_t
 * @retval the size of the item converted to local format if needed.
 */
uint32_t
computeSize(struct header* pHeader)
{
  if ((pHeader->s_type & 0xffff) == 0) {
    uint8_t swappedSize[4];
    memcpy(swappedSize, &pHeader->s_size, 4);
    uint32_t result = 0;
    for (int i =0; i < 4; i++) {
      uint32_t byte = swappedSize[i];
      byte          = byte << (24 - 4*i);
      result       |= byte;
    }
    return result;
  }
  else {
    return pHeader->s_size;
  }
}

void dumpWords(void* src, size_t nwords)
{
  uint16_t* s = reinterpret_cast<uint16_t*>(src);
  std::cerr << std::hex;
  for (int i=0; i < nwords; i++) {
    if ((i % 8) == 0 ) std::cerr << std::endl;
    std::cerr << *s++ << " ";
  }
  std::cerr << std::dec;
}

/**
 * Put data into the ring.
 * Each data item is assumed to be preceded by a two longword header of the form:
 * struct header {
 *    uint32_t size;
 *    uint32_t type
 *  };
 * 
 * Futhermore, from the point of view of byte ordering, the type field only has
 * nonzero bits in the lower order 16 bits.
 *
 * We're going to put each data item in the ring atomically.
 * If there are left over data in the buffer, that will be shifted down
 * to the beginning of the buffer and
 * the remaining size will be returned:
 * 
 * @param ring    - reference to the target ring buffer.
 * @param pBuffer - Pointer to the data buffer.
 * @param nBytes  - Number of bytes in the data buffer.
 *
 * It is assumed that the total buffer size will be larger than
 * needed to hold the largest item.   That's controld by the 
 * --mindata switch in any event.
 */
size_t 
putData(CRingBuffer& ring, void* pBuffer, size_t nBytes)
{
    // Figure out how many bytes of complete ring items we have and
    // write them all out in one put:
    
    size_t putSize(0);
    size_t bytesLeft(nBytes);
    uint8_t* p;
    while (bytesLeft > sizeof(struct header)) {
        uint32_t itemSize = computeSize(reinterpret_cast<header*>(p));
        if (itemSize < bytesLeft) {
            putSize   += itemSize;
            bytesLeft -= itemSize;
            p         += itemSize;
        } else {
            break;              // Inomplete item.
        }
    }
    if (putSize > 0) {
        // There's data to put, a small read may well not give us this.
        
        ring.put(pBuffer, putSize);
        
        // Have to slide any residual data down.
        
        if (bytesLeft) {
            memmove(pBuffer, p, bytesLeft);
        }
    }
    
    
    return bytesLeft;                    // Amount of residual data.
}

