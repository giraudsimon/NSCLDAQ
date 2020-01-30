/******************************************************************************
**
**      Filename:       bt_open.c
**
**      Purpose:        IRIX implementation of mmap for NanoBus hardware
**
**      Functions:      bt_open(), bt_close()
**
**      $Revision: 1.9 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000, 2005 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.9 $" __DATE__;
#endif /* LINT */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__sun)
#include <sys/time_impl.h>
#endif /* defined(__sun) */

#if defined (BT951)
#include <ipc.h>
#include <sem.h>
#else
#include <sys/ipc.h>
#include <sys/sem.h>
#endif /* BT951 */

#if !defined(__linux__)
#include <semaphore.h>
#endif /* __linux__ */

#include "btapi.h"
#include "btpiflib.h"

#if !defined(BT965)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) && !defined(BT965)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif /* (__GNU_LIBRARY__) && !(_SEM_SEMUN_UNDEFINED) && !(BT965) */
#endif /* BT965 */

int semaphore_create( key_t key );
int semaphore_delete( int sem_id );
int semaphore_lock( int sem_id );
int semaphore_unlock( int sem_id );

/*****************************************************************************
**
** Global data
**
*****************************************************************************/
pthread_mutex_t bt_lib_mutex_g = PTHREAD_MUTEX_INITIALIZER;

key_t bt_sema_key_gp = (key_t) 1567;
int bt_sema_id_gp;

bt_lib_data_t   *bt_lib_data_gp = NULL;
bt_icbr_list_t  *bt_icbr_head_gp = NULL;
int             init_failed = 0;

/*****************************************************************************
**
** Local Macros
**
*****************************************************************************/

/*****************************************************************************
**
** Static functions
**
*****************************************************************************/

static void lock_all(void);
static void unlock_all(void);

/*****************************************************************************
**
**      Name:           lib_init
**
**      Purpose:        Do any initialization that needs to be done
**                      for the library exactly once.
**
**      Args:
**          void
**
**      Modifies:
**          
**      Returns:
**          void
**
*****************************************************************************/

static void lib_init(
    void)
{
    int os_ret;

    /*
    ** Open the global semaphore
    */
    bt_sema_id_gp = semaphore_create(bt_sema_key_gp);
    if (bt_sema_id_gp == -1) {
        perror("Creation of named semaphore failed");
        init_failed = errno;
    }

    os_ret = pthread_atfork(lock_all, unlock_all, unlock_all);
    if (0 != os_ret) {
        DBG_MSG((stderr, __FILE__ "pthread_atfork failed %d: %s.\n", os_ret,
            strerror(os_ret)));
        init_failed = os_ret;
    }

    return;
}


/*****************************************************************************
**
**      Name:           lock_all
**
**      Purpose:        Grabs all the mutexs currently used by the library.
**
**      Args:
**          void
**
**      Returns:
**          void
**
*****************************************************************************/

static void lock_all(
    void
    )
{
    bt_lib_data_t       *curr_p = NULL;

    pthread_mutex_lock(&bt_lib_mutex_g);

    curr_p = bt_lib_data_gp;

    while (NULL != curr_p) {
        pthread_mutex_lock(&curr_p->mutex);
        curr_p = curr_p->next_p;
    }

    return;
}
/*****************************************************************************
**
**      Name:           unlock_all
**
**      Purpose:        Releases all the mutexs currently used by the library.
**
**      Args:
**          void
**
**      Returns:
**          void
**
*****************************************************************************/

static void unlock_all(
    void
    )
{
    bt_lib_data_t       *curr_p = NULL;

    curr_p = bt_lib_data_gp;

    while (NULL != curr_p) {
        pthread_mutex_unlock(&curr_p->mutex);
        curr_p = curr_p->next_p;
    }

    pthread_mutex_unlock(&bt_lib_mutex_g);
    return;
}

/*****************************************************************************
**
**              External functions
**
*****************************************************************************/

