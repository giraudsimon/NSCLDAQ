/*****************************************************************************
**      Filename:   bt_event.h
**
**      Purpose:    Generic header for bt_event.c file.            
**                  The Goal here is to have a single portable 
**                  include/source file combination bt_event.h/.c that 
**                  will allow adding the trace component to a new
**                  driver on any supported OS platform.
**
**      $Revision: 1.4 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1996-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if !defined (_BT_EVENT_H)
#define _BT_EVENT_H

/*
**  Include files
*/

#ifdef __linux__
#include <linux/sched.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#include <assert.h>
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5) */

#elif defined (__vxworks)
#if defined(FCTACH)
#include <assert.h>
#include <semLib.h>
#include <tickLib.h>
#include "sbs_bsp_api.h"
#endif /* defined(FCTACH) */

#else
#include <assert.h>
#endif /* operating system */

/*****************************************************************************
**
**  Data Structures for bt_event.h 
**
*****************************************************************************/


#if defined(__sun)

#if !defined(bool_t)
#define bool_t int
#endif /* !defined(bool_t) */

typedef struct {
    kmutex_t mutex;
    kcondvar_t condvar;
    volatile bool_t binary_sema;
} bt_event_t;

#elif defined(__sgi)

typedef sema_t bt_event_t;

#elif defined(__vxworks)

typedef SEM_ID bt_event_t;

#elif defined(__winxp)

typedef KEVENT bt_event_t;

#elif defined(BT_uCOS)

typedef OS_EVENT * bt_event_t;

#elif defined(__linux__)

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
typedef struct_timer_list bt_event_timer_list_t;
#else

typedef struct _bt_event_timer_list_t {
  struct timer_list   timer;
  void*               data;
  void(*function)(unsigned long);
} bt_event_timer_list_t;


#endif

typedef struct {
    int			irq_excl;	/* == 0 if not used from IRQ */
    struct semaphore 	sem;		/* Semaphore */
    unsigned int	timed_out;	/* Flag: Have we timed out */

    bt_event_timer_list_t timer;		/* Used for timeout function */
} bt_event_t;


#elif defined (__lynxos)

typedef int bt_event_t;

#else /* Havent't written yet */

#error Code not written yet

#endif /* Define data structures for each operating system */


#if defined (__sgi)
    /* left out of sgi header files */
    extern void sleep_init(sleep_t *lockp, int arg, lkinfo_t *lkinfop);
#endif /* sgi */

/*****************************************************************************
**
**  Protoypes and defineitions for bt_event.c
**
*****************************************************************************/

#ifdef BT13908
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_event_init)
#pragma alloc_text(PAGE, btk_event_fini)
#pragma alloc_text(PAGE, btk_event_set)
#pragma alloc_text(PAGE, btk_event_wait)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

/* Prototype for btk_event_init() includes OS dependent parameters */
#if defined (_AIX)

#elif defined (BT_NTDRIVER)
int btk_event_init(
        bt_event_t *event_p, 
        bool_t state);

#elif defined (__hpux)

#elif defined (__sgi)
int btk_event_init(
        bt_event_t *event_p, 
        bool_t state);

#elif defined (__sun)
int btk_event_init(
        bt_event_t *event_p, 
        bool_t state, 
        char *name_p, 
        ddi_iblock_cookie_t *cookie_p); 

#elif defined (__vxworks)
int btk_event_init(
        bt_event_t *event_p, 
        bool_t state);

#elif defined (BT_uCOS)
int btk_event_init(
        bt_event_t *event_p, 
        bool_t state);

#elif defined (__linux__)

int btk_event_init(
        bt_event_t *event_p, 
        bool_t state, 
        bt_cookie_t irq_cookie);

#elif defined (__lynxos)

int btk_event_init(
        bt_event_t *event_p, 
        bool_t state);

#else
#error unknown operating system
#endif /* operating system */

void btk_event_fini(
        bt_event_t *event_p);

bt_error_t btk_event_wait(
        bt_event_t *event_p, 
        bt_tck_t timeout);

void btk_event_set(
        bt_event_t *event_p);

#endif /* !defined(_BT_EVENT_H) */
