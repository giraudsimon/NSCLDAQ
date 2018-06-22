/*****************************************************************************
**
**      Filename:   bt_event.c
**
**      Purpose:    Kernel event notification routines.
**
**      Functions:      btk_event_init(), btk_event_fini(),
**                      btk_event_wait(), btk_event_set()
**
**      $Revision: 2330 $
**
*****************************************************************************/
/*****************************************************************************
**
**            Copyright (c) 1997-2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
#ifdef  __EPI__
__asm(".rdata");
__asm(".ascii\"@(#)"__FILE__"  $Revision: 2330 $ \\0\"");
__asm(".align 3; .text");
#else
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif  /* __EPI__*/
#endif  /* !LINT */


/*****************************************************************************
**
**      Include files
**
*****************************************************************************/

#include "btdd.h"

#ifdef __linux__

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#include <assert.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
/* Need a jacket to wrap the call to the real timeout: */
static void
bt_event_timeout_wrapper(struct timer_list* pList)
{
  /* Get a pointer to the event and then call the actual function: */

  bt_event_timer_list_t* pTimer = (bt_event_timer_list_t*)pList;
  struct bt_event_t*     pEvent = pTimer->data;
  (*pTimer->function)((unsigned long)pEvent);
}
#endif

#else

#include <assert.h>

#endif



/*****************************************************************************
**
**      Global exported data
**
*****************************************************************************/


#if defined (__sgi)
    /* left out of sgi header files */
    extern void sleep_init(sleep_t *lockp, int arg, lkinfo_t *lkinfop);
#endif /* sgi */

#ifdef BT13908
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, btk_event_init)
#pragma alloc_text(PAGE, btk_event_fini)
#pragma alloc_text(PAGE, btk_event_set)
#pragma alloc_text(PAGE, btk_event_wait)
#endif /* ALLOC_PRAGMA */
#endif /* BT13908 */

BT_FILE_NUMBER(TRACE_BT_EVENT_C);


/******************************************************************************
**
**	Linux local functions
**
******************************************************************************/

#if	defined(__linux__)

/******************************************************************************
**
**      Function:       timeout_event()
**
**      Purpose:        Wakes up the event if it isn't available in time.
**
**      Args:
**              vevent		Pointer to event structure.
**
**      Returns:
**              void
**
**      Note:
**      I have intentionally left out all of the debugging code in the
**      production driver.
******************************************************************************/

void timeout_event(
    unsigned long vevent
    )
{
    bt_event_t *event_p = (bt_event_t *) vevent;

#if	!defined(NDEBUG)
    FUNCTION("timeout_event");
    LOG_UNKNOWN_UNIT;
    FENTRY;
#endif	/* !defined(NDEBUG) */

    event_p->timed_out = TRUE;
    up(&event_p->sem);

#if	!defined(NDEBUG)
    FEXIT(0);
#endif	/* !defined(NDEBUG) */
    return;
}

#endif	/* defined(__linux__) */

