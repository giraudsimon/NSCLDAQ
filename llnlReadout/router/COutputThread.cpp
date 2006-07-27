// in review.... check:
//   - use of m_buffersize is consistent.

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

#include <config.h>
#include "COutputThread.h"
#include "CRunState.h"
#include "DataBuffer.h"
#include <assert.h>
#include <buffer.h>
#include <buftypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


///////////////////////////////////////////////////////////////////////
///////////////////// Local constants. ////////////////////////////////
///////////////////////////////////////////////////////////////////////

// The VMUSB header:

static const int VMUSBLastBuffer(0x8000);
static const int VMUSBisScaler(0x4000);
static const int VMUSBContinuous(0x2000);
static const int VMUSBMultiBuffer(0x1000);
static const int VMUSBNEventMask(0x0fff);

static const int VMUSBContinuation(0x1000);
static const int VMUSBEventLengthMask(0xfff);
static const int VMUSBStackIdMask(0xe000);
static const int VMUSBStackIdShift(13);

///////////////////////////////////////////////////////////////////////
///////////////////// Local data types ////////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef struct _BeginRunBuffer {
  BHEADER         s_header;
  struct ctlbody  s_body;
} BeginRunBuffer, *pBeginRunBuffer;

typedef struct _ScalerBuffer {
  BHEADER         s_header;
  struct sclbody  s_body;
} ScalerBuffer, *pScalerBuffer;

typedef struct _EventBuffer {
  BHEADER      s_header;
  uint16_t     s_body[1];
} EventBuffer, *pEventBuffer;
////////////////////////////////////////////////////////////////////////
///////////////////// Construction and destruction /////////////////////
////////////////////////////////////////////////////////////////////////

/*!
   Create an outupt thread.  Should only create 1 however if the
   following were parameters could create multiple:
   - Event buffer queue.
   - Free buffer queue.
   - Run state.

   In future applications this could be done to manage multiple VM-USB
   controlled VME crates in 'singles mode'... or with a subsequent chunk of
   software on the end of spectrodaq assembling data.

*/
COutputThread::COutputThread() : 
  m_sequence(0),
  m_outputBufferSize(0)		// Don't know yet.
{
  
}
/*!
  Destruction is a no-op at this time.
*/
COutputThread::~COutputThread()
{
}

////////////////////////////////////////////////////////////////////////
////////////////////// Thread entry point... ///////////////////////////
////////////////////////////////////////////////////////////////////////