/*****************************************************************************
**
**      Name:           bt_open
**
**      Purpose:        Opens a device for access.
**
**      Args:
**          btd_p       Pointer to Bit 3 descriptor
**          devname_p   String containing system-specific device name.
**                      Device name created using bt_gen_name() routine.
**          flags       Platform specific flags for how to open device.
**                      Zero will be accepted as BT_RD | BT_WR, otherwise
**                      you have to specify the flags.
**
**      Modifies:
**          bt_desc     Initializes everything.
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_open(
    bt_desc_t   *btd_p,         /* Pointer to Bit 3 descriptor */
    const char  *devname_p,     /* Device name, as created by bt_gen_name() */
    bt_accessflag_t     flags   /* Flags for read, write, etc */
    )
{
    bt_error_t  retvalue = BT_SUCCESS;
    bt_desc_t   btd = NULL;     /* Unit descriptor */
    int         rdwr_flags;
    int         os_ret;         /* errno return values from operating system */
    static pthread_once_t       bt_once_control = PTHREAD_ONCE_INIT;

    /*
    ** Make sure device name is a valid pointer
    */
    if ((NULL == devname_p)) {
        DBG_STR("Invalid device name");
        retvalue = BT_EFAIL;
        goto bt_open_end;
    }

    /* 
    ** Set up threading support 
    */
    os_ret = pthread_once(&bt_once_control, lib_init);
    if ((0 != os_ret) || (0 != init_failed)) {
        DBG_MSG((stderr, "%s failed %d.\n", 
                os_ret ? "pthread_once" : "pthread_atfork", os_ret));
        switch (os_ret) {
          case BT_ENOMEM:
          case BT_EINVAL:
            retvalue = os_ret;  /* Used same numbers as POSIX */
            break;

          default:
            retvalue = BT_EFAIL;
            break;
        }
        goto bt_open_end;
    }

    /* 
    ** Convert from our flags to normal open()/close() flags 
    */
    if ((0 == flags) || 
        ((flags & BT_RD) && (flags & BT_WR))) {
        rdwr_flags = O_RDWR;
    } else if (flags & BT_RD) {
        rdwr_flags = O_RDONLY;
    } else if (flags & BT_WR) {
        rdwr_flags = O_WRONLY;
    } else {
        DBG_STR("Invalid read/write flags.");
        retvalue = BT_EINVAL;
        goto bt_open_end;
    }

    /*
    ** Allocate an SBS descriptor
    */
    btd = malloc(sizeof(bt_lib_data_t));
    if (NULL == btd) {
        DBG_STR("Can't allocate memory for descriptor.");
        retvalue = BT_ENOMEM;
        goto bt_open_end;
    }
    memset(btd, 0, sizeof(bt_lib_data_t));
    btd->access_flags = flags;
    btd->icbr_head_p = NULL;
    btd->icbr_running = FALSE;
    DBG_MSG((stderr, "Device to open: %s\n", devname_p));

    /*
    ** Actually open the device
    */
    if (-1 == (btd->fd = open((char *) devname_p, rdwr_flags, 0))) { 
        DBG_MSG((stderr, "Call to open failed, devname_p = %s\n", devname_p));
        switch (errno) {
          case BT_EACCESS:
          case BT_EIO:
          case BT_EINVAL:
            retvalue = errno;   /* Our number is same as POSIX error number */
            break;

          case ENOENT:
          case ENXIO:
            /* retvalue = BT_ENODEV; */
            retvalue = BT_EFAIL;
            break;

          case ENAMETOOLONG:
            retvalue = BT_EINVAL;
            break;

          default:
            /* retvalue = BT_ESYSTEM;  */
            retvalue = BT_ENOSUP;
            break;
        }
        goto bt_open_end;
    }

    /* 
    ** Initialize mutex private to this descriptor 
    */
    os_ret = pthread_mutex_init(&btd->mutex, NULL);
    assert(0 == os_ret);

    /* 
    ** Insert this descriptor in the list of open descriptors 
    */
    pthread_mutex_lock(&bt_lib_mutex_g);
    btd->next_p = bt_lib_data_gp;
    bt_lib_data_gp = btd;
    pthread_mutex_unlock(&bt_lib_mutex_g);

bt_open_end:
    if ((BT_SUCCESS != retvalue) && (NULL != btd)){
        free(btd);
        btd = NULL;
    }
    *btd_p = btd;
    return retvalue;
}


/*****************************************************************************
**
**      Name:           bt_close
**
**      Purpose:        Shuts down a logical device.
**
**      Args:
**          btd         Unit descriptor
**
**      Modifies:
**          btd         Decrements reference count, closes down unit if
**          btd->unit_p no one is using it anymore.
**          
**      Returns:
**          0           Completed successfully
**          Otherwise   Error return
**
*****************************************************************************/

