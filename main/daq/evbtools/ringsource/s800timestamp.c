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
 * This is a timestamp extractof for s800 data.
 *  The structure of an s800 event body from the ring buffers is:
 * \verbatim
 * +----------------------------------+
 * | Words in event (32 bits)         |
 * +----------------------------------+
 * | S800 packet size                 |
 * +----------------------------------+
 * | S800_PACKET (16 bits)            |
 * +----------------------------------+
 * | first packet size (16 bits)      |
 * +----------------------------------+
 * | S800_xxxx (16 bits)              |
 * +----------------------------------+
 * | sub packet payload               |
 *     ....
 * +----------------------------------+
 *
 * \endverbatim
 */


#include "s800.h"
#include <DataFormat.h>
#include <assert.h>
#include <stdint.h>

/**
 * Returns the timestamp from the s800 event passed in. Several sanity checks are performed:
 * - the event must have the correct type.
 * - the event format level must be at least S800_MINVERSION
 * - there must be a packet of type S800_TIMESTAMP_PACKET 
 *
 * If any of these assertions fails, we kill the program.
 * @param pEvent - void* pointer to the event
 *
 * @return long long
 * @retval timestamp extracted from the data.
 */
uint64_t
timestamp(pPhysicsEventItem item)
{
  pS800event pFullEvent;
  pS800timestamp pTimestamp;
  ppacket    pNextPacket;
  uint16_t*  pWords;
  int64_t   nRemaining;
  int        nPktSize;

  /* Check that we have an s800 event and it matches the minimum version: */

  pFullEvent = (pS800event)(item->s_body.u_noBodyHeader.s_body); 
  assert(pFullEvent->s_type == S800_PACKET);
  uint16_t versionWord = pFullEvent->s_version;
  versionWord = (versionWord & S800_OUTERVSNMASK) >> S800_OUTERVSNSHIFT;
  assert(versionWord == S800_OUTERVERSION);

  /* Now we can continue processing; Get a pointer to the first packet and
     the number of words that remain after it. */

  nRemaining = pFullEvent->s_size * sizeof(uint16_t);
  pNextPacket= &(pFullEvent->s_firstPacket);
  nRemaining -= sizeof(S800event) - sizeof(packet);

  /* Look at the sub-packets hunting for a timestamp.  If we run out of words assert. */


  while(nRemaining > 0) {
    if (pNextPacket->s_type == S800_TIMESTAMP_PACKET) {
      pTimestamp = (pS800timestamp)pNextPacket;
      return (pTimestamp->s_timestamp); 
    }
    /* skip to the next packet */
    
    nPktSize = pNextPacket->s_size;
    pWords   = (uint16_t*)pNextPacket;
    pNextPacket = (ppacket)(pWords + nPktSize);
    nRemaining -= nPktSize*sizeof(uint16_t);
  }
  /* IF we ran out of words assert that too */

  assert(0);

  
}

