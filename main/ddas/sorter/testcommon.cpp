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

/** @file:  testcommon.cpp
 *  @brief: Implement common code for tests
 */
#include "testcommon.h"
#include <string.h>
/**
 * makeHit
 *    Given parameters for a hit, creates the data for a 4 longword hit.
 *
 * @param[out] hit - Pointer to a uint32_t[4]  which will receive the hit.
 * @param    crate   - Hit crate number.
 * @param    slot    - Hit slot number.
 * @param    chan    - Hit chanel number.
 * @param    rawTime - The hit time from the clock.
 * @param    energy  - Energy value.
 * @param    cfdTime - Defaults to 0, the CFD fractional time.
 */
void makeHit(
  uint32_t* hit,
  int crate, int slot, int chan, uint64_t rawTime, uint16_t energy,
  uint16_t cfdTime
)
{
  int eventSize = 4;
  int hdrSize   = 4;
  memset(hit, 0, sizeof(uint32_t)*4);
  hit[0] =
    (eventSize << 17) | (hdrSize << 12) | (crate << 8) | (slot << 4) | chan;
  hit[1] = rawTime & 0xffffffff;
  hit[2] = (rawTime >> 32) | (cfdTime << 16);
  hit[3] = energy;
  
}
//////////////