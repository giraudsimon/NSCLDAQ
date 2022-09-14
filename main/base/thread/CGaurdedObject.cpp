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

//#include <config.h>
#include <CGaurdedObject.h>
#include <ErrnoException.h>
#include <pthread.h>

/*!
  Create the object by initializing the mutex to be recursive.
*/
CGaurdedObject::CGaurdedObject()
{
  m_mutex = new pthread_mutex_t;
  pthread_mutexattr_t attrib;

  pthread_mutexattr_init(&attrib);
  pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE_NP);

  if (pthread_mutex_init(m_mutex, &attrib)) {
    throw CErrnoException("CGaurdedObject construction creating mutex");
  }
			 
}

/*!
   Free the mutex on destruction
*/
CGaurdedObject::~CGaurdedObject()
{
  pthread_mutex_destroy(m_mutex);
  delete m_mutex;
}


/*!
   Enter a critical region. This is just locking the mutex.  Our thread
   will block until the mutex is released.
*/
void
CGaurdedObject::Enter()
{
  int status = pthread_mutex_lock(m_mutex);
  if (status) {
    throw CErrnoException("Entering gaurded object");
  }
}
/*!
   Leave a critical region.  THis is just unlocking the mutex.
   Most likely this causes a scheduler pass if there are threads that
   are queud in the Enter... 
*/
void
CGaurdedObject::Leave()
{
  int status = pthread_mutex_unlock(m_mutex);
  if (status) {
    throw CErrnoException("Leaving gaurded object");
  }
}
/*!
   Returns a reference to the mutex.  This is required for objects that
   may also want to implement a condition variable on top of this.
   For example the CBufferQueue class.

   @return pthread_mutex&
   @retval m_mutex
*/
pthread_mutex_t&
CGaurdedObject::mutex()
{
  return *m_mutex;
}
