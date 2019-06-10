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

#ifndef __CCONTROLQUEUES_H
#define __CCONTROLQUEUES_H
#ifndef __CGAURDEDOBJECT_H
#include <CGaurdedObject.h>
#endif

#ifndef __CBUFFERQUEUE_H
#include <CBufferQueue.h>
#endif




/*! 

     This file contains definitions of queues used to communicate control
     requests and responses from the main thread to the readout thread and
     back.   The idea  is that if the main thread needs to do something that
     requires the USB, it will do a control request to the readout thread.
     The readout thread will then pause data taking and reply to the 
     main thread, which will then do its USB voodoo, and send another control
     request to the readout thread which will restart data taking and 
     respond with an acknowledge.

     The original design required the main thread to communicate control requests
     that would be executed in the readout thread, but this is more complex
     than required.  The communication protocol is quite simple:
     - main thread sends an ACQUIRE string to the readout thread.
     - readout thread sends an ACK back indicating that it has halted data taking
       and is now waiting.
     - main thread performs the USB stuff it needs to do.
     - when done with the USB, main thread sends a RELEASE string to the readout
       thread which has been waiting for this.
     - readout thread sends an ACK back indicating that it is now resuming
       data taking.
     - When the control thread wants to end a run it sends an ENDRUN message
       to the readout thread which ends the run and, as its last act before exiting,
       sends an ACK back.

       This is all done via a request and a reply queue which are both
       BufferQueues of std::string.   
       These are bundled together in  a CControlQueues class with appropriate
       members to manage the communication protocol.

       CControlQueues is a singleton object.

*/

class CControlQueues : public CGaurdedObject
{
  // Public data types (operation codes):
public:
  typedef enum _opCode {
    ACQUIRE,			// Acquire VM-USB for other thread.
    RELEASE,			// Release VM-USB back to readout thread
    END,			// End the run.
    PAUSE,			// Pause the run.
    RESUME,			// Resume the run.
    ACK,			// Acknowledge operation (reply).
    NAK				// Negative acknowledge (reply not yet used).

  } opCode;
  // data:
private:
  CBufferQueue<opCode>     m_requestQueue;
  CBufferQueue<opCode>     m_replyQueue;

  static CControlQueues*        m_pTheInstance;

private:
  CControlQueues();
public:
  static CControlQueues* getInstance();

  // operations on the queue.
  //     - Operations used by the control thread;
  //       These block until the appropriate reply occurs.
  //
public:
  void AcquireUsb();		//!< Send ACQUIRE to m_requestQueue wait for ACK.
  void ReleaseUsb();		//!< Send RELEASE to m_requestQueue wait for ACK.
  void EndRun();		//!< Send END     to m_requestQueue wait for ACK.
  void PauseRun();
  void ResumeRun();
  // 
  //    - Operations used by the readout thread;
  //
  void Acknowledge();		         //!< Send ACK msg to replyQueue. 
  opCode getRequest();	         //!< Blocks until request.
  bool testRequest(opCode& code); //!< Returns without blocking.

  // utilities:

private:
  void blockingTransaction(opCode);
};



#endif