/******************************************************************************
**
**      Function:       btk_event_init()
**
**      Purpose:        Initializes a kernel event object.
**
**      Creates the object. The 'state' parameter is the initial 
**      state of the condition, TRUE (full) or FALSE (empty). 
**      The parameters for this routine will vary depending on the 
**      operating system. Some operating systems may need additional 
**      information in order to create the object.
**
**
**      Args:
**              event_p         Pointer to bt_event_t to be intialized
**              state           Initial state of object:
**                  FALSE       Create the object empty
**                  TRUE        Create the object full
**
**              Solaris Specific parameters:
**
**              name_p          String name for event and mutex.
**              iblock_p        Interrupt cookie
**
**              VxWorks specific parameters:
**              None
**
**              Linux specific parameters:
**              cookie		Cookie indicating the highest process context
**                              this event will be accessed from.
**
**      Returns:
**              0               Success
**              Otherwise       error value.
**
**  Notes:
**      Storage for the event object pointed to by the event_p argumentmust
**      be resident, it cannot be in pageable memory.
**
**      The bt_event_t is an opaque type to be defined for each operating
**      system.
**
**      All of the event routines need to accept NULL for the unit pointer.
**      This will most likey only affect tracing.
**
**      The basic idea is to wait for the occurance of something, with a
**      timeout interval.  Three examples of use include creating a
**      locking mechanism, waiting for a DMA Done interrupt, and
**      waiting for a remote system reset to complete.
**
**  NT Notes:
**      Callers must be running at IRQL PASSIVE_LEVEL.
**      
******************************************************************************/
int btk_event_init(
    bt_event_t  *event_p,
    bool_t      state

#if     defined(__sun)
        ,
    char        *name_p,
    ddi_iblock_cookie_t *event_cookie_p
#elif	defined(__linux__)
        ,
    bt_cookie_t irq_cookie
#endif  /* __sun */
    )
{
    int         retvalue = 0;   /* Assume success */

    FUNCTION("btk_event_init");
    LOG_UNKNOWN_UNIT;
    FENTRY;

#if     defined(__sun)

    /* Need a mutex to protect the condition variable */
    mutex_init(&event_p->mutex, name_p, MUTEX_DRIVER, event_cookie_p);

    /* Now initialize the condition variable */
    cv_init(&event_p->condvar, name_p, CV_DRIVER, event_cookie_p);

    mutex_enter(&event_p->mutex);
    event_p->binary_sema = state;
    mutex_exit(&event_p->mutex);

#elif defined(__sgi)

    TRC_STR(BT_TRC_KLIB, "Initilize the sleep event.\n");

    initnsema(event_p, state, NULL);
    retvalue = BT_SUCCESS;


#elif   defined(__vxworks)

    if (TRUE == state) {
        *event_p = semCCreate(SEM_Q_PRIORITY, 1);
    } else {
        *event_p = semCCreate(SEM_Q_PRIORITY, 0);
    }

    if (NULL == event_p) {
        INFO_STR("Couldn't create semaphore.");
        retvalue = ENOMEM;
    } else {
        retvalue = BT_SUCCESS;
    }

#elif   defined(_NTDDK_)

    KeInitializeEvent(event_p, SynchronizationEvent, (BOOLEAN)state);


#elif   defined(BT_uCOS)

    /* create the semaphore and initilize the event to empty so a call to */
    /* OSSemPend() will decrement the semaphore count and places the */
    /* calling task in the waiting list for the semaphore. */
    *event_p = OSSemCreate(state); 
    if (NULL == *event_p) {
        INFO_STR("No event control block available.\n");
        retvalue = ENOMEM;
    } else {
        retvalue = BT_SUCCESS;
    }
    
#elif   defined(__linux__)
    event_p->irq_excl = irq_cookie;

#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    if (state) {
        sema_init(&event_p->sem, 1);
    } else {
        sema_init(&event_p->sem, 0);
    }
#else   /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
    if (state) {
        event_p->sem = MUTEX;
    } else {
        event_p->sem = MUTEX_LOCKED;
    }
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) || \
            LINUX_VERSION_CODE <  KERNEL_VERSION(2,4,0) */

    event_p->timed_out = FALSE;

#elif defined (__lynxos)

    /* Check to make sure the state is 0 on initializing */
    if (state != FALSE) {
        *event_p = 0;
    } else {
        *event_p = state;
    }

#else

#error  Code not written yet

#endif  /* __sun, __vxworks, _NTDDK_ */

    FEXIT(retvalue);
    return retvalue;
} /*  end btk_event_init() */


/******************************************************************************
**
**      Function:       btk_event_fini()
**
**      Purpose:        Releases any resources allocated by btk_event_init().
**                      Gets rid of objec, flushing any threads waiting,
**                      and releases resources used by the object.
**
**      Args:
**              event_p         Pointer to bt_event_t to be intialized
**          
**
**      Returns:        Void
**
**      Notes:
**      Must be called exactly once, after the last btk_event_alloc() call, when
**      the driver is unloaded. Do not call it once per unit.
**
**      The bt_event_t is an opaque type to be defined for each operating
**      system.
**
**      All of the event routines need to accept NULL for the unit pointer.
**      This will most likey only affect tracing.
**
******************************************************************************/
void btk_event_fini(
    bt_event_t  *event_p
    )
{
#if   defined(__vxworks)
    SEM_ID      dead_event;
    STATUS      vx_ret;
#endif /* vxworks */


    FUNCTION("btk_event_fini");
    LOG_UNKNOWN_UNIT;
    FENTRY;


#if     defined(__sun)

    /*
    ** The documentation doesn't say what happens if I destroy a mutex or
    ** semaphore that is currently in use. Hopefully, it takes care of it
    ** by giving an error at the cv_wait() call.
    */

    cv_destroy(&event_p->condvar);
    mutex_destroy(&event_p->mutex);

#elif    defined(__sgi)

    TRC_STR(BT_TRC_KLIB, "Destroy the sleep lock.\n");

    freesema(event_p);

#elif   defined(__vxworks)

    dead_event = *event_p;
    *event_p = NULL;
        
    vx_ret = semDelete(dead_event);
    if (OK != vx_ret) {
        INFO_STR("Semaphore ID is invalid.");
    }
    

#elif   defined(_NTDDK_)

    ;

#elif   defined(BT_uCOS)

    /* no action required */
    /* Once an uCOS semaphore is created it can not be deleted */
    ;

#elif   defined(__linux__)

    /* No action required */
    ;

#elif defined (__lynxos)

    sreset(event_p);

#else

#error  Code not written yet

#endif  /* defined(__sun, __vxworks, _NTDDK_) */

    FEXIT(0);

    return;
} /* end  btk_event_fini() */



