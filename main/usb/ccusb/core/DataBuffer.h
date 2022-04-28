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
#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif 
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

#ifndef __CBUFFERQUEUE_h
#include <CBufferQueue.h>
#endif

/*!
    \file DataBuffer
        Defines the structure of data buffers that will be
        passed around from readout thread to output thread.
        These are managed in two buffer queues.:
	gFilledBuffers is the queue of buffers with data and
        gFreeBuffers is a queue of empty buffers available for
	use by the readout thread.
*/

/**
 *  This is the structure of a string buffer's rawData storage:
 */
typedef struct _StringsBuffer {
  uint32_t s_stringCount;      	// Number of strings in the buffer.
  uint32_t s_ringType;		// Type of text item.
  char     s_strings[];		// The strings themselves, null terminated.
} StringsBuffer, *pStringsBuffer;

// Structure of a buffer queue element.

struct DataBuffer {
  uint32_t   s_bufferSize;	//!< Bytes used in the buffer.
  uint32_t   s_storageSize;     //!< bytes in s_rawData[].
  uint32_t   s_bufferType;	//!< Type of buffer.
  time_t     s_timeStamp;	//!< When the buffer was received.
  uint16_t   s_rawData[1];	//!< Really larger than that

};


extern CBufferQueue<DataBuffer*>  gFilledBuffers;
extern CBufferQueue<DataBuffer*>  gFreeBuffers;

//  A couple of useful unbound functions:

extern DataBuffer* createDataBuffer(uint32_t bodySize); //!< Create a new data buffer.
extern void        destroyDataBuffer(DataBuffer* pBuffer); //!< Free data buffer.

// The CCUSB header:

static const int CCUSBLastBuffer(0x8000);
static const int CCUSBisScaler(0x4000);
static const int CCUSBContinuous(0x2000);
static const int CCUSBMultiBuffer(0x1000);
static const int CCUSBNEventMask(0x0fff);

static const int CCUSBContinuation(0x1000);
static const int CCUSBEventLengthMask(0xfff);
static const int CCUSBStackIdMask(0x2000);
static const int CCUSBStackIdShift(13);

// Buffer types;

static const int TYPE_START(1);
static const int TYPE_STOP(2);
static const int TYPE_EVENTS(3);
static const int TYPE_STRINGS(4);
static const int TYPE_PAUSE(5);           // Pause/resume for Bug #5882
static const int TYPE_RESUME(6);
