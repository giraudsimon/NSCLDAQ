/******************************************************************************
**
**      Filename:       btp_cfg.c
**
**      Purpose:        Linux configuration and module loading/unloading for
**                      NanoBus/PCI adapters.
**
**      Functions:      init_module(), cleanup_module()
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2006 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $";
#endif  /* LINT */
static const char driver_version[] = "$Name: 1003v3p1pC 1003v3p1pB $";

/* Used by btpdd.h to know if it should define __NO_VERSION__ or not */

#define BTP_CFG_C


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <asm/irq.h>
#include <asm/io.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
#include <linux/devfs_fs_kernel.h>
#endif
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

#include "btdd.h"

#include <linux/interrupt.h>

#if     0 && !defined(NDEBUG)

#define DEBUG_INIT_SEQUENCE 1   /* For when you need the init_fail() routine */

#elif   !defined(DEBUG_INIT_SEQUENCE)

#define DEBUG_INIT_SEQUENCE 0   /* Don't put init_fail() in code */

#endif  /* 0 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0))
#define DMA_32BIT_MASK (DMA_BIT_MASK(32))
#endif

static int btp_suspend(struct pci_dev *dev, pm_message_t state);

static int btp_resume(struct pci_dev *dev);

#ifndef __devinit
#define __devinit		/* 3.8 kernel */
#endif

#ifndef __devexit
#define __devexit		/* 3.8 kernel. */
#endif

#ifndef __devinitdata
#define __devinitdata
#endif

#ifndef __devexit_p
#define __devexit_p(p) p
#endif


static int __devinit btp_init_one(struct pci_dev *curr_dev_p, const struct pci_device_id *id);

static void __devexit btp_remove_one(struct pci_dev *dev_p);

static struct pci_device_id btp_ids[] __devinitdata = {
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_617) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_614) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_616) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_RPQ890) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_618) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_619) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_704) },
    { PCI_DEVICE(BT_PCI_VENDOR_BIT3, BT_PCI_DEVICE_RPQ601332) },
    { 0, },
};

MODULE_DEVICE_TABLE(pci, btp_ids);

static struct pci_driver btp_pci_driver = {
    .name       = "btp",
    .id_table   = btp_ids,
    .probe      = btp_init_one,
    .remove     = __devexit_p(btp_remove_one),
    .suspend    = btp_suspend,
    .resume     = btp_resume,
};


/******************************************************************************
**
**      Global symbols
**
******************************************************************************/

bt_data32_t bt_trace_lvl_g = BT_TRC_DEFAULT;

EXPORT_SYMBOL(bt_trace_lvl_g);

const char *bt_name_gp = BT_DRV_NAME;

EXPORT_SYMBOL(bt_name_gp);

unsigned int btp_major_g = 0;   /* Our major device number */

volatile bt_data32_t *bt_trace_mreg_gp32;

volatile bt_data64_t *bt_trace_mreg_gp64;

volatile bt_data32_t bt_trace_mreg_size;

#if     defined(DEBUG)
EXPORT_SYMBOL(bt_unit_array_gp);
/* EXPORT_SYMBOL(bt_num_units_g); */
#endif  /* DEBUG */

bt_unit_t *bt_unit_array_gp[BT_MAX_UNITS+1] = {
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,

    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
};
static struct semaphore bt_units_lock_g;

/* default File operations structure */

struct file_operations btp_fops = {

#if     defined(__GCC__)
   .open=       btp_open,
   .release=    btp_close,
   .llseek=     btp_llseek,
   .read=       btp_read,
   .write=      btp_write,
   .ioctl=      btp_ioctl,
   .mmap=       btp_mmap
#else   /* __GCC__ */

   llseek:	btp_llseek,
   read:	btp_read,
   write:	btp_write,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
   ioctl:	btp_ioctl,
#else
   unlocked_ioctl: btp_ioctl,
   compat_ioctl:   btp_ioctl,
#endif
   mmap:	btp_mmap,
   open:	btp_open,
   release:	btp_close,

#endif  /* __GCC__ */
};

/******************************************************************************
**
**      Module definitions and Driver loading parameters
**  
**      The User is allowed to pass command line arguments into this
**      driver via the btp.conf file.   The btp.conf file is filled 
**      with variables at run time by the insmod command in /sys/Makefile.
**      The global variables that the user is allowed to modifed are 
**      defined with the module_param() and MODULE_PARM_DESC() macros.
**
******************************************************************************/

unsigned int bt_major = 0;      /* Default major device number */

module_param(bt_major, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(bt_major, "Default major device number, 0 == auto configure it.");

unsigned long trace = 0;        /* Device driver trace level */

module_param(trace, long, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

MODULE_PARM_DESC(trace, "Bit mask of module tracing flags.");

/* always give a MODULE variable an initaial default value */
unsigned int icbr_q_size[BT_MAX_UNITS+1] = {
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE,
    DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE, DEFAULT_Q_SIZE
};


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
/* number of parameters for icbr_q_size */
MODULE_PARM(icbr_q_size, "0-" __MODULE_STRING(BT_MAX_UNIT) "i");
#else
int icbr_q_size_cnt = 0;
module_param_array(icbr_q_size, int, &icbr_q_size_cnt, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)) */

MODULE_PARM_DESC(icbr_q_size, "Number of entries to create in the interrupt callback routine (ICBR) queue.");

/* always give a MODULE variable an initaial default value */
unsigned long lm_size[BT_MAX_UNITS+1] = {
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE,
    DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE, DEFAULT_LMEM_SIZE
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
/* number of parameters for icbr_q_size */
MODULE_PARM(lm_size, "0-" __MODULE_STRING(BT_MAX_UNIT) "l");
#else
/* number of parameters for lm_size */
int lm_size_cnt = 0;
module_param_array(lm_size, long, &lm_size_cnt, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)) */


MODULE_PARM_DESC(lm_size, "Per unit array given the size of the local memory device. Default " __MODULE_STRING(DEFAULT_LMEM_SIZE) ".");

MODULE_AUTHOR("SBS Technologies, Inc.");

MODULE_DESCRIPTION("Device driver for SBS Technologies Connectivity Products PCI-VMEbus adapters.");

MODULE_SUPPORTED_DEVICE(BT_DRV_NAME);

MODULE_LICENSE("Proprietary");

#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/vermagic.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

/*
** Local function prototypes
*/
static int create_unit(bt_unit_t *unit_p);
static int destroy_unit(bt_unit_t *unit_p);

/*
** External function prototypes
*/
extern unsigned long bt_kvm2bus(void * vm_addr_p);
extern bt_error_t btk_irq_qs_init(bt_unit_t *unit_p, size_t q_size);
extern void btk_irq_qs_fini(bt_unit_t *unit_p, size_t q_size);
#if LINUX_VERSION_CODE < LINUX_VERSION(4,9,0)
extern irqreturn_t btk_isr(int irq, void *vunit_p, struct pt_regs *regs);
#else
extern irqreturn_t btk_isr(int irq, void* vunit_p);
#endif
extern void btk_setup(bt_unit_t *unit_p);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_CFG_C);


