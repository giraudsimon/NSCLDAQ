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
#include <CSynchronizedThread.h>
#include <stdexcept>
/**
 * Creation  just chains to the base class construtor.
 */
CSynchronizedThread::CSynchronizedThread()
  : Thread()
{}

/**
 * Destructor only exists to chain destructors:
 */
CSynchronizedThread::~CSynchronizedThread()
{
}

/**
 * Start the thread by locking the mutex, invoking the base class start and waiting on the condition
 * variable
 */
void
CSynchronizedThread::start()
{
  m_mutex.lock();

  Thread::start();

  m_conditionVar.wait(m_mutex);
  m_mutex.unlock();
}
/**
 * Gets control when the thread starts.
 * - Invoke init
 * - Signal the condition.
 * - release the mutex,
 * - invoke operator()
 */
void
CSynchronizedThread::run()
{
  try {
    m_mutex.lock();

    init();			// Do thread unsafe initialization.

    m_conditionVar.signal();
    m_mutex.unlock();
    operator()();
  } catch (...) {
    // if anything in the try block failed, allow the parent thread to 
    // continue. Maybe it will correct something and then attempt to respawn the
    // thread.
    m_conditionVar.signal();
    m_mutex.unlock();

    // throw to signal that this thread failed.
    std::rethrow_exception(std::current_exception());
  }
  
}

/**
 *  No op in case someone wants to use this thread as a normal thread:
 */
void
CSynchronizedThread::init() {}
