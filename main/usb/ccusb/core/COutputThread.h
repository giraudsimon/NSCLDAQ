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

#ifndef COUTPUTTHREAD_H
#define COUTPUTTHREAD_H

using namespace std;

#include <CSynchronizedThread.h>

#include <stdint.h>
#include <string>

// Forward definitions:

struct DataBuffer;
typedef struct _StringsBuffer StringsBuffer;
class CRingBuffer;
class CSystemControl;

/*!
    This class bridges the gap between the buffer format of the
    VM-USB and the NSCL Buffer format.  We will not bother to
    reformat the data within the buffer, just change the
    header word into an NSCL style buffer.  The data
    we are using will come from a buffer queue.  Elements
    of the buffer queue will have the following format:
\verbatim
    +--------------------------------+
    |  Buffer size                   | 
    +--------------------------------+
    |  Buffer Type                   |
    +--------------------------------+
    | Time stamp                     |
    +--------------------------------+
    |   Buffer contents as described |
    | in section 4.6 of the VM USB   |
    | manual                         |
    +- - - - - - -- - - - - - - - - -+

\endverbatim
    
    -#  Buffer size is gotten from the read from the VM_USB
        and indicates the number of bytes of data
        in the body and its header word (does not include either itself nor
	the buffer type.
    -#  The buffer type is one of the following:
        - 1   Run starting... in this case there will be no VM-USB body.
              and the buffersize will be size of Spectrodaq buffer desired.
        - 2   VM-USB data  note that run end is determined by seeing
              the LB bit in the VM-USB header.
    -# We will generate only the following sorts of NSCL data buffers:
        - 11  Begin run.
        - 12  End Run
        - 1   Physics data
        - 2   Scaler Data.
    -# Time stamp is result of time(2) when the buffer was received from
       the VMUSB.

 \note  This class is a separate thread of execution.
 \note  A global variable: gFilledBuffers is a CBufferQueue that contains
        the data shown above and is used to receive raw data buffers from
	the readout thread.
  \note There is no need to start/stop thread each run.   Once a run is over,
        this thread will simply block on the buffer queue until the next run
        emits the begin run buffer.
*/

class COutputThread  : public CSynchronizedThread
{
  // Class private types
  
  typedef uint64_t (*TimestampExtractor)(void*);
  typedef void      (*StateChangeCallback)();
  
  // Thread local data:
private:
  // These are fetched from the CRun state at start of run.

  uint32_t    m_runNumber;	// Run number;
  std::string m_title;          // Run title

  // other data:
private:
  unsigned int m_elapsedSeconds;   // Seconds into the run.
  uint32_t    m_sequence;	   // Buffer sequence number.
  uint32_t    m_outputBufferSize;  // Bytes in output buffers.
  timespec      m_startTimestamp;    //!< Run start time.
  timespec      m_lastStampedBuffer; //!< Seconds into run of last stamped buffer.
  std::string m_ringName;
  CRingBuffer* m_pRing;
  uint64_t    m_nEventsSeen;
  unsigned    m_nBuffersBeforeCount;
  
  // Event assembly buffer:

  size_t       m_nOutputBufferSize;
  uint8_t*     m_pBuffer;
  uint8_t*     m_pCursor;
  size_t       m_nWordsInBuffer;
  TimestampExtractor m_pEvtTimestampExtractor;
  TimestampExtractor m_pSclrTimestampExtractor;
  StateChangeCallback m_pBeginRunCallback;
  CSystemControl&   m_systemControl;
  
  // Constuctors and other canonicals.

public:
  COutputThread(const char* pRing, CSystemControl& sysControl);
  virtual ~COutputThread();
private:
  COutputThread(const COutputThread& rhs);
  COutputThread& operator=(const COutputThread& rhs);
  int operator==(const COutputThread& rhs) const;
  int operator!=(const COutputThread& rhs) const;
public:

  // Thread operations are all non-public in fact.. don't want to call them
  // from outside this class.. only from within the thread.. This includes the
  // thread entry point.

public:
  virtual void init();		// Entry point.

protected:

  virtual void operator()();	// Old style entry point.
private:
  DataBuffer& getBuffer();
  void freeBuffer(DataBuffer&  buffer);

  void processBuffer(DataBuffer& buffer);
  void formatBuffer(DataBuffer& buffer);
  void startRun(DataBuffer& buffer);
  void endRun(DataBuffer&   buffer);
  void pauseRun(DataBuffer& buffer);   // Bug #5882.
  void resumeRun(DataBuffer& buffer);  // Bug #5882.
  void scaler(void* pData);
  void events(DataBuffer&   buffer);
  void processStrings(DataBuffer& buffer, StringsBuffer& strings);
  void getTimestampExtractor();
  void scheduleApplicationExit(int status);

 
  void event(void* pData);

  void attachRing();
  uint8_t* newOutputBuffer();
  void outputTriggerCount(uint32_t runOffset);

};

#endif