/*****************************************************************************
**
**      Name:           btp_init
**
**      Purpose:        Registers the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          Otherwise   Error number
**
**      Notes:
**
*****************************************************************************/
int __init btp_init(void)
{
    int         ret_val;

    FUNCTION("btp_init");
    LOG_UNKNOWN_UNIT;
    
#if !defined(CONFIG_PCI)
    #error Must compile on a system with PCI support.
    return -ENODEV;
#endif

    /*
    ** Check page size for kernel
    */
    if (BT_PAGE_SIZE  > PAGE_SIZE) {
        printk("Kernel page size does not match hardware page size\n");
        WARN_STR("Kernel page size does not match hardware page size\n");
        return -ENODEV;
    }
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26)
    SET_MODUE_OWNER(&btp_fops);
#else
    btp_fops.owner = THIS_MODULE;
#endif

    /*
    ** Initialize unit number spinlock
    */
    sema_init(&bt_units_lock_g, 1);
    
    /* 
    ** Initialize trace value & memory handling routines 
    */
    if (0 != trace) {
        bt_trace_lvl_g = trace;
    }
    bt_trace_mreg_size = 32;
    bt_trace_mreg_gp32 = (volatile bt_data32_t *) NULL;
    bt_trace_mreg_gp64 = (volatile bt_data64_t *) NULL;
    btk_mem_init();

    /*
    ** Register the driver to discover all the installed cards
    */
    if (pci_register_driver(&btp_pci_driver) < 0) {
        TRC_STR(BT_TRC_CFG, "No boards found in system\n")
        pci_unregister_driver (&btp_pci_driver);
        btk_mem_fini();
        FEXIT(-ENODEV);
        return(-ENODEV);
    }

    /* 
    ** Get the major device number for our card 
    */
    if ((ret_val = register_chrdev(bt_major, bt_name_gp, &btp_fops)) < 0) {
        WARN_MSG((LOG_FMT "Could not register device with major number %d.\n",
            LOG_ARG, bt_major));
        btp_major_g = 0;
        pci_unregister_driver(&btp_pci_driver);
        btk_mem_fini();
        FEXIT(ret_val);
        return(ret_val);
    }
    btp_major_g = ret_val;
    TRC_STR(BT_TRC_CFG, "Driver loaded.\n");

    /*
    ** Create all the device nodes
    */
#ifdef CONFIG_DEVFS_FS
    down(&bt_units_lock_g);
    for (unit = 0; unit < BT_MAX_UNITS; unit++) {
        if (bt_unit_array_gp[unit] != NULL) {
            for (minor = 0; minor < BT_MAX_AXSTYPS; minor++) {
                sprintf(name, "btp%d", minor * (BT_MAX_UNITS + 1));
                ret_val = devfs_mk_cdev(MKDEV(btp_major_g, minor * (BT_MAX_UNITS + 1)), S_IFCHR|S_IRUGO|S_IWUGO, name);
            }
        }
    }
    up(&bt_units_lock_g);
#endif /* CONFIG_DEVFS_FS */

    return(0);
}

/*****************************************************************************
**
**      Name:           btp_exit
**
**      Purpose:        Deregisters the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          Void
**
**      Notes:
**
*****************************************************************************/
void __exit btp_exit(void)
{

    FUNCTION("btp_exit");
    LOG_UNKNOWN_UNIT;


#ifdef CONFIG_DEVFS_FS
    /*
    ** Destroy all the device nodes
    */
    {
    int         unit, minor;
    char        name[256];
    down(&bt_units_lock_g);
    for (unit = 0; unit < BT_MAX_UNITS; unit++) {
        if (bt_unit_array_gp[unit] != NULL) {
            for (minor = 0; minor < BT_MAX_AXSTYPS; minor++) {
                sprintf(name, "btp%d", minor * (BT_MAX_UNITS + 1));
                devfs_remove(name);
            }
        }
    }
    up(&bt_units_lock_g);
    }
#endif /* CONFIG_DEVFS_FS */

    /*
    ** Release major number 
    */
    unregister_chrdev(btp_major_g, bt_name_gp);
    btp_major_g = 0;
    
    /*
    ** Unregister driver 
    */
    pci_unregister_driver(&btp_pci_driver);
    
    /* 
    ** Clean up for our memory handling routines 
    */
    btk_mem_fini();

    TRC_STR(BT_TRC_CFG, "Driver unloaded.\n");
    FEXIT(0);
    return;
}

/*****************************************************************************
**
**      Name:           btp_suspend
**
**      Purpose:        Suspend the card
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          -EBUSY
**
**      Notes:
**          Not implemented since it would consume too many resources to
**          put the device into a power save mode.
**
*****************************************************************************/
static int btp_suspend(
    struct pci_dev *dev, 
    pm_message_t s_state)

{
  int state = s_state.event;
    return -EBUSY;
}

/*****************************************************************************
**
**      Name:           btp_resume
**
**      Purpose:        Restore from power transition
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          SUCCESS	Should never go to sleep so nothing to resume from
**
**      Notes:
**          Not implemented since it would consume too many resources to
**          put the device into a power save mode.
**
*****************************************************************************/
static int btp_resume(
    struct pci_dev *dev)
{
    return 0;
}
module_init(btp_init);
module_exit(btp_exit);

/*****************************************************************************
**
**      Name:           btp_init_one
**
**      Purpose:        Loads and initializes the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          0           Success
**          Otherwise   Error number
**
**      Notes:
**
*****************************************************************************/

