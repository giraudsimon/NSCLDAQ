/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/


#include <sys/time.h>
#include <time.h>
#include <Synchronizable.h>
#include <dshwrapthreads.h>
#include <MonitorException.h>

using namespace std;






/*===================================================================*/
/** @fn _SyncStruct::_SyncStruct()
* @brief Constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this                 
*/      
_SyncStruct::_SyncStruct() :
  __lock_depth__(0)
{ 
  dshwrapthread_mutex_init(&__guard_mutex__);
  dshwrapthread_mutex_init(&__monitor_mutex__);
  dshwrapthread_cond_init(&__notify_cond__);
}

/*===================================================================*/
/** @fn _SyncStruct::~_SyncStruct()
* @brief Destructor.
*                                        
* Destroy this object.
*                                         
* @param None
* @return None
*/      
_SyncStruct::~_SyncStruct() { 
  dshwrapthread_mutex_destroy(&__guard_mutex__); 
  dshwrapthread_mutex_destroy(&__monitor_mutex__); 
  dshwrapthread_cond_destroy(&__notify_cond__); 
}

/*===================================================================*/
/** @fn _SyncStruct* Synchronizable::_create_sync(_SyncStruct *sync)
* @brief Thread safe creator of synchronization structures.
*                                        
* Thread safe (static) creator of synchronization data structures.
*                                         
* @param sync Create only if sync is NULL.
* @return A pointer to a _SyncStruct.
*/      
_SyncStruct* Synchronizable::_create_sync(_SyncStruct* sync) {


  _SyncStruct *s = sync;
    try {
      if (sync == NULL) s = new _SyncStruct();
    } catch (...) {
      s = NULL;
    }

  return s;
}

/*===================================================================*/
/** @fn Synchronizable::Synchronizable()
* @brief Constructor.
*                                        
* Basic constructor method for this class.  By building the sync struct here,
* we avoid the need for all sorts of additional synchronization, improving
* performance as well as making the code simpler.
*                                         
* @param None
* @return this                 
*/      
Synchronizable::Synchronizable()  :
  sync(0)
{ 
  sync = _create_sync(sync);
}

/*===================================================================*/
/** @fn Synchronizable::~Synchronizable()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
Synchronizable::~Synchronizable() { 
  delete sync;
  sync = NULL;
}



/*===================================================================*/
/** @fn void Synchronizable::waitFor()
* @brief Wait for notification.
*                                        
* Wait for notification from another thread.
*                                         
* @param None
* @return None
* @throw CMonitorException If the thread calling notify is not synchronized on this object.
*/      
void Synchronizable::waitFor() {

  int lockdepth = Lock();

  // We should now have reliquished all control and saved state.
  // Now unlock the monitor and wait...
  dshwrapthread_cond_wait(&sync->__notify_cond__,&sync->__monitor_mutex__);

  // ... We should now have the monitor back

  Unlock(lockdepth);
}

/*===================================================================*/
/** @fn void Synchronizable::waitFor(long timeout,int nanos)
* @brief Wait for notification with a timed wait.
*                                        
* Wait for notification from another thread with a timed wait.
* Thread stops waiting due to a call to notify() for this object
* or when the timeout has expired.
*
* The thread calling notifyAll() must already be synchronized on 
* this object (using sync_begin/sync_end) or an exception will
* be thrown. 
*                                         
* @param timeout A timeout in seconds.
* @param nanos A timeout in nanoseconds.
* @return None
* @throw CMonitorException If the thread calling notify is not synchronized on this object.
*/      
void Synchronizable::waitFor(long timeout,int nanos) {


  struct timeval now;
  struct timespec abstime;



  int lockdepth = Lock();
  
 
  // Create the timeout

  gettimeofday(&now,NULL); // Get the current time
  abstime.tv_sec = now.tv_sec + timeout; 
  long nsec = (now.tv_usec * 1000) + nanos;
  abstime.tv_nsec = nsec;

  // We should now have reliquished all control and saved state.
  // Now unlock the monitor and wait...
  dshwrapthread_cond_timedwait(&sync->__notify_cond__,&sync->__monitor_mutex__,&abstime);

  // ... We should now have the monitor back

  Unlock(lockdepth);
}

