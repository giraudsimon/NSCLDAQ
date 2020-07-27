/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

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

#include <CVMEInterface.h>
#include <ErrnoException.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <time.h>
#include <string>

using namespace std;


// In some cases we need to define the semun union:

#if defined(__GNU_LIBRARY__) && !(defined(_SEM_SEMUN_UNDEFINED))
#else
union semun {
  int                val;	// SETVAL value.
  struct semid_ds    *buf;	// IPC_STAT, IPC_SET buffer.
  unsigned short int *array;	// Array for GETALL/SETALL
  struct seminfo*    _buf;	// Buffer for IPC_INFO
};
#endif


/*!
   This file implements coarse grained VME locking.
   If applications use this, the entire VME subsystem will be controlled
   by a single lock.  
*/

static int semid = -1;		// This will be the id of the locking semaphore
static int semkey= 0x564d4520;  // "VME " :-).

/**
 * doSemop
 *    Utility static function to perform a semaphore operation
 *    retrying if interrupted by signal
 *  @param semid - sempahore id.
 *  @param buf references the semaphore buffer.
 *  @throw CErrnoException if semop failed.
 */
static void
doSemop(int semid, sembuf& buf)
{
 while (1) {			// In case of signal...
    int stat = semop(semid, &buf, 1);

    if(stat == 0) break;

    if(errno != EINTR) {	// Bad errno:
      throw CErrnoException("CVMEInterface::Lock semop gave bad status");
    }
    // On EINTR try again.
  }
}

/*!
   This internal function is used to establish the semaphore:
   - If the semaphore exists, it's id is just stored in semid.
   - If the semaphore does not exist we try to create it O_EXCL
     this is an attempt to deal with any timing holes that may
     occur when two programs simultaneously attemp to create the semaphore.
   - If the O_EXCL creation succeeds (semaphore does not exist), it is
     given in initial value of 1 so that a single process can pass the lock
     gate.
   - If the O_EXCL creation fails, the process backs off for a while and
     then does a semget for an existing  semaphore again assuming that on the 
     second try, all initialization has been complete.

   \throw   CErrnoException
      If an error occured on any of the system calls.

*/
void
CVMEInterface::AttachSemaphore()
{
  // Retry loop in case anybody makes and then kills it:

  while(1) {
    // Try to get the id of an existing semaphore:

    semid = semget(semkey, 0, 0777); // Try to map:
    if(semid >= 0) break;	     // Previously existing!!
    if(errno != ENOENT) {
      throw 
	CErrnoException("AttachSemaphore - semget error unexpected");
    }
    // Sempahore does not exist.  Try to be the only guy to 
    // create it:

    semid = semget(semkey, 8, 0777 | IPC_CREAT | IPC_EXCL);
    if(semid >= 0) {
      // We're the creator... initialize the sempahore, and return.

      union semun data;
      data.val = 1;

      for (int i = 0; i < 8; i++) {

	int istat = semctl(semid, i, SETVAL, data); // Allow 1 holder
	if(istat < 0) {
	  throw CErrnoException("AttachSemaphore - semctl error unexpected");
	}
	
      }
      break;
    }
    if(errno != EEXIST) {
      throw
	CErrnoException("AttachSemaphore - semget error unexpected");
    }
    //   The semaphore popped into being between the initial try
    //   to just attach it and our try to create it.
    //   The next semget should work, but we want to give
    //   the creator a chance to initialize the semaphore so
    //   we don't try to take out a lock on the semaphore before
    //   it is completely initialized:

    sleep(1);
  }
  return;
}
/*!
  Lock the transfer semaphore (semaphore 0).

*/

void CVMEInterface::Lock()
{
  Lock(0);
}
/*!
  Unlock the transfer semaphore (semaphore 0).
*/
void CVMEInterface::Unlock()
{
  Unlock(0);
}

bool CVMEInterface::TryLock(int timeoutSeconds) 
{
  return TryLock(0, timeoutSeconds);
}
/*!
    Lock the semaphore.  If the semid is -1, the
    semaphore is created first.  

    \throw CErrnoException
       - Really from AttachSemaphore
       - From failures in semop.
*/
void 
CVMEInterface::Lock(int semnum) 
{
  // If necessary, get the semaphore id..

  if (semid == -1) AttachSemaphore();
  assert(semid >= 0);		// Otherwise attach.. throws.

  struct sembuf buf;
  buf.sem_num = semnum;		
  buf.sem_op  = -1;		// Want to take the semaphore.
  buf.sem_flg = SEM_UNDO;	// For process exit.
	doSemop(semid, buf);
  
  return;
}
/*!
  Unlock the semaphore. It is a crime to unlock the semaphore if it doesn
  not yet exist, since that would be unlocking a semaphore that is not yet
  locked.
  
  \throw  CErrnoException
     If the semop operation produced an error.
  \throw string
     If the semaphore did not yet exist.
*/
void
CVMEInterface::Unlock(int semnum)
{
  if(semid == -1) {
    throw string("Attempt to unlock the semaphore before it was created");
  }
  struct sembuf buf;
  buf.sem_num = semnum;
  buf.sem_op  = 1;
  buf.sem_flg= SEM_UNDO;	// Undoes the locking undo.
	doSemop(semid, buf);
  
  return;
}


bool
CVMEInterface::TryLock(int semnum, int nTimeoutSeconds) {
  if (semid == -1) AttachSemaphore();
  assert(semid >= 0);
  

  bool success = false;
//  int semval = semctl(semid, semnum, GETVAL);
//  if (semval == 1) {
    // there is no one holding the semaphore...
    // try to acquire it.
    struct sembuf buf;
    buf.sem_num = semnum;
    buf.sem_op = -1;         // acquire the sem
    buf.sem_flg = SEM_UNDO;  

    struct timespec timeout;
    timeout.tv_sec = nTimeoutSeconds;
    timeout.tv_nsec = 0;

    int stat = semtimedop(semid, &buf, 1, &timeout);
    if(stat == 0) { 
      success = true; 
    } else {
      // failure observed...

			// Interruption is taken to mean the trylock failed.
			
      if ((errno == EAGAIN) || (errno == EINTR)) {
        // interrupted by signal or we were unable to take the semaphore
        success = false;
      } else {
        // something bad happened. This should fail rather than returning false.
        throw CErrnoException("CVMEInterface::TryLock semop gave bad status");
      }

    }
//  } else {
//    // someone is holding the semaphore
//    success = false;
//  }
  return success;
}