static int __devinit btp_init_one(
    struct pci_dev *curr_dev_p,
    const struct pci_device_id *id)
{
    int                 ret_val = 0;
    unsigned int        local_pn;       /* Our assembly part number */
    bt_unit_t           *unit_p;
    bt_data32_t         bt_status = 0;

    FUNCTION("btp_init_one");
    LOG_UNKNOWN_UNIT;

    FENTRY;
    printk("<0>" "btp module loaded.\n");
    
    /*
    ** Determine the local card
    */
    switch (id->device) {
      case BT_PCI_DEVICE_617:
        local_pn = BT_PN_PCI_DMA;
        SET_BIT(bt_status, BT_DMA_LOCAL);
        break;

      case BT_PCI_DEVICE_614:
        local_pn = BT_PN_PCI;
        break;

      case BT_PCI_DEVICE_616:
        local_pn = BT_PN_PCI_NODMA;
        break;

      case BT_PCI_DEVICE_618:
        /* Could be either the 618 or the 628, can't tell the two apart */
        local_pn = BT_PN_PCI_FIBER;
        SET_BIT(bt_status, BT_DMA_LOCAL);
        break;

      case BT_PCI_DEVICE_704:
        local_pn = BT_PN_PCI_FIBER_D64;
        SET_BIT(bt_status, BT_DMA_LOCAL);
        SET_BIT(bt_status, BT_NEXT_GEN);
        break;

#if  EAS_A64_CODE
/* EAS A64 */
      case BT_PCI_DEVICE_RPQ601332:
        local_pn = BT_PN_PCI_DBA64_RPQ;
        SET_BIT(bt_status, BT_DMA_LOCAL);
        SET_BIT(bt_status, BT_NEXT_GEN);
        break;
#endif  /* EAS_A64_CODE */

      default:
        WARN_MSG((LOG_FMT "Unknown card at bus %d, slot %d, function %d\n",
                  LOG_ARG, curr_dev_p->bus->number, PCI_SLOT(curr_dev_p->devfn),
                  PCI_FUNC(curr_dev_p->devfn)));
        ret_val = BT_EIO;
        goto init_module_cleanup;
    }

    /*
    ** Inform the system we are a 32 bit DMA engine and make sure
    ** it is ok with that for both normal and consistent DMAs
    */
    if (pci_set_dma_mask(curr_dev_p, (u64) DMA_32BIT_MASK)) {
        WARN_STR("System not capable of 32 bit DMAs.\n");
        ret_val = BT_EIO;
        goto init_module_cleanup;
    } else if (pci_set_consistent_dma_mask(curr_dev_p, (u64) DMA_32BIT_MASK)) {
        WARN_STR("System not capable of 32 bit consistent DMAs.\n");
        ret_val = BT_EIO;
        goto init_module_cleanup;
    }
    TRC_MSG(BT_TRC_CFG, (LOG_FMT "Found %d at bus %d.\n", 
        LOG_ARG, local_pn, curr_dev_p->bus->number));

    /* 
    ** Got another unit to handle 
    */
    unit_p = btk_mem_alloc(sizeof(bt_unit_t),0);
    if (NULL == unit_p) {
        WARN_MSG((LOG_FMT "Not enough memory to create unit structure for device at bus %d.\n",
                  LOG_ARG, curr_dev_p->bus->number));
        ret_val = BT_ENOMEM;
        goto init_module_cleanup;
    }
    BTK_BZERO(unit_p, sizeof(bt_unit_t));

    /*
    ** Initialize this unit
    */
    unit_p->loc_id = local_pn;
    unit_p->dev_p = curr_dev_p;
    unit_p->bt_status = bt_status;
    if (0 != create_unit(unit_p)) {
        WARN_MSG((LOG_FMT "Could not create unit for device at bus %d.\n",
            LOG_ARG, curr_dev_p->bus->number));
        btk_mem_free(unit_p, sizeof(bt_unit_t));
    }
    TRC_MSG(BT_TRC_CFG,
        (LOG_FMT "PN %d found at bus %d, slot %d - assigned unit %d\n",
         LOG_ARG, local_pn, curr_dev_p->bus->number, 
         PCI_SLOT(curr_dev_p->devfn), unit_p->unit_number));
    FEXIT(0)
    return 0;


init_module_cleanup:
    if (ret_val >= 0) {
        ret_val = -ENXIO;
    }
    TRC_STR(BT_TRC_CFG,"Failed module initialization for unit.\n");
    FEXIT(ret_val);
    return ret_val;
}


/*****************************************************************************
**
**      Name:           btp_remove_one
**
**      Purpose:        Releases resources and unloads the device driver
**
**      Args:
**          void
**
**      Modifies:
**          None
**          
**      Returns:
**          void
**
**      Notes:
**
*****************************************************************************/

static void __devexit btp_remove_one(
	struct pci_dev *dev_p)
{
    bt_unit_t           *unit_p;
    unsigned short      unit;
    int                 ret_val;

    FUNCTION("btp_remove_one");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    /*
    ** Search through list for matching pci_dev
    */
    down(&bt_units_lock_g);
    for (unit = 0; unit <= BT_MAX_UNITS; unit++) {
        unit_p = bt_unit_array_gp[unit];
        if ((unit_p != NULL) && 
	    (dev_p == unit_p->dev_p)) {
            ret_val = destroy_unit(unit_p);
            if (ret_val < 0) {
                WARN_MSG((LOG_FMT "Error %d: Could not release unit %d.\n", 
                    LOG_ARG, -ret_val, (int) unit));
            }
        }
    }
    up(&bt_units_lock_g);

    FEXIT(0);
    return;
}


/******************************************************************************
**
**      Local Functions
**
******************************************************************************/


/******************************************************************************
**
**              Description of initialization sequence
**
******************************************************************************/

/*
** The functions create_unit() and destroy_unit() use a table of functions
** to drive them. The table contains a pointer to an init_xxx() function
** and either a cleanup_xxx() function or a pointer to the cleanup_null()
** function.
**
** Each step of the initialization is given it's own function. The order
** it is placed in the table determines which order the functions are
** called in. 
**
** It is assumed that a function that fails has cleaned up after itself
** as best it could; the associated clean up function is not called, nor 
** are functions later in the list called.
**
** IMPORTANT: If init_xxx() returns an error, cleanup_xxx() is NOT called.
**
** Every function takes the same parameter, the unit pointer, and returns
** an integer. These functions are called from within the create_unit()
** and destroy_unit() functions.
**
** Since Linux returns -errno when indicating errors, we have done the
** same in these functions.
*/

/* init function signature */
typedef int init_fn_t(bt_unit_t *);

/* declare all the functions */
static init_fn_t 
    init_pci_config,    cleanup_pci_config,
    init_defaults,
    init_boot_parms,
    init_unit_array,    cleanup_unit_array,
    init_bit_maps,      cleanup_bit_maps,
    init_cookies,
    init_events,        cleanup_events,
    init_mutexs,        cleanup_mutexs,
    init_lists,         cleanup_lists,
    init_irq_q,         cleanup_irq_q,
    init_isr,           cleanup_isr,
    init_dma,           cleanup_dma,
    init_lm,            cleanup_lm,
    init_ldev,
    init_card,
    init_swapping,
#if     DEBUG_INIT_SEQUENCE > 0
    init_fail,  /* DEBUG only */
#endif  /* DEBUG_INIT_SEQUENCE > 0 */
                        cleanup_null;


#define INIT_FN(func_idx, unit_p) \
    (*(bt_init_func[func_idx][0]))(unit_p)

#define CLEANUP_FN(func_idx, unit_p) \
    (*(bt_init_func[func_idx][1]))(unit_p)