/*===================================================================*/
/** @fn void Synchronizable::waitFor(long timeout)
* @brief Wait for notification with a timed wait.
*                                        
* Wait for notification from another thread with a timed wait.
* Thread stops waiting due to a call to notify() for this object
* or when the timeout has expired.
*
* The thread calling notifyAll() must already be synchronized on 
* this object (using sync_begin/sync_end) or an exception will
* be thrown. 
*                                         
* @param timeout A timeout in seconds.
* @return None
*/      
void Synchronizable::waitFor(long timeout) {
  waitFor(timeout,0);
}

/*===================================================================*/
/** @fn void Synchronizable::notify()
* @brief Notify a thread waiting on this object.
*                                        
* Notify a thread waiting for this object.  One of the threads
* that has called one of the waitFor() methods on this object will
* be allowed to continue.  The newly awoken thread can then compete
* for this object's mutex or go about other business.
*
* The thread calling notify() must already be synchronized on 
* this object (using sync_begin/sync_end) or an exception will
* be thrown. 
* 
* @param None
* @return None
* @throw CMonitorException If the thread calling notify is not synchronized on this object.
*/      
void Synchronizable::notify() {

  if ((sync->__lock_depth__ <= 0)||(sync->__guard_mutex__.lastmod != dshwrapthread_self())) {
    throw CMonitorException((int)dshwrapthread_self(),(int)sync->__guard_mutex__.lastmod, __FILE__, "__LINE__");

  } 
  dshwrapthread_cond_signal(&sync->__notify_cond__);
}

/*===================================================================*/
/** @fn void Synchronizable::notifyAll()
* @brief Notify all the threads waiting on this object.
*                                        
* Notify all the threads waiting for this object.  All of the threads
* that have called one of the waitFor() methods on this object will
* be allowed to continue.  The newly awoken threads can then compete
* for this object's mutex or go about other business.
*
* The thread calling notifyAll() must already be synchronized on 
* this object (using sync_begin/sync_end) or an exception will
* be thrown. 
* 
* @param None
* @return None
* @throw CMonitorException If the thread calling notify is not synchronized on this object.
*/      
void Synchronizable::notifyAll() {

  if ((sync->__lock_depth__ <= 0)||(sync->__guard_mutex__.lastmod != dshwrapthread_self())) {
    throw CMonitorException((int)dshwrapthread_self(),(int)sync->__guard_mutex__.lastmod, __FILE__, "__LINE__");
  } 
  dshwrapthread_cond_broadcast(&sync->__notify_cond__);
}

/*
  Lock the data structures to prepare for condition waits.
  Returns the lock depth.
*/
int
Synchronizable::Lock()
{
  if ((sync->__lock_depth__ <= 0)||(sync->__guard_mutex__.lastmod != dshwrapthread_self())) {
    throw CMonitorException( (int)dshwrapthread_self(),(int)sync->__guard_mutex__.lastmod, __FILE__, "__LINE__");

  } 

  dshwrapthread_mutex_lock(&sync->__monitor_mutex__); // Reacquire the monitor

  int lockdepth = sync->__lock_depth__;  // Save the lock depth
  sync->__lock_depth__ = 0; // Reset the lock depth

  dshwrapthread_mutex_unlock(&sync->__guard_mutex__); // Give up the guard...

  return lockdepth;
}

/*
  Release the data structures and reset the lock depth:
*/
void
Synchronizable::Unlock(int lockdepth)
{
  dshwrapthread_mutex_lock(&sync->__guard_mutex__); // Reacquire the guard
  sync->__lock_depth__ = lockdepth; // Reset the lock depth
  dshwrapthread_mutex_unlock(&sync->__monitor_mutex__); // Give up the monitor

}
