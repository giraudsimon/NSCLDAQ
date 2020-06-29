#ifndef DSHWRAPPTHREADS_H
#define DSHWRAPPTHREADS_H

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
#include <signal.h>
#include <unistd.h>
#include <errno.h>


#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>

typedef pthread_t dshwrapthread_t; 
typedef pthread_attr_t dshwrapthread_attr_t; 
typedef void *(*dshwrapthread_start_routine)(void*);
typedef pthread_cond_t dshwrapthread_cond_t; 

typedef struct _dshwrapthread_mutex_t_struct {
  pthread_mutex_t themutex;    // the actual mutex
  dshwrapthread_t     lastmod;     // thread that last locked this mutex
} dshwrapthread_mutex_t;


#define DSHTHREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define DSHTHREAD_COND_INITIALIZER PTHREAD_COND_INITIALIZER

// Basic functions
int dshwrappthread_create(dshwrapthread_t*,dshwrapthread_attr_t*,dshwrapthread_start_routine,void*);
void dshwrappthread_exit(void*);
int dshwrappthread_detach(dshwrapthread_t);
dshwrapthread_t dshwrappthread_self(void);
int dshwrappthread_join(dshwrapthread_t,void**);

// Mutexes
int dshwrappthread_mutex_init(dshwrapthread_mutex_t*);
int dshwrappthread_mutex_lock(dshwrapthread_mutex_t*);
int dshwrappthread_mutex_unlock(dshwrapthread_mutex_t*);
int dshwrappthread_mutex_trylock(dshwrapthread_mutex_t*);
int dshwrappthread_mutex_destroy(dshwrapthread_mutex_t*);

// Mutex query functions
dshwrapthread_t dshwrappthread_mutex_lastmod(dshwrapthread_mutex_t*);

// Condition vars
int dshwrappthread_cond_init(dshwrapthread_cond_t*);
int dshwrappthread_cond_wait(dshwrapthread_cond_t*,dshwrapthread_mutex_t*);
int dshwrappthread_cond_signal(dshwrapthread_cond_t*);
int dshwrappthread_cond_destroy(dshwrapthread_cond_t*);
int dshwrappthread_cond_broadcast(dshwrapthread_cond_t*);
int dshwrappthread_cond_timedwait(dshwrapthread_cond_t*,dshwrapthread_mutex_t*,const struct timespec*);

// How do we display this thing? 
char *dshwrappthread_tostr(char*,int,dshwrapthread_t);



#endif
