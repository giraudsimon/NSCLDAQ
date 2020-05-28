#ifndef DAQHWYAPI_SYNCGUARD_H
#define DAQHWYAPI_SYNCGUARD_H

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
#include <string>
#include <iomanip>



#ifndef DSHWRAPTHREADS_H
#include <dshwrapthreads.h>
#endif



class Synchronizable;

/*=====================================================================*/
/**
* @class SyncGuard
* @brief Guard class for synchronizable objects.
*
* Thread guard class for synchronizable objects.  This class
* should be constructed inside a scope implicitly (without using
* new).  When the scope is exited, the guard will automatically
* destruct and release the mutex on the synchroinziable object
* provided during construction.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class SyncGuard
{
  // Member data:
private:
    Synchronizable& syncer; 

public:
  SyncGuard();			// Constructor 
  SyncGuard(Synchronizable&);   // Constructor 
  SyncGuard(Synchronizable&,bool); // Constructor 
  virtual ~SyncGuard();		// Destructor
  dshwrapthread_t getOwner();	// Get the lock owner
  
  // Utilities:

private:
  void lock();			// Lock the synchronizable
  void trylock();		// Try to lock the synchronizable
	void createIfNeeded();  // Make syncer.sync if needed.
	bool owner();     // True if I already own the mutex.
};


#define sync_self { SyncGuard __syncguard__((Synchronizable&)(*this));
#define sync_begin(s) { SyncGuard __syncguard__((Synchronizable&)(s));
#define sync_begin2(t,s) { SyncGuard __syncguard__##t((Synchronizable&)(s));
#define sync_end }


#define sync_trybegin(s) { \
  SyncGuard __syncguard__((Synchronizable&)(s),true); \
  if (__syncguard__.getOwner() == dshwrapthread_self()) {

#define sync_trybegin2(t,s) { \
  SyncGuard __syncguard__##t((Synchronizable&)(s),true); \
  if (__syncguard__##t.getOwner() == dshwrapthread_self()) {

#define sync_tryend }}

#define sync_global_begin(t) { SyncGuard __syncguard__##t;
#define sync_global_end }


#endif
