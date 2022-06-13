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
#define COMPILINGCBUFFERQUEUE
#ifndef __CBUFFERQUEUE_H
#include "CBufferQueue.h"
#endif

#include <ErrnoException.h>
#include <sys/time.h>

/*!  Construct the base class. we just need to set the initial value of
  the high water mark for waiters.  Note that there are two ways that
  a client may block:
  - When attempting to do a get, if there are no buffers, the caller will
    automatically block until either the buffer load in the queue is
    above m_nWakeLevel or wake is called.
  - The client decides to wait for buffers via wait().  In this case,
    the client blocks until at least m_nWakeLevel buffers are present and
    then is given the entire queue contents.

  \param wakeLevel : size_t [default = 0]
     Sets the high water mark for waking.  The default ensures that wakes happen
     whenever a buffer is inserted in the queue.
*/
template<class T>
CBufferQueue<T>::CBufferQueue(size_t wakeLevel) :
  m_nWakeLevel(wakeLevel)
{
  
  if(pthread_cond_init(&m_condition, NULL)) {
    throw CErrnoException("CBufferQueue construction initializing condition variable");
  }

}
/*!
   Destructor - it is the destroyer's responsibility to synchronize the destruction
   of this object so that nobody is getting the rug yanked out from underneath them..
   e.g.there should be no threads waiting n the condition, nor blocked on the
   queue.
*/
template<class T>
CBufferQueue<T>::~CBufferQueue<T>() 
{
  // Not really safe since behavior if the condvar is owned is not defined.
  //pthread_cond_destroy(&m_condition);
}

/*!
   Enter an element in the queue.  This requires synchronization with any
   other users  of the queue:
   \param object : T
     The object to enter in the queue (note if T is a pointer this is a copy
     free entry.

     \note this function can block the thread.
*/
template<class T> void
CBufferQueue<T>::queue(T object)
{
  Enter();
  m_queue.push_back(object);
  if (m_queue.size() > m_nWakeLevel) {
    wake();
  }
  Leave();
}

/*!
   Remove an element from the queue.  The front element of the queue
   is retrieved and removed.  If there are no elements in the
   queue, wait() is invoked which blocks until there are at least m_nWakelevel
   (set by setWakeThreshold) elements in the queue.
*/
template<class T> T
CBufferQueue<T>::get()
{

  T element;
  while (!getnow(element)) {
    wait();
  }
  return element;

}
/**
 * Get an element from the front of the queue without waiting.
 * If no element is available, return immediately anyway.
 *
 * @param element - the object that is gotten from the queue
 *                  valid only if there is an element.
 * @return bool   - true if an element was gotten, false otherwise.
 */
template<class T> bool
CBufferQueue<T>::getnow(T& element)
{
  bool result;
  Enter();
  if (m_queue.empty()) {
    result = false;
  } else {
    element = m_queue.front();
    m_queue.pop_front();
    result = true;
  }
  Leave();
  return result;
}


