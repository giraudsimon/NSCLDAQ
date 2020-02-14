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

/** @file:  testcommon.h
 *  @brief: Provide common code for tests.
 */
#ifndef TESTCOMMON_H
#define TESTCOMMON_H
#include <stdint.h>

void makeHit(
  uint32_t* hit,
  int crate, int slot, int chan, uint64_t rawTime, uint16_t energy,
  uint16_t cfdTime = 0
);
int randRange(int n);


#endif