/*
   Thread entry point.  This is just an infinite buffer processing loop.
*/
int
COutputThread::operator()(int argc, char** argv)
{
  // Main loop is pretty simple.

  while(1) {

    DataBuffer& buffer(getBuffer());
    processBuffer(buffer);
    freeBuffer(buffer);

  }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////// Utility functions ///////////////////////////
/////////////////////////////////////////////////////////////////////////


/*
   Send a formatted buffer to spectrodaq.
   Parameters:
     void*        pBuffer            - The formatted buffer.
     unsigned int sdaqTag            - How to tag the buffer.
     size_t       sdaqWords          - How big a Spectrodaq buffer to create.
     size_t       copInSize          - words from pBuffer to copy to the spectrodaq
                                       buffer.

    Note that the spectrodaq buffer will be created, filled and routed
    in this function.
*/
void
COutputThread::bufferToSpectrodaq(void*          pBuffer,
				  unsigned int   sdaqTag,
				  size_t         sdaqWords,
				  size_t         copyInSize)
{
  DAQWordBuffer sdaqBuffer(sdaqWords);
  sdaqBuffer.SetTag(sdaqTag);
  sdaqBuffer.CopyIn(pBuffer, 0, copyInSize);
  sdaqBuffer.Route();

  // This will finalize the buffer.

}
/*
   Get a buffer from the event queue.  A reference to the buffer will
   be returned.

   Note that this will block if needed to wait for a buffer.
   note as well that between runs, we'll wind up blocking in here.

*/
DataBuffer&
COutputThread::getBuffer()
{
  DataBuffer* pBuffer = gFilledBuffers.get(); // Will block if needed.
  return *pBuffer;

}
/*
   Free a buffer that has been completely processed.
   This will return the buffer to the gFreeBuffers queue.
   Parameters:
      DataBuffer& buffer   - Reference to the buffer to return.

*/
void
COutputThread::freeBuffer(DataBuffer& buffer)
{
  gFreeBuffers.queue(&buffer);
}
/*
   Process a buffer from the reader.  At this time we are just going
   to figure out what type of buffer we have and dispatch accordingly.
   Buffers are as follows:
   Begin run is indicated by the type in the DataBuffer, all others look like
      data buffers.
   End run is indicated by the last bit set in the vmusb header.
   scaler is indicated by the scaler bit in the vm usb header.
   All others are event data.
   Parameters:
     DataBuffer&   buffer   - The buffer from the readout thread.
*/
void
COutputThread::processBuffer(DataBuffer& buffer)
{
  if (s_bufferType == 1) {
    startRun(buffer);
  } else {
    uint16_t header = buffer.s_rawData[0]; // VMUSB header.
    if (header & VMUSBLastBuffer) {
      endRun(buffer);
    }
    else if (header & VMUSBisScaler) {
      scaler(buffer);
    }
    else {
      events(buffer);
    }
  }
}
/*
   Process a begin run pseudo buffer. I call this a psuedo buffer because
   there will not be any data from the VM_usb for this
   We must:
   - Update our concept of the run state.
   - Set the m_outputBufferSize accordingly.
   - Create an NSCL begin run buffer and
   - send it to spectrodaq.
   - Destroy the NSCL begin run buffer we created.
   
   Parameters:
     DataBuffer&  buffer   - the buffer from the buffer queue.
*/
void
COutputThread::startRun(DataBuffer& buffer)
{
  // Update our concept of run state, and buffer size:

  CRunState* pState = getInstance();
  m_RunNumber       = pState->getRunNumber();
  m_title           = pState->getTitle();
  m_sequence        = 0;
  m_outputBufferSize= buffer.s_bufferSize;
  m_startTimestamp       = buffer.s_timeStamp;
  m_lastStampedBuffer    = 0; 

  // Allocate the Begin run buffer and fill it in.

  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(malloc(sizeof(BeginRunBuffer)));

  formatControlBuffer(BEGRUNBF, p);
  

  // Submit the buffer to spectrodaq as tag 2 and free it.

  bufferToSpectrodaq(p, 2, m_outputBufferSize, 
		     sizeof(BeginRunBuffer/sizeof(uint16_t)));
  free(p);
}
/*
   Called when an end of run has occured.  The end of run
   with a VMUSB is a data buffer.  We submit that data buffer.
   by calling events, and then we generate an end of run buffer.
   for the DAQ system to use to note the change of run state.
   Parameters:
      DataBuffer& buffer  - The data from the readout thread.
*/
void
COutputThread::endRun(DataBuffer& buffer)
{
  // Process the data buffer:

  events(buffer);

  // Now do an end run buffer.

  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(malloc(sizeof(BeginRunBuffer)));
  formatControlBuffer(ENDRUNBF, p);

  // Submit to spectrodaq and free:

  bufferToSpectrodaq(p, 2, m_outputBufferSize,
		     sizeof(BeginRunBuffer/sizeof(uint16_t)));
  free(p);
}

/*
    Format a scaler buffer.  Scaler buffers are contain a single event.
    The VMUSB manual is not completely clear, but I think the event
    consists of a normal event header followed, given the way I'm managing
    scalers a list of 32 bit words that contain the scaler values.
    For now we assume that there are no continuation cases...thsi is probably true as
    we'd need more than 1000 scalers to exceed the eventlength field.
*/
void
COutputThread::scaler(DataBuffer& buffer)
{

  // first put the scalers in place..


  uint16_t length = buffer.s_rawData[0] & VMUSBEventLengthMask;
  length   = length * sizeof(uint16_t)/sizeof(uint32_t); // Count of longs.
  uint32_t* pScalers = static_cast<uint32_t*>(&(buffer.s_rawData[1]));

  uint16_t  finalWordCount = sizeof(ScaerlBufferr)/sizeof(uint16_t) + 
                              length*sizeof(uint32_t)/sizeof(uint16_t) - 1;

  pScalerBuffer outbuf  = static_cast<pScalerBuffer>(malloc(m_outputBufferSize));
  memcpy(outbuf->s_body.scalers, pScalers, length*sizeof(uint32_t));

  // fill in the header.
  
  outbuf->s_header.nwds   = finalWordCount;
  outbuf->s_header.type   = 2;
  outbuf->s_header.cks    = 0;
  outbuf->s_header.run    = m_runNumber;
  outbuf->s_header.seq    = m_sequence;
  outbuf->s_header.nevt   = length; // This is the number of scalers.
  outbuf->s_header.nlam   = 0;
  outbuf->s_header.nbit   = 0;
  outbuf->s_header.buffmt = BUFFER_VERSION;
  outbuf->s_header.ssignature = 0x0102;
  outbuf->s_header.lsignature = 0x01020304;

  // Now the body that does not have scaler data:

  outbuf->s_body.btime    = m_lastTimestamp;
  m_lastTimestamp         = buffer.s_timeStamp - m_startTimestamp;
  outbuf->s_body.etime    = m_lastTimestamp;

  // Submit the buffer and free it:

  bufferToSpectrodaq(outbuf, 2, m_outputBufferSize, finalWordCount);
  free(outbuf);
		     

}


/*
   Format an event buffer. 
   An event buffer's size is given by teh buffer's s_bufferSize field.
   The body of the buffer is just copied to the body of the event buffer,
   the event count however has to be determined by walking through the
   events.  The VM-USB manual is unclear about whether or not the 
   VMUSB is including the terminator words in the event count.
   We'll assume it is for now and fix it if we detect later problems.
   We know we've hit the end of a buffer when the first word of an event
   is 0xffff since we don't use stack id 0xe (otherwise this is ambiguous
   with a max length continuation field.
*/
void 
COutputThread::events(DataBuffer& buffer)
{
  //  Create the output buffer first; and copy the
  //  event data into it:

  pEventBuffer p  = static_cast<pEventBuffer>(malloc(m_bufferSize));
  memcpy(pEventBuffer->s_body, buffer.s_rawData, buffer.s_bufferSize/sizeof(uint16_t));

  //  Now the he4ader except for nevt...

  uint16_t finalLength = sizeof(BHEADER) + buffer.s_bufferSize/sizeof(uint16_t);
  p->s_header.nwds   = finalLength;
  p->s_header.type   = DATABF;
  p->s_header.cks    = 0;
  p->s_header.seq    = m_sequence++;
  p->s_header.nlam   = 0;
  p->s_header.nbit   = 0;
  p->s_header.buffmt = BUFFER_REVISION;
  p->s_header.ssignature = 0x0102;
  p->s_header.lsignature = 0x01020304;


  // The event count

  p->s_header.nevt   = eventCount(p);

  // route the buffer and free it:

  bufferToSpectrodaq(p, 1, m_buffersize, finalLength);
  free(p);
  
}


/*
   Format a control buffer.  A control buffer is a begin or end run buffer.
   Parameters:
     uint16_t   type     - Type of the buffer (e.g. BEGRUNBF).
     void*      buffer   - Pointer to the buffer to forrmat.

*/
void
COutputThread::formatControlBuffer(uint16_t type, void* buffer)
{
  pBeginRunBuffer p = static_cast<pBeginRunBuffer>(buffer);

  // Header:
  
  p->s_header.nwds  = sizeof(BeginRunBuffer/sizeof(uint16_t));
  p->s_header.type  = type;
  p->s_header.cks   = 0;	// Not using the checksum!!!
  p->s_header.run   = m_runNuber;
  p->s_header.seq   = m_sequence;
  p->s_header.nevt  = 0;
  p->s_header.nlam  = 0;
  p->s_header.cpu   = 0;
  p->s_header.nbit  = 0;
  p->s_header.buffmt= BUFFER_REVISION;
  p->s_header.ssignature = 0x0102;
  p->s_header.lsignature = 0x01020304;


  // Body:
  
  memset(p->s_body.title, 0, 80);
  strncpy(p->s_body.title, m_title.c_str(), 79);

  time_t t;
  struct tm structuredtime;
  time(&t);

  p->s_body.sortim    = t - m_startTimestamp;


  localtime_r(&t, &structuredtime);
  p->s_body.tod.month = structuredtime.tm_mon;
  p->s_body.tod.day   = structuredtime.tm_mday;
  p->s_body.tod.year  = structuredtime.tm_year;
  p->s_body.tod.hours = structuredtime.tm_hour;
  p->s_body.tod.min   = structuredtime.tm_min;
  p->s_body.tod.sec   = structuredtime.tm_sec;
  p->s_body.tod.tenths= 0;	// Always 0 in unix.

}
/*
   Compute the event count for a buffer of VMUSB data.
   We step our way through the data counting cases where we have
   event headers without the continuation bit set until we get
   the 0xffff buffer terminator.
   Parameters:
   void* nsclBuffer   -  Pointer to an NSCL structured buffer whose body
                         has vmusb data.
*/

uint16_t
COutputThread::eventCount(void* nsclBuffer)
{
  pEventBuffer p = static_cast<pEventBuffer>(nsclBuffer);
  uint16_t*    pb= p->s_body;
  uint16_t events = 0;

  // Here we go:

  while (*pb != 0xffff) {
    uint16_t count = *pb & VMUSBEventLengthMask;
    if (*pb & VMUSBContinuation) event++;
    p += count;
  }

  return events;
}
