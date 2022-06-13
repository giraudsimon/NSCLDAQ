#ifndef RINGBUFINT_H
#define RINGBUFINT_H
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

/*
  This file contains defintions of structures and constants that
  are internal to the ring buffer subsystem.  This header is not
  intended to be included by client software and therefore should not
  be installed if possible.
*/
#include <unistd.h>


/* constants - These are defined in this way so that they
               can be overidden by compiler -D switches. 
*/

#ifndef DEFAULT_DATASIZE  
#define DEFAULT_DATASIZE  1024*1024*8
#endif

#ifndef DEFAULT_MAX_CONSUMERS
#define DEFAULT_MAX_CONSUMERS 100
#endif

#ifndef DEAULT_POLLMS
#define DEFAULT_POLLMS 3
#endif

/*
   The front of each ring buffer consists of a header.
   The header helps the software locate important segments of the
   buffer as well as to know how big the buffer itself is:
*/

#define MAGICSTRING "NSCLRing"

typedef struct __RingHeader {
   char       s_magicString[32];	/* Should contain the text "NSCLRing" */
  volatile size_t     s_maxConsumer;	/* Maximum # of consumers. allowed by the ring.  */
  volatile size_t     s_dataBytes;      	/* Number of bytes of data in the data segment.  */
  volatile off_t      s_producerInfo;    /* Offset to the producer descriptor.            */
  volatile off_t      s_firstConsumer;	/* Offset to the first consumer descriptor       */
  volatile off_t      s_dataOffset;	/* Offset to the data segment                    */
  volatile off_t      s_topOffset;	/* Offset to the top of the storage.             */
} RingHeader, *pRingHeader;

/*
  Each client is described by the following data structure.  Both producers and
  consumers have a descriptor like this:

*/
typedef struct __ClientInformation {
  volatile off_t      s_offset;          /* Put/get offset into the buffer for consumer. */
  volatile pid_t      s_pid;		/* Process Id of the client.                    */
} ClientInformation, *pClientInformation;

/*
   This is the Ring buffer structure itself.  The only thing we won't be able
   to show is the data region because where that starts depends on the size of
   the consumer information array, and that's indefinite.
*/
typedef struct __RingBuffer {
  RingHeader         s_header;	     /* the ring buffer header.                     */
  ClientInformation  s_producer;     /* Client information for the single producer. */
  ClientInformation  s_consumers[1]; /* Client information for the consumers.       */
} RingBuffer, *pRingBuffer;



#endif
