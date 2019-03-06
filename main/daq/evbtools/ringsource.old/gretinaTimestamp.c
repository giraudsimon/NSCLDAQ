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
 * This is a timestamp extractor for Gretina events.
 * this extractor allows Gretina or even Gretina + s800 to work in conjunction with
 * arbitrary devices in the event builder.
 *
 *  The format of a gretina event body is shown below:
 * 
 * \verbatim
 *  +----------------------------------------------------+
 *  |  Words in event (32 bits)                          |
 *  +----------------------------------------------------+
 *  | fragment1 type (32 bits)                           |
 *  +----------------------------------------------------+
 *  | fragment1 payload size (32 bits)                   |
 *  +----------------------------------------------------+
 *  | Timestamp (64 bits)                                |
 *  |                                                    |
 *  +----------------------------------------------------+
 *  | Fragment 1 body                                    |
 *  ...                                               ....
 *  +----------------------------------------------------+
 * \endverbatim
 *
 *  The code in this extractor takes the timestamp from the first
 *  fragment and returns it.  More advanced code could take an average
 *  timestamp I suppose.
 */
#include <assert.h>
#include <DataFormat.h>
#include <stdint.h>

/**
 *  This struct is the gretina event fragment header.
 */

struct fragment {
  uint32_t  type;
  uint32_t  size;
  uint64_t  timestamp;
  char*  pData;
};

/**
 * Returns the timestamp from the Gretina event passed in.
 * Sanity checks are that the event size is at least a header.
 *
 * @param item  - Pointer to a physics event item containing
 *                the data.
 * @return uint64_t
 * @retval Timestamp of the first gretina fragment.
 */
uint64_t
timestamp(pPhysicsEventItem item)
{
  struct fragment* pFragment;
  uint32_t*        pBody;

  // Get the body, ensure the size is ok and then
  // create a pointer to the first fragment:

  pBody = (uint32_t*)(item->s_body.u_noBodyHeader.s_body);
  uint32_t bodySize = *pBody++;

  assert((bodySize - sizeof(uint32_t)) >= sizeof(struct fragment));

  // pBody now points to the first fragment:

  pFragment = (struct fragment*)(pBody);
  return pFragment->timestamp;

}