/* 
** The actual array of pointers 
*/
static init_fn_t * bt_init_func[][2] = {
    { init_defaults,          cleanup_null },
    { init_pci_config,        cleanup_pci_config },
    { init_cookies,           cleanup_null },
    { init_unit_array,        cleanup_unit_array },
    { init_boot_parms,        cleanup_null },
    { init_mutexs,            cleanup_mutexs },
    { init_events,            cleanup_events },
    { init_lists,             cleanup_lists },
    { init_irq_q,             cleanup_irq_q },
    { init_bit_maps,          cleanup_bit_maps },
    { init_dma,               cleanup_dma },
    { init_isr,               cleanup_isr },
    { init_ldev,              cleanup_null },
    { init_card,              cleanup_null },
    { init_swapping,          cleanup_null },
    { init_lm,                cleanup_lm },

#if     DEBUG_INIT_SEQUENCE > 0
    /*
    ** Insert this anywhere you believe you may have a problem.
    ** It allows you to test the initialization/cleanup routines by
    ** forcing initialization to fail at the defined spot.
    */
    { init_fail,              cleanup_null}, /* Force initialization to fail */
#endif /* DEBUG_INIT_SEQUENCE > 0 */

    /*
    ** Put in a dummy entry at the end.
    ** This makes it easier to move stuff around, since only the last entry
    ** doesn't have a comma. You can just move any of the other entries,
    ** no editting required.
    */
    { cleanup_null,           cleanup_null }
};

/* 
** autosize the array 
*/
#define NUM_INIT_FUNCS (sizeof(bt_init_func)/sizeof(bt_init_func[0]))

/******************************************************************************
**
**      Name:           create_unit()
**
**      Purpose:        Initializes the given unit structure
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          Everything in the unit structure
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int create_unit(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    int func_idx;       /* Index into initialization/cleanup function table */

    FUNCTION("create_unit");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    /*
    ** Check ioctl structure for misalignment
    */
    BT_CHK_IOCTL_STRUCT;

    /*
    ** We don't assign a unit number until the unit pointer is inserted
    ** in the global array.
    */
    unit_p->unit_number = BT_MOCK_UNIT;

    for (func_idx = 0; func_idx < NUM_INIT_FUNCS; func_idx++) {
        /* Call each initialization function in order */
        ret_val = INIT_FN(func_idx, unit_p);
        if (ret_val < 0) {
            /* Failed to initialize the unit, need to cleanup and return */
            TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
                "Failed on initialization function %d, error code = %d (0x%x).\n",
                LOG_ARG, func_idx, ret_val, ret_val));
            while (--func_idx >= 0) {
                if (0 >= CLEANUP_FN(func_idx, unit_p)) {
                    TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
                    "Failed on cleanup function %d, error code = %d (0x%x).\n",
                        LOG_ARG, func_idx, ret_val, ret_val));
                    /* Can't even finish cleaning up, this is bad. */
                    break;
                }
            }

            /* Don't call any more initialization functions */
            break;
        }
        /* 
        ** This can be a good place to trace what happens in init_* functions. 
        */
    }
    
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Name:           destroy_unit()
**
**      Purpose:        Releases resources allocated in the unit structure
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          Everything in the unit structure
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int destroy_unit(
    bt_unit_t *unit_p
    )
{

    int ret_val = 0;

    int func_idx;       /* Index into initialization/cleanup function table */

    FUNCTION("destroy_unit");
    LOG_UNKNOWN_UNIT;

    FENTRY;

    for (func_idx = NUM_INIT_FUNCS-1; func_idx > -1; func_idx--) {
        /* Call each cleanup function in reverse order */
        ret_val = CLEANUP_FN(func_idx, unit_p);
        if (ret_val < 0) {
            TRC_MSG(BT_TRC_WARN|BT_TRC_CFG, (LOG_FMT
                "Failed on cleanup function %d, error code = %d (0x%x).\n",
                LOG_ARG, func_idx, ret_val, ret_val));
            /* Don't call any more cleanup functions */
            break;
        }
    }
    
    FEXIT(ret_val);
    return ret_val;
}

#if     DEBUG_INIT_SEQUENCE > 0

/******************************************************************************
**
**      Name:           init_fail()
**
**      Purpose:        Causes initialization to fail. Useful for debugging
**                      problems in the initialization or cleanup sequence.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          -ENXIO      Always returns an error
**
*****************************************************************************/

/* ARGSUSED */
static int init_fail (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_fail");
    LOG_UNKNOWN_UNIT;
    int ret_val = -ENXIO;

    FENTRY;

    FEXIT(ret_val);
    return ret_val;
}

#endif  /* DEBUG_INIT_SEQUENCE > 0 */

/******************************************************************************
**
**      Name:           cleanup_null()
**
**      Purpose:        Does nothing, place holder for functions that
**                      don't need a separate cleanup.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**
*****************************************************************************/

/* ARGSUSED */
static int cleanup_null(
    bt_unit_t *unit_p
    )
{
    return 0;
}


/******************************************************************************
**
**      Name:           init_pci_config()
**
**      Purpose:        Reads configuration information from PCI space, maps
**                      the device into memory space.
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int init_pci_config(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    struct pci_dev  *curr_dev_p = unit_p->dev_p;
    unsigned long   phys_addr;
    int             result;
    u8              bd_rev;

    FUNCTION("init_pci_config");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /* 
    ** Get the kernel physical address of each of the windows 
    */
    unit_p->csr_phys_addr = pci_resource_start(curr_dev_p, 1)
        & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->mr_phys_addr = pci_resource_start(curr_dev_p, 2)
        & PCI_BASE_ADDRESS_MEM_MASK;
    unit_p->rr_phys_addr = pci_resource_start(curr_dev_p, 3)
        & PCI_BASE_ADDRESS_MEM_MASK;
    ret_val = pci_enable_device(curr_dev_p);
    if (ret_val < 0) {
        WARN_STR("Could not enable the device.\n");
        goto init_pci_config_end;
    }

    /*
    ** Rather than read it from configuration space, get the
    ** interrupt line from the curr_dev_p structure.
    */
    unit_p->irq = curr_dev_p->irq;

    /*
    ** PCI windows are always allocated on a boundary of their size.
    **
    ** Since our smallest PCI Memory window is 64KBytes, the addresses
    ** will always be aligned properly. The following will catch the
    ** problem should these assumptions ever fail.
    */
