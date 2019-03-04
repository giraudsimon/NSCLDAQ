#ifndef DAQHWYAPI_SYNCHRONIZABLE_H
#define DAQHWYAPI_SYNCHRONIZABLE_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include <dshwrapthreads.h>

// forward class references.

class SyncGuard;

/*=====================================================================*/
/**
* @class _SyncStruct
* @brief Container for synchronization data structures.
*
* Container for synchronization data structures.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class _SyncStruct {
  public:
    _SyncStruct();
    virtual ~_SyncStruct();

    /**
    * @var __notify_cond__
    * @brief The waitfor()/notify() thread conditional.
    *
    * The condition variable  used for implementing the waitfor()/notify() methods.
    */
    dshwrapthread_cond_t __notify_cond__;

    /**
    * @var __guard_mutex__
    * @brief The mutex.
    *
    * The actual mutex that is locked.
    */
    dshwrapthread_mutex_t __guard_mutex__;

    /**
    * @var __monitor_mutex__
    * @brief The monitor mutex.
    *
    * The monitor mutex that protects the recursive structures
    * used to implement the recursive mutex.
    */
    dshwrapthread_mutex_t __monitor_mutex__;

    /**
    * @var __lock_depth__
    * @brief The depth to which this mutex has been locked.
    *
    * The depth to which the owning thread has locked this mutex.
    */
    unsigned int      __lock_depth__;
};

/*=====================================================================*/
/**
* @class Synchronizable
* @brief Thread synchronizable class.
*
* Thread synchroinziable class.  Class that can be used for
* synchronization should inherit from this class.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Synchronizable 
{

 private:   
  _SyncStruct *sync;

  
public:
  Synchronizable();   // Constructor 
  virtual ~Synchronizable();  // Destructor
  
  void waitFor();     // Wait on this object
  void waitFor(long); // Wait on this object with timeout
  void waitFor(long,int); // Wait on this object with timeout
  void notify();      // Notify a thread waiting on this object 
  void notifyAll();   // Nofify all threads waiting on this object
  
  // Factorized utilities:

private:
  int Lock();
  void Unlock(int);

protected:
  static _SyncStruct* _create_sync(_SyncStruct*);  

  friend class SyncGuard;


};



#endif
