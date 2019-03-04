#ifndef DSHWRAPTHREADS_H
#define DSHWRAPTHREADS_H

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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

  #include <dshwrappthreads.h>
  #define dshwrapthread_create dshwrappthread_create
  #define dshwrapthread_exit dshwrappthread_exit
  #define dshwrapthread_detach dshwrappthread_detach
  #define dshwrapthread_self dshwrappthread_self
  #define dshwrapthread_join dshwrappthread_join
  #define dshwrapthread_mutex_init dshwrappthread_mutex_init
  #define dshwrapthread_mutex_lock dshwrappthread_mutex_lock
  #define dshwrapthread_mutex_unlock dshwrappthread_mutex_unlock
  #define dshwrapthread_mutex_trylock dshwrappthread_mutex_trylock
  #define dshwrapthread_mutex_destroy dshwrappthread_mutex_destroy
  #define dshwrapthread_cond_init dshwrappthread_cond_init
  #define dshwrapthread_cond_wait dshwrappthread_cond_wait
  #define dshwrapthread_cond_signal dshwrappthread_cond_signal
  #define dshwrapthread_cond_destroy dshwrappthread_cond_destroy
  #define dshwrapthread_cond_broadcast dshwrappthread_cond_broadcast
  #define dshwrapthread_cond_timedwait dshwrappthread_cond_timedwait
  #define dshwrapthread_tostr dshwrappthread_tostr
  #define dshwrapthread_mutex_lastmod dshwrappthread_mutex_lastmod


#endif
