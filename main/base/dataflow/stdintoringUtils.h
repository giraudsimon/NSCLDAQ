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

/** @file:  stdintoringUtils.h
 *  @brief: Header for utilities used by stdintoring.
 */
#ifndef STDINTORINGUTILS_H
#define STDINTORINGUTILS_H
#include <stdint.h>
#include <stddef.h>

class CRingBuffer;

struct header {
  uint32_t s_size;
  uint32_t s_type;
};

int integerize(const char* str);
uint32_t computeSize(struct header* pHeader);
void dumpWords(void* src, size_t nwords);
size_t putData(CRingBuffer& ring, void* pBuffer, size_t nBytes);

#endif