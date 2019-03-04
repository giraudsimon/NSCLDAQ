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
#include <CControlQueues.h>
#include <assert.h>
#include <list>

using namespace std;

//

CControlQueues* CControlQueues::m_pTheInstance(0);

/*!
   The constructor does othing since the individueal buffer queues
   are quite able to instantiate themselves.
   The constructor must be implemented in order to get it to be private.
*/
CControlQueues::CControlQueues()
{}

/*!
   Get the singleton instance of these queues (lazy instantiation).
   Note that while this is not inherently thread-safe, in fact the instance
   will be created early in system startup before additional threads have been
   created and scheduled, so it's safe enough.  To be truly threadsafe, the
   body of the if must be protected from threaded execution.
*/
CControlQueues* 
CControlQueues::getInstance()
{
  if(!m_pTheInstance) {
    m_pTheInstance = new CControlQueues;
  }
  return m_pTheInstance;
}

/*!
  Acquire the usb from the readout thread.  This sends an
  ACQUIRE request to the m_requestQueue and waits for an ACK to 
  be received from the m_replyQueue.
*/
void
CControlQueues::AcquireUsb()
{
  Enter();
  blockingTransaction(ACQUIRE);

  // Leave when USB is released.

}

/*!
    Release the USB.. this sends a RELEASE message to the request queue
    and blocks for an ACK response:
*/
void
CControlQueues::ReleaseUsb()
{
  blockingTransaction(RELEASE);
  Leave();
}
/*!
   Request an end run, block until the readout thread is just about to exit
   (caller or creator can then join).
*/
void
CControlQueues::EndRun()
{
  Enter();
  blockingTransaction(END);
  Leave();

}
/*!
   Request to pause a run.
*/
void
CControlQueues::PauseRun()
{
  Enter();
  blockingTransaction(PAUSE);
  Leave();
}

/*!
   Request to resume a paused run.
*/
void
CControlQueues::ResumeRun()
{
  Enter();
  blockingTransaction(RESUME);
  Leave();
}

/*!
   Send an acknowledge message to the reply queue.
*/
void
CControlQueues::Acknowledge()
{
  m_replyQueue.queue(ACK);
}
/*!
  Get a command request form the request queue... blocking if needed.
  the op code is returned to the caller.
*/
CControlQueues::opCode
CControlQueues::getRequest()
{
  opCode request = m_requestQueue.get();
  return request;

}
/*!
   Check for a request in the command queue.  This is non blocking.
   It is used between buffer acquisitions from the readout thread
   to see if it needs to back off for the control thread.
   \param command : CControlQueues::opCode
      Command received if one was received.
    \return bool
    \retval true   - A command message is available.
    \retval false  - no command message was available.
*/
bool
CControlQueues::testRequest(CControlQueues::opCode& command)
{
  list<opCode> messages = m_requestQueue.getAll();
  if(messages.empty()) {
    return false;
  }
  else {
    assert(messages.size() == 1); // can't stack multiply deep.
    command = messages.front();
    return true;
  }
}
/////////////////////////////////////////////////////////////////////////

//  Do the common work of a blocking transaction.

void
CControlQueues::blockingTransaction(CControlQueues::opCode command)
{

  m_requestQueue.queue(command);
  opCode reply = m_replyQueue.get();
  assert(reply == ACK);
}
