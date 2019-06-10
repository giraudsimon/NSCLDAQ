/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



// This file contains the two buffer queues for event data and
// unbound functions that create and destroy event data buffers.
//

#include <config.h>

#include "DataBuffer.h"
#include <stdlib.h>
#include <unistd.h>


// Buffer queues that communicate between the readout and routing threads:

CBufferQueue<DataBuffer*> gFilledBuffers; 
CBufferQueue<DataBuffer*> gFreeBuffers;

/*!
   Create a new data buffer.  
   \param bodySize  - Number of 16 bit words in the body.

   \return DataBuffer*
   \retval A pointer to the newly created data buffer. It is up to the caller
           to decide what to do with it (e.g. we don't insert it into the
	   gFreeBuffers queue
*/

DataBuffer*
createDataBuffer(uint32_t bodySize)
{
  size_t bytes = sizeof(struct DataBuffer) +  (bodySize-1)*sizeof(uint16_t);
  DataBuffer* p =  static_cast<DataBuffer*>(malloc(bytes));
  p->s_storageSize = bodySize;
  return p;
}
/*!
   Free storage that was allocated by a data buffer.   It would be a very bad
   thing if this data buffer were in one of the buffer queues when destroyed!
*/
void
destroyDataBuffer(DataBuffer* pBuffer) 
{
  free(pBuffer);
}
