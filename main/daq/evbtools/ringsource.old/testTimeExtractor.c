/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file testTimeExtrctor.c
 * @brief Pull timestamp out of test events.
 *
 * This is a test timestamp extractor that goes with the ring test data
 * source.
 */

#include <stdint.h>
#include <DataFormat.h>


uint64_t
timestamp(pPhysicsEventItem item)
{
  uint64_t* pStamp = (uint64_t*)(item->s_body.u_noBodyHeader.s_body);
  return *pStamp;
}
