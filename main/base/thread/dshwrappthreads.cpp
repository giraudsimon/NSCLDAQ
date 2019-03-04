/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/



#ifdef USE_PTHREADS


#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <dshwrappthreads.h>
#include <pthread.h>

/*================================================================*/
int dshwrappthread_create(dshwrapthread_t *pThread,dshwrapthread_attr_t *pAttr,dshwrapthread_start_routine aStart,void *pArgs) 
{
  int rc;

  rc = pthread_create((pthread_t *)pThread,(pthread_attr_t*)pAttr,aStart,pArgs);
  errno = rc; 
  return(rc);
}

/*================================================================*/
void dshwrappthread_exit(void *aRc)
{
  pthread_exit(aRc);
}

/*================================================================*/
int dshwrappthread_detach(dshwrapthread_t aThread)
{
  int rc;

  rc = pthread_detach((pthread_t)aThread);
  errno = rc; 
  return(rc);
}

/*================================================================*/
dshwrapthread_t dshwrappthread_self(void)
{
  return((dshwrapthread_t)pthread_self());
}

/*================================================================*/
int dshwrappthread_join(dshwrapthread_t aThread,void **pStatus)
{
  int rc;

  rc = pthread_join((pthread_t)aThread,pStatus);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_mutex_init(dshwrapthread_mutex_t *pMutex)
{
  int rc;

  pMutex->lastmod = (dshwrapthread_t)pthread_self();
  rc = pthread_mutex_init((pthread_mutex_t*)&pMutex->themutex,NULL);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_mutex_lock(dshwrapthread_mutex_t *pMutex)
{
  int rc;

  rc = pthread_mutex_lock((pthread_mutex_t*)&pMutex->themutex);
  if (rc == 0) pMutex->lastmod = (dshwrapthread_t)pthread_self();

  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_mutex_unlock(dshwrapthread_mutex_t *pMutex)
{
  int rc;

  rc = pthread_mutex_unlock((pthread_mutex_t*)&pMutex->themutex);

  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_mutex_trylock(dshwrapthread_mutex_t *pMutex)
{
  int rc;

  rc = pthread_mutex_trylock((pthread_mutex_t*)&pMutex->themutex);
  if (rc == 0) pMutex->lastmod = (dshwrapthread_t)pthread_self();

  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_mutex_destroy(dshwrapthread_mutex_t *pMutex)
{
  int rc;

  rc = pthread_mutex_destroy((pthread_mutex_t*)&pMutex->themutex);
  pMutex->lastmod = (dshwrapthread_t)pthread_self();

  errno = rc; 
  return(rc);
}

/*================================================================*/
dshwrapthread_t dshwrappthread_mutex_lastmod(dshwrapthread_mutex_t *pMutex)
{
  return(pMutex->lastmod);
}

/*================================================================*/
int dshwrappthread_cond_init(dshwrapthread_cond_t *pCond)
{
  int rc;

  rc = pthread_cond_init((pthread_cond_t*)pCond,NULL);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_cond_wait(dshwrapthread_cond_t *pCond,dshwrapthread_mutex_t *pMutex)
{
  int rc;

  rc = pthread_cond_wait((pthread_cond_t*)pCond,(pthread_mutex_t*)&pMutex->themutex);
  if (rc == 0) pMutex->lastmod = (dshwrapthread_t)pthread_self();
   
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_cond_signal(dshwrapthread_cond_t *pCond)
{
  int rc;

  rc = pthread_cond_signal((pthread_cond_t*)pCond);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_cond_destroy(dshwrapthread_cond_t *pCond)
{
  int rc;

  rc = pthread_cond_destroy((pthread_cond_t*)pCond);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_cond_broadcast(dshwrapthread_cond_t *pCond)
{
  int rc;

  rc = pthread_cond_broadcast((pthread_cond_t*)pCond);
  errno = rc; 
  return(rc);
}

/*================================================================*/
int dshwrappthread_cond_timedwait(dshwrapthread_cond_t *pCond,dshwrapthread_mutex_t* pMutex,const struct timespec *pAbsTime)
{
  int rc;

  rc = pthread_cond_timedwait((pthread_cond_t*)pCond,(pthread_mutex_t*)&pMutex->themutex,pAbsTime);
  if (rc == 0) pMutex->lastmod = (dshwrapthread_t)pthread_self();

  errno = rc; 
  return(rc);
}

/*================================================================*/
char *dshwrappthread_tostr(char* aStr,int aLen,dshwrapthread_t aThread)
{
  if ((aStr == NULL)||(aLen < 1)) return(NULL);

#ifdef HAVE_SNPRINTF
  snprintf(aStr,aLen,"%ld",(long)aThread);
#else
  sprintf(aStr,"%ld",(long)aThread);
#endif

  aStr[aLen] = '\0';

  return(aStr);
}

#endif