bt_error_t bt_close(
    bt_desc_t   btd             /* Bit 3 descriptor */
    )
{
    bt_error_t          retvalue = BT_SUCCESS;
    bt_icbr_list_t      *curr_p, *next_p;
    bt_thread_wait_t    thread_wake_data;

    if (BT_DESC_BAD(btd)) {
        DBG_STR("Invalid descriptor.");
        retvalue = BT_EDESC;
        goto close_end;
    }

    /* 
    ** Cancel the ICBR thread and wait for it to exit
    ** before destroying the descriptor that it was using
    */
    pthread_mutex_lock(&btd->mutex);
    if (btd->icbr_running == TRUE) {
        pthread_cancel(btd->icbr_thread);
        thread_wake_data.thread_id = btd->thread_id;
        (void) bt_ctrl(btd, BIOC_THREAD_WAKE, &thread_wake_data);
        (void) pthread_cond_wait(&btd->icbr_started, &btd->mutex);
        
    }
    
    /* 
    ** Unregister for any ICBRs currently defined 
    */
    while ((curr_p = btd->icbr_head_p) != NULL) {
        next_p = curr_p->next_p;
        free(curr_p);
        btd->icbr_head_p = next_p;
    }
    pthread_mutex_unlock(&btd->mutex);

    /* 
    ** Remove this descriptor from global list 
    */
    pthread_mutex_lock(&bt_lib_mutex_g);
    {
        bt_lib_data_t   *current_p, *previous_p;

        /* 
        ** Removal of 1st element is handled by next_p always being
        ** first structure field
        */
        previous_p = (bt_lib_data_t *) &bt_lib_data_gp;
        current_p = bt_lib_data_gp;
        while ((NULL != current_p) && (btd != current_p)) {
            previous_p = current_p;
            current_p = current_p->next_p;
        }
        if (NULL != current_p) {
            previous_p->next_p = current_p->next_p;
        } else {
            DBG_MSG((stderr, "Couldn't find descriptor %p in list.\n", btd));
        }
    }
    pthread_mutex_unlock(&bt_lib_mutex_g);

    /*
    ** Destroy the local mutex 
    */
    pthread_cond_destroy(&btd->icbr_started);
    pthread_mutex_destroy(&btd->mutex);

    /*
    ** Must not use any mutexs from this point on! 
    */
    if (-1 == close(btd->fd)){
        switch (errno) {
          case EBADF:
            retvalue = BT_EDESC;
            break;
          default:
            /* retvalue = BT_ESYSTEM;  */
            retvalue = BT_ENOSUP;
            break;
        }
    }
    free(btd);

close_end:
    return retvalue;
}
/*
*************************************************************************
*
*	semaphore_create()
*
*	Description:	This procedure creates a semaphore for the given
*					key_t value.  It returns the semaphore ID.
*
*	Return value:	upon success, positive, non-zero, value for sem_id
*					upon error -1
*	
*************************************************************************
*/
 int semaphore_create( key_t key )
 {
        int sema_id;
        union semun sem_union;
	__attribute__((unused)) struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = 1;	/* V */
	sem_b.sem_flg = 0;
        sem_union.val = 1;

        /*
        ** Exclusively create the semaphore
        */
        if ((sema_id = semget( (key_t) key, 1, 0666 | IPC_CREAT | IPC_EXCL)) != -1) {
            if (semctl(sema_id, 0, SETVAL, sem_union) != -1) {
                return(sema_id);
            } else {
                perror("Initialize semaphore failed");
                (void) semaphore_delete(sema_id);
                return(-1);
            }
        /*
        ** Someone else created it, so just open it
        **
        ** Note, race condition here between someone creating it and trying to 
        ** init it to 1 and someone opening it and then quickly getting it in 
        ** read/write
        **
        ** Race condition only exists at first create because semaphore stays
        ** created until system is rebooted.
        */
        } else {
            return(semget((key_t) key, 1, 0666 | IPC_CREAT));
        }
 }

 /*
 *************************************************************************
 *
 *	semaphore_delete()
 *
 *	Description:	This procedure deletes the semaphore with the given
 *					sem_id value.
 *
 *	Return value:	upon success, positive, non-zero, value for sem_id
 *					upon error, -1
 *	
 *************************************************************************
 */
int semaphore_delete( int sem_id )
{
	union semun sem_union;

        sem_union.val = 0;
	return( semctl( sem_id, 0, IPC_RMID, sem_union ) );
}
/*
*************************************************************************
*
*	semaphore_lock()
*
*	Description:	This procedure requests ownership of the semaphore
*					with the specified sem_id value.
*
*	Return value:	0 Semaphore ownership granted
*				   -1 Semaphore error
*					
*************************************************************************
*/
int semaphore_lock( int sem_id )
{
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;	/* P */
	sem_b.sem_flg = SEM_UNDO;

	if( semop( sem_id, &sem_b, 1) == -1) {
		fprintf( stderr, "semaphore lock failed\n" );
		return( -1 );
	}
	return( 0 );
}
		

/*
*************************************************************************
*
*	semaphore_unlock()
*
*	Description:	This procedure relinquishes ownership of the semaphore
*					with the specified sem_id value.
*
*	Return value:	0 Semaphore ownership relinquished
*				   -1 semaphore error
*					
*************************************************************************
*/
int semaphore_unlock( int sem_id )
{
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op =  1;
	sem_b.sem_flg = SEM_UNDO;

	if( semop( sem_id, &sem_b, 1) == -1) {
		fprintf( stderr, "semaphore v failed\n" );
		return( -1 );
	}

	return( 0 );
}