/*!
    Return a std::list that consists of all elements in the queue.
    if the queue is empty, this will be an empty list.
*/
template<class T> std::list<T>
CBufferQueue<T>::getAll()
{
  list<T> result;
  Enter();

  // Something I don't understand is wrong with the code below:
  //  typename std::list<T>::iterator f = m_queue.begin();
  //  typename std::list<T>::iterator b = m_queue.end();
  //  result.insert(result.begin(), f, b);
  // So substitute this code which is what I'm trying to do.

  while(!m_queue.empty()) {
    result.push_back(m_queue.front());
    m_queue.pop_front();
  }

  Leave();
  return result;
}
/*!
   Queue blocking operates on the basis of a low watermark called the
   Wake level. A wake is done if an insertion causes the number of elements 
   in the queue to become greater than the wake level (thus a wake level of 
   0 will wake on all insertions an entry.  This scheme allows the user
   to implement an amortized queue where the dequeueing thread waits empties
   the list and then does a wait() before doing the next get.

   This function sets the wake level.
   \param level : size_t
      The new wake level.
*/
template<class T> void
CBufferQueue<T>::setWakeThreshold(size_t level)
{
  m_nWakeLevel = level;
}
/*!
   Waits for the next wake().  Note that in cases where there is more than
   one consuming thread for the queue, there is no gaurentee that there
   will be elements in the queue on return from this function.  
   Suppose there are two threads a and b, and both of them issue a wait().
   when wake happens, if only one processor is avaiable, say thread gets scheduled
   and then does a getall() emptying the queue.  If thread b is not scheduled
   prior to the next queue then it will run but the queue will be empty.

   So the only gaurentee you have on exit is that at some point the wake
   level was exceeded. Note that to deal with this, the get member does
   something like:
\verbatim
Enter();
while (m_queue.empty()) {
   Leave();
   wait();
   Enter();
}
   // and now it owns the queue semaphore with entries in the queue.

\endverbatim
@param timeout - number of milli-seconds to wait for the signal.  -1 means no timeout.

  @note - a timed wait is used because there's a small timing hole in get:
          if, between the check for data and the wait acquiring the mutex
          the entire queue is filled, the condition variable will never be
          signalled.  By timing out the wait (.5sec) when this rare case
          occurs, the wait will time out and then there will be a new check
          for data in the queue in get.
*/
template<class T> void
CBufferQueue<T>::wait(int timeout)
{
  Enter();			// Blocking on the condition var requires this.
  
  int status;
  if (timeout == -1) {
    struct timespec endwait = msToAbsTime(500);
    status = pthread_cond_timedwait(&m_condition, &mutex(), &endwait);
  } else {
    struct timespec abstime;
    
    
    struct timeval  now;
    struct timeval  dt;
    struct timeval  timeoutTime;
    gettimeofday(&now, NULL);        // now.
    
    // Express the timeout as a timeval



    dt.tv_sec   = timeout / 1000;   // Seconds;
    dt.tv_usec  = 1000*(timeout % (1000)); // Left over usec.
    timeradd(&now, &dt, &timeoutTime);   // abs time of timeout -> timeoutTime
    
    // Convert timeval -> timespec:
    
    abstime.tv_sec = timeoutTime.tv_sec;
    abstime.tv_nsec = timeoutTime.tv_usec * 1000;

    status = pthread_cond_timedwait(&m_condition, &mutex(), &abstime);
  }
  Leave();			// We return owning the semaphore.
  
  if (status && (status != ETIMEDOUT)) {
    throw CErrnoException("Waiting on buffer queue");
  }
 
}
/*!
    Wakes up all threads that are hanging around in wait().
    One reason a normal user might want to call this is if
    a thread is no longer going to enqueue buffers for some long time,
    but the queue wake level is > 0.   A wake here will ensure the buffer
    queue is flushed by the consumer thread.
*/
template<class T> void
CBufferQueue<T>::wake()
{
  int status = pthread_cond_broadcast(&m_condition);
  if (status) {
    throw CErrnoException("Waking up buffer queue waiters");
  }
}

/**
 *  msToAbsTime
 *    Convert a ms time offset into an absolute time (timespec).
 *
 *  @param ms - number of milliseconds.
 *  @return struct timespec - corresponding absolute time specification.
 */
template <class T> struct timespec
CBufferQueue<T>::msToAbsTime(unsigned ms)
{
  // Break this in to seconds and microseconds.
  
  unsigned secs = ms / 1000;
  unsigned msec = ms % 1000;
  
  // Get the time of day and add the secs/msec to it:
  
  struct timeval now;
  gettimeofday(&now, NULL);
  
  struct timespec result;
  
  result.tv_sec = now.tv_sec + secs;
  result.tv_nsec= (now.tv_usec + msec * 1000) * 1000;
  result.tv_sec += result.tv_nsec/(1000*1000*1000);
  result.tv_nsec = result.tv_nsec % (1000*1000*1000);
  
  return result;
}