/******************************************************************************
**
**  Function:   btk_event_set()
**
**  Purpose:    Fills (sets) an event, causing exactly one of the thread
**              which may be waiting to wake up.
** 
**              Forces the object set (full). If there are any threads 
**              waiting on the object, this will cause EXACTLY ONE
**              of those sleeping threads to wake.
**
**  Args:
**      event_p         Pointer to bt_event_t to be set.
**
**  Returns:    Void
**
**  Notes:
**      The bt_event_t is an opaque type to be defined for each operating
**      system.
**
**      All of the event routines need to accept NULL for the unit pointer.
**      This will most likey only affect tracing.
**
**  NT Notes:
**      Callers can be running at IRQL <= DISPATCH_LEVEL.
**
******************************************************************************/
void btk_event_set(
    bt_event_t  *event_p
    )
{
    FUNCTION("btk_event_set");
    LOG_UNKNOWN_UNIT;
    FENTRY;

#if     defined(__sun)
    {
        bool_t  old_state;

        mutex_enter(&event_p->mutex);
        old_state = event_p->binary_sema;
        event_p->binary_sema = TRUE;

        /*
        ** This can cause a signal even when no one is waiting. I don't
        ** see the 'extra' signal as being a problem, since the wait
        ** routine has to check that there is really something available
        ** anyways.
        */
        if (!old_state) {
            cv_signal(&event_p->condvar);
        }
        mutex_exit(&event_p->mutex);
        TRC_MSG(BT_TRC_KLIB, (LOG_FMT "Set Event: Sent signal = %s\n",
                LOG_ARG, (!old_state) ? "True" : "False"));
    }

#elif   defined(__sgi)
    
    TRC_STR(BT_TRC_KLIB, "Release the sleep lock.\n");

    vsema(event_p);

#elif   defined(__vxworks)

    {
        STATUS  vx_ret;

        vx_ret = semGive(*event_p);
        if (OK != vx_ret) {
            INFO_STR("Semaphore ID is invalid.");
        }

    }

#elif   defined(_NTDDK_)

    KeSetEvent(event_p, EVENT_INCREMENT, FALSE);

#elif   defined(BT_uCOS)

    {
        UBYTE err;

        err = OSSemPost(*event_p);

        if (OS_NO_ERR != err) {
            TRC_STR(BT_TRC_KLIB, "Semaphore count overflow.\n");
        }
    }

#elif   defined(__linux__)
    {
        up(&event_p->sem);
    }

#elif defined (__lynxos)

    /* Wake up one process waiting for this event */
    ssignal(event_p);

#else

#error  Code not written yet

#endif  /* __sun, __vxworks, _NTDDK_ */

    FEXIT(0);

    return;
} /* end  btk_event_set() */



