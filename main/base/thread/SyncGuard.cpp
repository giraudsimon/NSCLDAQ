/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/



#include <Synchronizable.h>
#include <dshwrapthreads.h>
#include <SyncGuard.h>



using namespace std;

static Synchronizable __default_global_synchronizable__;

#undef DEBUG_SYNCGUARD 

/*===================================================================*/
/** @fn SyncGuard::SyncGuard()
* @brief Default constructor.
*                                        
* Default constructor.  Operates on a global synchronizable object.
*                                         
* @param None
* @return this                 
*/      
SyncGuard::SyncGuard() : syncer(__default_global_synchronizable__) 
{
  lock();
}

/*===================================================================*/
/** @fn SyncGuard::SyncGuard(Synchronizable& aSync)
* @brief Constructor with a synchronizable reference.
*                                        
* Constructor with a synchronizable reference.
*                                         
* @param aSync A reference to a synchronizable object.
* @return this                 
*/      
SyncGuard::SyncGuard(Synchronizable& aSync) : syncer(aSync) 
{
  lock();
}

/*===================================================================*/
/** @fn SyncGuard::SyncGuard(Synchronizable& aSync,bool justtry)
* @brief Constructor with a synchronizable reference.
*                                        
* Constructor with a synchronizable reference.  If the the justtry
* parameter is true then we only try to lock the mutex.  
*                                         
* @param aSync A reference to a synchronizable object.
* @param justtry True if you should try to lock the mutex.
* @return this                 
*/      
SyncGuard::SyncGuard(Synchronizable& aSync,bool justtry) : syncer(aSync) 
{
  if (justtry) trylock();
  else lock(); 
}

/*===================================================================*/
/** @fn SyncGuard::~SyncGuard()
* @brief Destructor.
*                                        
* Destructor method for this class. 
*                                         
* @param None
* @return None 
*/      
SyncGuard::~SyncGuard() {
  if (syncer.sync != NULL) {
    dshwrapthread_mutex_lock(&(syncer.sync->__monitor_mutex__));



    // Require that I actually own the mutex:
    if (owner()) {
      syncer.sync->__lock_depth__--;
      if(syncer.sync->__lock_depth__ <= 0) {  // Release mutex if completely unlocked.
#ifdef DEBUG_SYNCGUARD
  cerr << "SyncGuard: unlock(" << syncer.sync->__guard_mutex__.lastmod << "," << (void*)(&syncer) << ")" << endl;
#endif
        dshwrapthread_mutex_unlock(&(syncer.sync->__guard_mutex__));
      }
    }

    dshwrapthread_mutex_unlock(&(syncer.sync->__monitor_mutex__));
  }
}

/*===================================================================*/
/** @fn dshwrapthread_t SyncGuard::getOwner()
* @brief Get the thread id of the owning thread.
*                                        
* Get the thread id of the thread that owns the synchronizable lock.
*                                         
* @param None
* @return The owning thread id.
*/      
dshwrapthread_t SyncGuard::getOwner() {
  createIfNeeded();
  return syncer.sync->__guard_mutex__.lastmod;
}

/*===================================================================*/
/** @fn void SyncGuard::lock()
* @brief Lock the synchronizable object.
*                                        
* Lock the synchronizable object.
*                                         
* @param None
* @return None
*/      
void SyncGuard::lock() {
  createIfNeeded();
  dshwrapthread_mutex_lock(&(syncer.sync->__monitor_mutex__));

  if (owner()) {
    syncer.sync->__lock_depth__++;
  } else {
    dshwrapthread_mutex_unlock(&(syncer.sync->__monitor_mutex__));
    // Compete for the lock with the owner of the lock.
    dshwrapthread_mutex_lock(&(syncer.sync->__guard_mutex__));
    dshwrapthread_mutex_lock(&(syncer.sync->__monitor_mutex__));
    syncer.sync->__lock_depth__ = 1;
#ifdef DEBUG_SYNCGUARD
  cerr << "SyncGuard: lock(" << syncer.sync->__guard_mutex__.lastmod << "," << (void*)(&syncer) << ")" << endl;
#endif
  }

  dshwrapthread_mutex_unlock(&(syncer.sync->__monitor_mutex__));
}

/*===================================================================*/
/** @fn void SyncGuard::trylock()
* @brief Try to ock the synchronizable object.
*                                        
* Try to lock the synchronizable object.
*                                         
* @param None
* @return None
*/      
void SyncGuard::trylock() {
  
  createIfNeeded();
  dshwrapthread_mutex_lock(&(syncer.sync->__monitor_mutex__));

  if (owner()) {
    syncer.sync->__lock_depth__++;
  } else {
    // Compete for the lock with the owner of the lock.
    if (dshwrapthread_mutex_trylock(&(syncer.sync->__guard_mutex__)) == 0) {
      // Got the lock
      syncer.sync->__lock_depth__ = 1;
#ifdef DEBUG_SYNCGUARD
  cerr << "SyncGuard: trylock(" << syncer.sync->__guard_mutex__.lastmod << "," << (void*)(&syncer) << ")" << endl;
#endif
    }  
  }

  dshwrapthread_mutex_unlock(&(syncer.sync->__monitor_mutex__));
}
void SyncGuard::createIfNeeded()
{
  if (syncer.sync == NULL) syncer.sync =
    Synchronizable::_create_sync(syncer.sync);
}
bool
SyncGuard::owner()
{
  return
    (syncer.sync->__lock_depth__ > 0)  &&
    (syncer.sync->__guard_mutex__.lastmod == dshwrapthread_self());
}