#if     PAGE_SIZE > SIZE_64KB
#error  "Need to adjust ioremap call, page size exceeds PCI window size"
#endif  /* PAGE_SIZE < SIZE_64KB */

    /*
    ** Get pointer to CSR space
    */
    phys_addr = unit_p->csr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    unit_p->csr_p = (bt_pci_reg_t *) ioremap(phys_addr, PAGE_SIZE);
    if (NULL == unit_p->csr_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in Node I/O registers.\n");
        goto init_pci_config_end;
    }

    /*
    ** Get pointer to Mapping RAM
    */
    phys_addr = unit_p->mr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
        unit_p->mreg_p = (caddr_t) ioremap(phys_addr, BT_CMR_SPACE_SIZE_NGEN);
    } else {
        unit_p->mreg_p = (caddr_t) ioremap(phys_addr, BT_CMR_SPACE_SIZE);
    }
    if (NULL == unit_p->mreg_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in mapping registers.\n");

        /* Need to unmap Node I/O first */
        iounmap((void *) unit_p->csr_p);
        goto init_pci_config_end;
    }
    bt_trace_mreg_size = 32;
    bt_trace_mreg_gp32 = (volatile bt_data32_t *) (unit_p->mreg_p + 0xfffc);
    bt_trace_mreg_gp64 = (volatile bt_data64_t *) NULL;

    /*
    ** Invalidate the PIO and DMA to PCI map regs to prevent accidentally
    ** messing up Linux memory, which normally leads to a panic
    */
    btk_put_mreg_range(unit_p, 0, BT_MAX_SDMA_BIT, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
    btk_put_mreg_range(unit_p, 0, BT_MAX_SDMA_BIT, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);

    /*
    ** Get pointer to Remote RAM
    */
    phys_addr = unit_p->rr_phys_addr;
    BTK_ASSERT(0 == (phys_addr % PAGE_SIZE));
    unit_p->rmem_p = (caddr_t) ioremap(phys_addr, 32 * SIZE_1MB);
    if (NULL == unit_p->rmem_p) {
        ret_val = -ENOMEM;
        WARN_STR("Could not map in remote memory window.\n");

        /* Need to unmap Node I/O and mapping registers */
        iounmap((void *) unit_p->csr_p);
        iounmap((void *) unit_p->mreg_p);
        goto init_pci_config_end;
    }

    /* 
    ** Make sure the master bit is set
    */
    pci_set_master(curr_dev_p);

    /*
    ** Max out latency timer, our card gets off bus every 512 bytes so 
    ** this is not a big deal to system but will make DMA go much faster.
    */
    result = pci_write_config_byte(curr_dev_p, PCI_LATENCY_TIMER, (u8) 0xff);
    if (PCIBIOS_SUCCESSFUL != result) {
        INFO_STR("FAILED to write latency timer during device init");
    }

    /*
    ** Read the board revision register
    */
    (void) pci_read_config_byte(curr_dev_p, PCI_REVISION_ID, &bd_rev);
    unit_p->board_revision = (bt_data8_t) bd_rev;

init_pci_config_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Name:           cleanup_pci_config()
**
**      Purpose:        Unmaps the device in PCI space
**
**      Args:
**          unit_p      Pointer to unit structure
**
**      Modifies:
**          
**      Returns:
**          0           success
**         <0           -ERRNO
**
**      Notes:
**
*****************************************************************************/

static int cleanup_pci_config(
    bt_unit_t *unit_p
    )
{
    int ret_val = 0;

    FUNCTION("cleanup_pci_config");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /* 
    ** Unmap the device in the kernel virtual address space 
    */
    iounmap((void *) unit_p->csr_p);
    iounmap((void *) unit_p->mreg_p);
    iounmap((void *) unit_p->rmem_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_defaults()
**
**      Purpose:    Set up the default values for many parameters.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**      There is no corresponding cleanup routine for this function.
**
******************************************************************************/

static int init_defaults (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_defaults");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;
    
    /* 
    ** Set the various defaults in the unit structure. 
    */
    unit_p->reset_timer = DEFAULT_RESET_TIMER;
    unit_p->dma_timeout = DEFAULT_DMA_TIMEOUT;
    unit_p->dma_threshold = DEFAULT_DMA_THRESHOLD;
    unit_p->dma_poll_size = DEFAULT_DMA_POLL;
    unit_p->a64_offset = 0;
    unit_p->bt_kmalloc_buf = NULL;
    unit_p->bt_kmalloc_ptr = NULL;
    unit_p->bt_kmalloc_size = SIZE_64KB;

    /*
    ** Set the revision info
    */
    strncpy((char *)&unit_p->driver_version, driver_version, BT_DIAG_MAX_REV_INFO-1);
    unit_p->driver_version[BT_DIAG_MAX_REV_INFO-1] = '\0';

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_boot_parms()
**
**      Purpose:    Set up the default values for many parameters.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**      There is no corresponding cleanup routine for this function.
**
**      The unit number must already be defined before calling this routine.
**
**      The only parameters that you can specify by unit are the remote bus
**      address used for Local Memory, and the local memory size.
**
******************************************************************************/

static int init_boot_parms (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_boot_parms");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;
    short unit;

    FENTRY;

    /* 
    ** Set the various boot parameters in the unit structure
    **
    ** For Linux, we use module parameters to do this 
    */
    unit = unit_p->unit_number;
    if ((unit < 0) || (unit > BT_MAX_UNITS)) {
        /* Invalid unit number in structure, oops! */
        WARN_STR("Unit number not defined yet!\n");
        unit_p->lm_size = DEFAULT_LMEM_SIZE;
        unit_p->q_size = DEFAULT_Q_SIZE;
    } else {
        unit_p->lm_size = lm_size[unit];
        unit_p->q_size = icbr_q_size[unit];
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_unit_array()
**
**      Purpose:    Adds the unit to bt_unit_array_gp[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_unit_array (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_unit_array");
    LOG_UNKNOWN_UNIT;
    int unit, ret_val = 0;

    FENTRY;

    /* 
    ** Kernel prevents open from occurring while we load the driver. 
    */

    /* 
    ** Search for an available spot in the unit array 
    */
    ret_val = -ENODEV; /* Assume there is no room */
    down(&bt_units_lock_g);
    for (unit = 0; unit < BT_MAX_UNITS + 1; unit++) {
        if(NULL == bt_unit_array_gp[unit]) {
            unit_p->unit_number = unit;
            bt_unit_array_gp[unit] = unit_p;
            SET_UNIT_NUMBER(unit);
            ret_val = 0;
            INFO_MSG((LOG_FMT "Assigning unit %d to card\n", LOG_ARG, unit));
            break;
        }
    }
    up(&bt_units_lock_g);
    if (0 > ret_val) {
        WARN_STR("Failed to add unit to global array.");
    }

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_unit_array()
**
**      Purpose:    Removes the unit from the bt_unit_array_gp[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:      Calling function should hold bt_units_lock_g.
**
******************************************************************************/

static int cleanup_unit_array (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_unit_array");
    LOG_UNKNOWN_UNIT;
    int ret_val = 0;
    short unit;

    FENTRY;

    unit = unit_p->unit_number;
    SET_UNIT_NUMBER(unit);

    /* 
    ** Do a reasonably complete consistency check 
    */
    if ( (unit < 0) || (unit > BT_MAX_UNITS) || 
         (unit_p != bt_unit_array_gp[unit])) {
        WARN_MSG((LOG_FMT "Invalid: unit = %d  unit_p = %p  bt_unit_array_gp[unit] = %p.\n",
                  LOG_ARG, unit, unit_p, bt_unit_array_gp[unit]));
        ret_val = -ENXIO;
    } else {
        bt_unit_array_gp[unit] = NULL;
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_bit_maps()
**
**      Purpose:    Initializes any bit maps used for resource tracking.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_bit_maps (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_bit_maps");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;
    FENTRY;

    if (BT_SUCCESS != 
            btk_bit_init(BT_MAX_MMAP_BIT, &unit_p->mmap_aval_p)) {
        ret_val = -ENOMEM; /* Assume memory problem */
        goto init_bit_maps_end;
    } 

    if (BT_SUCCESS != 
            btk_bit_init(BT_MAX_SDMA_BIT, &unit_p->sdma_aval_p)) {
        ret_val = -ENOMEM; /* Assume memory problem */
        (void) btk_bit_fini(unit_p->mmap_aval_p);
        goto init_bit_maps_end;
    }

    /* 
    ** Number of mapping registers per system page, usually one. 
    */
    unit_p->mr_page = BT_SYS_PAGE_SIZE / BT_PAGE_SIZE;

init_bit_maps_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_bit_maps()
**
**      Purpose:    Removes the unit from the bt_bit_maps[]
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_bit_maps (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_bit_maps");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    (void) btk_bit_fini(unit_p->mmap_aval_p);
    (void) btk_bit_fini(unit_p->sdma_aval_p);

    FEXIT(ret_val);
    return ret_val;
}



/******************************************************************************
**
**      Function:   init_lm()
**
**      Purpose:    Initializes the local memory buffer
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_lm(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_lm");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    bt_error_t      retval = BT_SUCCESS;
    unsigned int    need;
    unsigned int    start;
    unsigned int    inx;
    unsigned long   pci_addr;   /* Bus address of memory */

    bt_data32_t     mreg_value = 0; /* Value to program mapping register to */

    FENTRY;

    TRC_MSG(BT_TRC_CFG | BT_TRC_DETAIL, 
           (LOG_FMT "Unit %d lm_size = 0x%x\n", 
           LOG_ARG, unit_p->unit_number, unit_p->lm_size));

    /* 
    ** Initialize all the mapping registers into PCI as invalid 
    */
    if (unit_p->lm_size != 0) {
        if (0 != unit_p->lm_size % PAGE_SIZE) {
            WARN_STR("Local Memory device size must be multiple of system page size.\n");
            unit_p->lm_size += PAGE_SIZE;
            unit_p->lm_size -= (unit_p->lm_size % PAGE_SIZE);
        }

        /*
        ** Allocate the memory for local memory.
        **
        ** Since it is always a multiple of the page size, it is better to
        ** call vmalloc() for the buffer.
        */
        unit_p->lm_kaddr = (caddr_t) (pci_alloc_consistent(unit_p->dev_p, 
            (size_t) unit_p->lm_size, &(unit_p->lm_dma_handle)));

        if (NULL == unit_p->lm_kaddr) {
            WARN_STR("Not enough memory to allocate local memory device.\n");
            unit_p->lm_size = 0;
            goto init_lm_end;
        }
        BTK_BZERO(unit_p->lm_kaddr, unit_p->lm_size);

        /*
        ** Set up sequential mapping registers to point to our local memory
        ** buffer in kernel space.
        */
        need = unit_p->lm_size / BT_PAGE_SIZE;
        retval = btk_bit_alloc(unit_p->sdma_aval_p, need, 1, &start);
        if (retval != BT_SUCCESS) {
            pci_free_consistent(unit_p->dev_p, (size_t) unit_p->lm_size, unit_p->lm_kaddr,
                                unit_p->lm_dma_handle);
            unit_p->lm_size = 0;
            WARN_STR("No open mapping regs for local memory device");
            goto init_lm_end;
        }
        unit_p->lm_start = start;
        unit_p->lm_need = need;

        /*
        ** Setup parts of mapping ram that do not change per for loop
        ** Address modifer does not need to be programmed for PCI accesses
        */
        btk_setup_mreg(unit_p, BT_AXSLM, &mreg_value, BT_OP_BIND);

        for (inx = 0; inx < need; inx++) {
        
            /* 
            ** Get PCI physical address and program in mapping RAM
            */
            pci_addr = (bt_data32_t) unit_p->lm_dma_handle + inx * BT_PAGE_SIZE;
            mreg_value = (mreg_value & ~ BT_MREG_ADDR_MASK) | ((bt_data32_t) pci_addr & BT_MREG_ADDR_MASK);

            btk_put_mreg(unit_p, start+inx, BT_LMREG_CABLE_2_PCI, mreg_value);
            if ( (btk_get_mreg(unit_p, start+inx, BT_LMREG_CABLE_2_PCI)) != mreg_value ) {
                WARN_MSG((LOG_FMT "Verify Write BT_LMREG_CABLE_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                             LOG_ARG, start+inx));
                btk_put_mreg_range(unit_p, start, inx+1, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
                btk_put_mreg_range(unit_p, start, inx+1, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
                btk_bit_free(unit_p->sdma_aval_p, start, need);
                pci_free_consistent(unit_p->dev_p, (size_t) unit_p->lm_size, 
                    unit_p->lm_kaddr, unit_p->lm_dma_handle);
                unit_p->lm_size = 0;
                WARN_STR("Local memory device disabled.\n");
                break;
            }

            btk_put_mreg(unit_p, start+inx, BT_LMREG_DMA_2_PCI, mreg_value);
            if ( (btk_get_mreg(unit_p, start+inx, BT_LMREG_DMA_2_PCI)) != mreg_value ) {
                WARN_MSG((LOG_FMT "Verify Write BT_LMREG_DMA_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                             LOG_ARG, start+inx));
                btk_put_mreg_range(unit_p, start, inx+1, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
                btk_put_mreg_range(unit_p, start, inx+1, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
                btk_bit_free(unit_p->sdma_aval_p, start, need);
                pci_free_consistent(unit_p->dev_p, (size_t) unit_p->lm_size, 
                    unit_p->lm_kaddr, unit_p->lm_dma_handle);
                unit_p->lm_size = 0;
                WARN_STR("Local memory device disabled.\n");
                break;
            }

        }
    }

init_lm_end:

    /*
    ** Setup logical device info for the local memory device
    */
    if (unit_p->lm_size != 0) {
        unit_p->kern_addr[BT_AXSLM] = unit_p->lm_kaddr;
        unit_p->kern_length[BT_AXSLM] = unit_p->lm_size;
        unit_p->data_size[BT_AXSLM] = BT_WIDTH_ANY;
        unit_p->logstat[BT_AXSLM] |= (STAT_ONLINE | STAT_READ | STAT_WRITE | STAT_MMAP);
    } else {
        unit_p->lm_kaddr = NULL;
        unit_p->kern_addr[BT_AXSLM] = NULL;
        unit_p->kern_length[BT_AXSLM] = 0;
        unit_p->data_size[BT_AXSLM] = 0;
        unit_p->logstat[BT_AXSLM] = 0;
    }
        
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_lm()
**
**      Purpose:    Releases resources used by local memory device
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_lm(
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_lm");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Release mapping registers used for local memory device 
    */
    if ((unit_p->lm_size != 0) &&
        (unit_p->lm_kaddr != NULL)) {
        btk_put_mreg_range(unit_p, unit_p->lm_start, unit_p->lm_need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
        btk_put_mreg_range(unit_p, unit_p->lm_start, unit_p->lm_need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
        btk_bit_free(unit_p->sdma_aval_p, unit_p->lm_start, unit_p->lm_need);
        pci_free_consistent(unit_p->dev_p, (size_t) unit_p->lm_size, unit_p->lm_kaddr,
                                unit_p->lm_dma_handle);
        unit_p->lm_kaddr = NULL;
        unit_p->lm_start = 0;
        unit_p->lm_need = 0;
    }

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_cookies()
**
**      Purpose:    Initializes the cookies used for mutexs and events.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_cookies (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_cookies");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    unit_p->hirq_cookie = unit_p->irq;
    unit_p->sirq_cookie = 0;
    unit_p->task_cookie = 0;

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_events()
**
**      Purpose:    Initializes all the events used by the device driver
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_events(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_events");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /* DMA transfers */
    ret_val = btk_event_init(&unit_p->dma_event, FALSE, unit_p->sirq_cookie);
    if (BT_SUCCESS != ret_val) {
        WARN_STR("Could not create semaphore for DMA.\n");
        goto init_events_end;
    }

init_events_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_events()
**
**      Purpose:    Removes all the events used by the device driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_events (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_events");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    btk_event_fini(&unit_p->dma_event);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_mutexs()
**
**      Purpose:    Initializes a
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_mutexs (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_mutexs");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    btk_mutex_init(&(unit_p->mreg_mutex), 0);
    btk_mutex_init(&(unit_p->open_mutex), 0);
    btk_mutex_init(&(unit_p->dma_mutex), 0);
    btk_mutex_init(&(unit_p->llist_mutex), 0);
    btk_mutex_init(&(unit_p->llist_bind_mutex), 0);
    btk_mutex_init(&(unit_p->prg_irq_mutex), 0);
    btk_rwlock_init(&(unit_p->hw_rwlock));
    spin_lock_init(&unit_p->isr_lock);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_mutexs()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_mutexs (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_mutexs");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    btk_rwlock_fini(&(unit_p->hw_rwlock));
    btk_mutex_fini(&(unit_p->prg_irq_mutex));
    btk_mutex_fini(&(unit_p->llist_bind_mutex));
    btk_mutex_fini(&(unit_p->llist_mutex));
    btk_mutex_fini(&(unit_p->dma_mutex));
    btk_mutex_fini(&(unit_p->open_mutex));
    btk_mutex_fini(&(unit_p->mreg_mutex));

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_lists()
**
**      Purpose:    Initializes all the lists used by the device driver
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_lists(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_lists");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /*
    ** Lists for user interrupt handlers
    */
    btk_llist_init(&unit_p->qh_err_fn);
    btk_llist_init(&unit_p->qh_prg_fn);
    btk_llist_init(&unit_p->qh_iack_fn);
    btk_llist_init(&unit_p->qh_mmap_requests);
    btk_llist_init(&unit_p->qh_bind_requests);
    btk_llist_init(&unit_p->icbr_thread_list);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_lists()
**
**      Purpose:    Removes all the lists used by the device driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_lists (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_lists");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /*
    ** No cleanup required on llists for user interrupt handlers
    */

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   init_irq_q()
**
**      Purpose:    Initializes interrupt queue
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_irq_q(
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_irq_q");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    ret_val = (int) btk_irq_qs_init(unit_p, unit_p->q_size);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_irq_q()
**
**      Purpose:    Removes the interrupt queue used by the driver.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_irq_q (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_irq_q");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    btk_irq_qs_fini(unit_p, unit_p->q_size);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_isr()
**
**      Purpose:    Initializes the interrupt service routine
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/
#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif
static int init_isr (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_isr");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Register the ISR 
    */
    ret_val = request_irq((unsigned int)unit_p->irq, btk_isr, 
			  (unsigned long)IRQF_SHARED, 
			  bt_name_gp, (void *) unit_p);
    if (0 > ret_val) {
        WARN_MSG((LOG_FMT "Could not register shared interrupt on level %d.\n",
            LOG_ARG, unit_p->irq));
        goto init_isr_end;
    }

init_isr_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_isr()
**
**      Purpose:    Removes the interrupt service routine
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_isr (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_isr");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Turn off interrupts on the card  & Unregister our handler
    */
    btk_put_io(unit_p, LOC_INT_CTRL, LIC_DIS_INTR);
    free_irq((unsigned int)unit_p->irq, (void *) unit_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_dma()
**
**      Purpose:    Initializes driver DMA structures.
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_dma (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_dma");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /* 
    ** Need to set BT_DMA_LOCAL if the local card can do DMA 
    */
    switch (unit_p->loc_id) {
      case BT_PN_PCI_DMA:
      case BT_PN_PCI_FIBER:
        unit_p->bt_status |= BT_DMA_LOCAL;
        break;
        
#if  EAS_A64_CODE
/* EAS A64 */
      case BT_PN_PCI_DBA64_RPQ:
      case BT_PN_PCI_FIBER_D64:
        unit_p->bt_status |= BT_DMA_LOCAL;
        SET_BIT(unit_p->bt_status, BT_NEXT_GEN);
        break;
#endif  /* EAS_A64_CODE */

      case BT_PN_PCI:
      case BT_PN_PCI_NODMA:
        /* These don't have DMA, so don't set the bit */
        break;

      default:
        /* Unrecognized, so don't set the bit */
        break;
    }
    SET_BIT(unit_p->bt_status, BT_DMA_BLOCK);

    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_dma()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_dma (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_dma");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_ldev()
**
**      Purpose:    Initializes the logical device fields
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_ldev (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_ldev");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    /*
    ** Setup one time logical device info
    ** Note logical dev info that can change when cable is pulled
    ** is setup in btk_setup()
    */
    /*
    **  Setup logical unit_p->dev_vertex information for node registers
    */
    unit_p->kern_addr[BT_AXSIO] = (caddr_t) unit_p->csr_p;
    unit_p->kern_length[BT_AXSIO] = MAX_IOREGS;
    unit_p->data_size[BT_AXSIO] = BT_WIDTH_D32;
    unit_p->logstat[BT_AXSIO] |= STAT_ONLINE;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A16 space
    */
    unit_p->kern_addr[BT_AXSRI] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRI] = 0;
    unit_p->pio_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->dma_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->mmap_addr_mod[BT_AXSRI] = BT_AMOD_A16;
    unit_p->data_size[BT_AXSRI] = BT_WIDTH_ANY;
    
    /*
    **  Setup logical unit_p->dev_vertex information for remote A24
    */
    unit_p->kern_addr[BT_AXS24] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXS24] = 0;
    unit_p->pio_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->dma_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->mmap_addr_mod[BT_AXS24] = BT_AMOD_A24;
    unit_p->data_size[BT_AXS24] = BT_WIDTH_ANY;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A23 lower half
    */
    unit_p->kern_addr[BT_AXSRR] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRR] = 0;
    unit_p->pio_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->dma_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->mmap_addr_mod[BT_AXSRR] = BT_AMOD_A32;
    unit_p->data_size[BT_AXSRR] = BT_WIDTH_ANY;

    /*
    **  Setup logical unit_p->dev_vertex information for remote A23 upper half
    */
    unit_p->kern_addr[BT_AXSRE] = unit_p->rmem_p;
    unit_p->kern_length[BT_AXSRE] = 0;
    unit_p->pio_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->dma_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->mmap_addr_mod[BT_AXSRE] = BT_AMOD_A32;
    unit_p->data_size[BT_AXSRE] = BT_WIDTH_ANY;

    /*
    **  Setup logical unit_p->dev_vertex information for local dual port
    */
    unit_p->data_size[BT_AXSLDP] = BT_WIDTH_D32;

    /*
    **  Setup logical unit_p->dev_vertex information for remote dual port
    */
    unit_p->data_size[BT_AXSRDP] = BT_WIDTH_D32;

#if 0
    /*
    ** This code sets up the bt_trace_mreg_gp so it points at a free
    ** longword on the GEMS APS system the VMETRO can them be
    ** placed to trigger on a D32 access to 0x18005000.  Be careful
    ** because this address is across the cable and may interfere with
    ** DMAs.
    */
    {
    unsigned int mreg_start;
    btk_mutex_enter(&unit_p->mreg_mutex);
    (void) btk_bit_alloc(unit_p->mmap_aval_p, 1, 1, &mreg_start);
    btk_mutex_exit(&unit_p->mreg_mutex);

    btk_put_mreg(unit_p, mreg_start, BT_LMREG_PCI_2_CABLE, 0x020);
    bt_trace_mreg_gp = (volatile bt_data32_t *) (unit_p->rmem_p + mreg_start * 0x1000);
    }
#endif

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_card()
**
**      Purpose:    Initializes the adapter card
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_card (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_card");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    btk_setup(unit_p);

    FEXIT(ret_val);
    return ret_val;
}


/******************************************************************************
**
**      Function:   init_swapping()
**
**      Purpose:    Initializes default swapping
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_swapping (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_swapping");
    LOG_UNIT(unit_p->unit_number);
    
    int     inx, ret_val = 0;

    FENTRY;
    
    /*
    **  Setup swapping bits for logical unit_p->dev_vertexs depending on what 
    **  type of Adaptor we have
    */
    for (inx = 0; inx < BT_MAX_AXSTYPS; inx++) {
        switch (unit_p->rem_id) {
          case BT_PN_PCI_DMA:
          case BT_PN_PCI_NODMA:
          case BT_PN_PCI_FIBER:
          case BT_PN_PCI_FIBER_D64:
#if  EAS_A64_CODE
/* EAS A64 */
          case BT_PN_PCI_DBA64_RPQ:
#endif  /* EAS_A64_CODE */
            unit_p->swap_bits[inx] = BT_SWAP_NONE;
            break;
      case BT_PN_VME_NOINC:
          case BT_PN_VME_NODMA:
          case BT_PN_VME_SDMA:
      /*  case BT_PN_VME_SDMA_64:   BT_PN_VME_SDMA_64 == BT_PN_VME_FIBER_D64 */
      case BT_PN_VME2_DMA:
      case BT_PN_VME_DMA:
      case BT_PN_VME:
      /*  case BT_PN_VME2:	    BT_PN_VME2 == BT_PN_VME_NODMA above.     */
          case BT_PN_VME_FIBER:
      case BT_PN_VME_A24:
      case BT_PN_VME_PCI:	/* EXPN may not be supported by this driver. */
      case BT_PN_VME64:	/* 2866 may not be supported by this driver. */
      case BT_PN_VME_NBDG:	/* NBDG may not be supported by this driver. */
          case BT_PN_VME_FIBER_D64:
            unit_p->swap_bits[inx] = BT_SWAP_VMEBUS;
            break;
          case BT_PN_QBUS:
            unit_p->swap_bits[inx] = BT_SWAP_QBUS;
            break;
          case BT_PN_MB:
            unit_p->swap_bits[inx] = BT_SWAP_MULTIBUS;
            break;
          case BT_PN_UNKNOWN:
          default:
            /*
            ** For next gen assume PCI to PCI
            */
#if  EAS_A64_CODE
/* EAS A64 */
            if ((unit_p->loc_id == BT_PN_PCI_FIBER_D64) ||
                (unit_p->loc_id == BT_PN_PCI_DBA64_RPQ)) {
#else
            if (unit_p->loc_id == BT_PN_PCI_FIBER_D64) {
#endif  /* EAS_A64_CODE */
              unit_p->swap_bits[inx] = BT_SWAP_NONE;
            } else {
              unit_p->swap_bits[inx] = BT_SWAP_VMEBUS;
            }
            break;
        }
    }

    FEXIT(ret_val);
    return ret_val;
}
#if 0

/******************************************************************************
**
**      Function:   init_xxx()
**
**      Purpose:    Initializes a
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int init_xxx (
    bt_unit_t * unit_p
    )
{
    FUNCTION("init_xxx");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;


init_xxx_end:
    FEXIT(ret_val);
    return ret_val;
}

/******************************************************************************
**
**      Function:   cleanup_xxx()
**
**      Purpose:    Removes 
**
**      Args:       unit_p      Pointer to the unit structure being initialized
**
**      Returns:    0           Success
**                  else        error number
**
**
**      Notes:
**
******************************************************************************/

static int cleanup_xxx (
    bt_unit_t * unit_p
    )
{
    FUNCTION("cleanup_xxx");
    LOG_UNIT(unit_p->unit_number);
    int ret_val = 0;

    FENTRY;

    FEXIT(ret_val);
    return ret_val;
}

#endif /* 0 */