/******************************************************************************
**
**  Function:   btk_event_wait()
**
**  Purpose:    Waits for specified timeout period or the event to
**              become set (FULL).
**              
**              Waits for event becomes availabe. Goes to sleep if empty.
**              The object is empty when this routine returns with
**              SUCCESS.
**
**  Args:
**      event_p         Pointer to bt_event_t to wait on.
**      timeout         How long to wait before we time out.
**
**  Returns:
**      BT_SUCCESS      Got the event, not a problem.
**      BT_EABORT       User action caused us to abort
**      BT_EBUSY        Timed out before object was set
**
**      Note:
**          Only the return values listed above are valid.
**
**      The bt_event_t is an opaque type to be defined for each operating
**      system.
**
**      All of the event routines need to accept NULL for the unit pointer.
**      This will most likey only affect tracing.
**
**
**  NT Notes:
**      Callers must be running at IRQL == PASSIVE_LEVEL and in the context
**      of a User thread.
**
******************************************************************************/
bt_error_t btk_event_wait(
    bt_event_t  *event_p,
    bt_tck_t    timeout
    )
{
    bt_error_t  retvalue;               /* Value to return from routine */

    FUNCTION("btk_event_wait");
    LOG_UNKNOWN_UNIT;
    FENTRY;

#if     defined (__sun)
    {
        bt_tck_t        abs_tick;       /* Timeout in clock ticks */
        bt_tck_t        tick;           /* Current time */
        int             ret;            /* Return value from operating system */

        /* 
        ** Calculate in absolute ticks when to timeout 
        */
        tick = btk_tck_get();
        abs_tick = tick + timeout;

        /*
        ** Actually do the waiting 
        */
        mutex_enter(&event_p->mutex);
        retvalue = BT_SUCCESS;
        if (!event_p->binary_sema) {
            if (BT_NO_WAIT == timeout) {
                /* Timed out, this isn't necessarily an error! */
                TRC_STR(BT_TRC_KLIB, "Event empty and no waiting requested.\n");
                retvalue = BT_EBUSY;
            } else {
                while (!event_p->binary_sema) {
                    if (BT_FOREVER == timeout) {
                        ret = cv_wait_sig(&event_p->condvar, &event_p->mutex);
                    } else {
                        /*
                        ** Should go back to cv_timedwait_sig() call but HW has a bug
                        ** where if we try to stop a DMA in progress i.e. CTRL-C was
                        ** pressed HW hangs, thus we don'g allow signals during waits
                        ** that specify a timeout like DMA
                        */
                        ret = cv_timedwait(&event_p->condvar, &event_p->mutex, abs_tick);
                    }
                    if (ret > 0) {
                        TRC_MSG(BT_TRC_KLIB, (LOG_FMT "event_p->binary_sema = %s\n",
                            LOG_ARG, (event_p->binary_sema) ? "Full" : "Empty"));
                        /* Condition value changed. This is good */
                        continue;
                    } else if (0 == ret) {
                        /* Interrupted by a signal */
                        INFO_STR("Aborted by signal.\n");
                        retvalue = BT_EABORT;
                        break;
                    } else {
                        /* Timed out */
                        INFO_STR("Timeout.\n");
                        retvalue = BT_EBUSY;
                        break;
                    }
                } /* end while */
            }
        }
        if (BT_SUCCESS == retvalue) {
            event_p->binary_sema = FALSE;
            TRC_STR(BT_TRC_KLIB, "Got event.\n");
        }

        mutex_exit(&event_p->mutex);
    }

#elif   defined(__sgi)

    TRC_STR(BT_TRC_KLIB, "Try to get the sleep lock.\n");

    /*
    ** Allow signals to interrupt and wait for the vsema call
    */
    if (psema(event_p, PZERO+1) == -1) {
        retvalue = BT_EABORT;
    } else {
        retvalue = BT_SUCCESS;
    }

#elif   defined(__vxworks)

    {
        STATUS  vx_ret;

        /*
        ** Timeout parameter is already in VxWorks native format, including
        ** settings for not waiting and waiting forever.
        */
        vx_ret = semTake(*event_p, timeout); 

        if (OK != vx_ret) {
            INFO_STR("Semaphore ID is invalid or task timed out.");
            retvalue = BT_EBUSY;
        } else {
            retvalue = BT_SUCCESS;
        }
    }

#elif   defined(_NTDDK_)

    {

    LARGE_INTEGER loc_timeout;
    NTSTATUS nt_status;

    loc_timeout.QuadPart = -timeout;
    nt_status = KeWaitForSingleObject(event_p, UserRequest, UserMode, FALSE,
        &loc_timeout);
    TRC_MSG(BT_TRC_WARN,
        (LOG_FMT "KeWait status = 0x%x, timeout = %d\n",
	 LOG_ARG, nt_status, timeout));

#if 0
    BTK_ASSERT(STATUS_SUCCESS == nt_status || STATUS_TIMEOUT == nt_status);
#endif
    if(STATUS_SUCCESS == nt_status) {
        retvalue = BT_SUCCESS;
    } else {
        retvalue = BT_EBUSY;
        INFO_STR("Timeout\n");
    }

    }

#elif defined(BT_uCOS)

    {
        UBYTE err;

        OSSemPend(*event_p, (UWORD) timeout, &err);

        if (OS_TIMEOUT == err) {
            TRC_STR(BT_TRC_KLIB, "Semaphore timeout occured.\n");
            retvalue = BT_EBUSY;
        } else if (OS_NO_ERR == err) {
            retvalue = BT_SUCCESS;
        } else {
            TRC_STR(BT_TRC_KLIB, "Semaphore error occured.\n");
            retvalue = BT_EABORT;
        }
    }

#elif defined(__linux__)
    {
	int     down_ret;
	retvalue = BT_SUCCESS;	/* Assume everything will work */

	if (BT_NO_WAIT == timeout) {

	    /* Don't wait for the lock */
	    down_ret = down_trylock(&event_p->sem);
#if	defined(DEBUG)
	    TRC_MSG(BT_TRC_KLIB, (LOG_FMT "down_ret = %d.\n", 
	        LOG_ARG, down_ret));
#endif	/* defined(DEBUG) */
	    if (down_ret) {
		retvalue = BT_EBUSY;
	    }
	} else if (BT_FOREVER == timeout) {

	    /* Will wait as long as it takes for the lock */
	    down_ret = down_interruptible(&event_p->sem);
#if	defined(DEBUG)
	    TRC_MSG(BT_TRC_KLIB, (LOG_FMT "down_ret = %d.\n", 
	        LOG_ARG, down_ret));
#endif	/* defined(DEBUG) */
	    if (down_ret) {
		retvalue = BT_EABORT;
	    }
	} else {

	    /* Need to timeout if we don't get the lock */

	    /* Create a timeout timer */

	  event_p->timed_out = FALSE;         /* Not yet timed out */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)	  
	    init_timer(&event_p->timer);
	    event_p->timer.expires  = jiffies + timeout;
	    event_p->timer.data = (unsigned long) event_p;
	    event_p->timer.function = timeout_event;


	    add_timer(&event_p->timer);

	    down_ret = down_interruptible(&event_p->sem);
	    del_timer(&event_p->timer);
#else
	    event_p->timer.timer.expires =  jiffies + timeout;
	    event_p->timer.data          = event_p;
	    event_p->timer.function      = timeout_event;

	    timer_setup(&(event_p->timer.timer), bt_event_timeout_wrapper, 0);

	    down_ret = down_interruptible(&event_p->sem);
	    del_timer(&event_p->timer.timer);
	    
#endif

#if	defined(DEBUG)
	    TRC_MSG(BT_TRC_KLIB, (LOG_FMT "down_ret = %d, timed_out = %d.\n", 
	        LOG_ARG, down_ret, event_p->timed_out));
#endif	/* defined(DEBUG) */
	    if (event_p->timed_out) {
		retvalue = BT_EBUSY;
	    } else if (down_ret) {
		retvalue = BT_EABORT;
	    }
	}
    }

#elif defined (__lynxos)

	retvalue = BT_SUCCESS;	/* Assume everything will work */
    /* For timeout:
     *   BT_NO_WAIT is okay for a value, tswait will return right away if 
     *   the semaphore is already locked.
     *   BT_FOREVER:
     */
    if (timeout) {
        retvalue = tswait(event_p, SEM_SIGIGNORE, timeout);
        switch (retvalue) {

        /* could not register a timeout */
            case TSWAIT_NOTOUTS:
            /* We cannot return anything else because of the assert later
             * on in this routine.
             */
            return BT_EABORT;

        /* interrupted by a signal */
            case TSWAIT_ABORTED:
            deliversigs();
            return BT_EABORT;

        /* timed out */
            case TSWAIT_TIMEDOUT:
            return BT_EBUSY;

        /* regular wakeup, fall through */
            default:
                retvalue = BT_SUCCESS;
	}
    }

#else

#error  Code not written yet

#endif  /* __sun, __vxworks, _NTDDK_ */

    BTK_ASSERT( (BT_SUCCESS == retvalue) || 
                (BT_EBUSY == retvalue) ||
                (BT_EABORT == retvalue) );

    FEXIT(retvalue);
    return retvalue;
} /* end  btk_event_wait() */

