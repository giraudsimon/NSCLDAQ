/*****************************************************************************
**
**      Filename:   bt_isr.c
**
**      Purpose:    Shared PCI Nanobus interrupt handler and related
**                  routines.
**
**      Functions:  btk_intr(), btk_dma_watchdog() 
**
**      $Revision: 2330 $
**
*****************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000-05 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/
#if !defined (LINT)
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif  /* !LINT */

/*
**  Include files
*/
#include "btdd.h"

/*
**  Local defines
*/
#define	VECTOR_8 (0xff00ul)	/* Check if only 8-bit vector returned */

/*
**  Exported function prototypes
*/

#if defined(__sgi)
void btk_isr(intr_arg_t arg);

#elif defined(__linux__)
/* A type return is now required for ISRs in the 2.6 kernel */
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,69)
#include <linux/interrupt.h>
#if    LINUX_VERSIOLN_CODE <= KERNEL_VERSION(4,9,0)
irqreturn_t btk_isr(int irq, void *vunit_p, struct pt_regs *regs);
#else
irqreturn_t btk_isr(int irq, void* vunit_p);
#endif
#else
void btk_isr(int irq, void *vunit_p, struct pt_regs *regs);
#endif

#elif defined(__vxworks)
void btk_isr(bt_unit_t *unit_p);

#elif defined(BT_NTDRIVER)
void btk_isr(
    IN PKDPC dpc_p,
    IN void *deferred_context_p,
    IN void *sys_arg1,
    IN void *sys_arg2);

#elif defined(__sun)
u_int btk_isr(caddr_t isr_param_p);
/*
** 946 needs to get the vector then call user isrs because it 
** emulates the Solaris DDI routines that lets ISRs be installed 
** on a given vector.
*/
#if defined (BT946)
extern bt_data32_t btp_user_isr(bt_unit_t *unit_p, btk_llist_t *qhead_p, bt_irq_t irq_type, bt_data32_t vector, bt_data8_t level);
#endif /* defined(BT946) */

#elif defined (__lynxos)

void btk_isr(bt_unit_t *unit_p);

#else /* Unknown OS */
#error Unknown OS
#endif /* Unknown OS */

void btk_dma_watchdog(caddr_t arg_p);

/*
**  Imported function prototypes
*/

/*
**  Private function prototypes
*/
static bt_data32_t bt_user_isr(bt_unit_t *unit_p, btk_llist_t *qhead_p, bt_irq_t irq_type, bt_data8_t cint);
static void bt_logerr_ints(bt_unit_t *unit_p, u_char unit_status);
static void bt_drv_vector(bt_unit_t *unit_p, bt_data32_t vector);

/*
**  Declare global variables defined elsewhere and used within
*/
extern void btk_dma_stop(bt_unit_t *unit_p);
extern void btk_insert_irq(bt_unit_t *unit_p, bt_irq_q_t *irq_q_p, bt_data32_t vector);
extern void btk_wakeup_thread(bt_unit_t *unit_p, int error_irq, int prog_irq, int vme_irq);
extern bt_data32_t btk_get_io(bt_unit_t *unit_p, bt_reg_t reg);
extern void btk_put_io(bt_unit_t *unit_p, bt_reg_t reg, bt_data32_t value);
extern bt_data32_t btk_get_mreg(bt_unit_t *unit_p, unsigned int mr_idx, bt_mreg_t mreg_type);
extern void btk_put_mreg(bt_unit_t *unit_p, unsigned int mr_idx, bt_mreg_t mreg_type, bt_data32_t value);

/*
**  List local variables here
*/
BT_FILE_NUMBER(TRACE_BT_ISR_C);

/******************************************************************************
**
**  Function:   btk_isr()
**
**  Purpose:    Handles device interrupts from the Bit 3 Adaptor.
**
**  Args:       arg     Pointer to the unit structure.
**
**  Returns:    0   All the time (taken from SGI example driver).
**              Linux returns a irqreturn_t type in the 2.6 kernel.
**
**  Notes:  Currently I still keep track of whether we service the interrupt
**          eventhough IRIX does not care.
**
******************************************************************************/

#if defined(__sgi)
void btk_isr(
    intr_arg_t arg)

#elif defined(__vxworks)
void btk_isr(
    bt_unit_t *unit_p)
    
#elif defined(BT_NTDRIVER)
/*
**  Called s a custom deferred procedure call in an arbitrary
**  thread context.  Processes adaptor's card interrupts.
*/

void btk_isr(
    IN PKDPC dpc_p,   /* pointer to actual DPC object */
    IN void *deferred_context_p, /* pointer to unit structure for the device */
    IN void *sys_arg1, /* not used */
    IN void *sys_arg2) /* not used */

#elif defined(__sun)
u_int btk_isr(
    caddr_t isr_param_p)

#elif defined(__linux__)
#if     LINUX_VERSION_CODE < KERNEL_VERSION(2,5,69)
void btk_isr(
#else
irqreturn_t btk_isr(
#endif
#if LINKUX_VERSIONL_CODE < KERNEL_VERSION(4,9,0)
		    int irq,
    void *vunit_p,
    struct pt_regs *regs)
#else
int irq, *vunit_p)
#endif

#elif defined (__lynxos)
void btk_isr(
        bt_unit_t *unit_p)

#else /* Unknown OS */
#error Unknown OS
#endif /* Unknown OS */
{
    FUNCTION("btk_isr");
    LOG_UNKNOWN_UNIT;

    bt_data32_t         reg_value;
    bt_data32_t         vector = 0, mreg_vector = 0;
    bt_data8_t          eint_pending, cint_pending;
    bt_data8_t          status, int_status, rstatus, dma_cmd;
    BTK_LOCK_RETURN_T   isr_pl;
    int                 vme_level, error_irq, prog_irq, vme_irq, dma_irq;

#if defined(__sgi)
    bt_unit_t           *unit_p;
#endif /* defined(__sgi) */

#if defined(BT_NTDRIVER)
    bt_unit_t           *unit_p;
    BOOLEAN             nt_retval = TRUE;
#endif /* defined(BT_NTDRIVER) */

#if defined(__sun)
    bt_unit_t           *unit_p;
    u_int sun_retval = DDI_INTR_CLAIMED;
#endif /* defined(__sun) */

#if defined(__linux__)
    bt_unit_t           *unit_p;
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,69)
    irqreturn_t handled = IRQ_NONE;
#endif
#endif /* defined(__linux__) */

    FENTRY;


    /*
    ** The following code must set unit_p and the t_unit
    */
#if defined(__sgi)
    /*
    **  Make sure we have a vaild unit pointer
    **  Panic the system, otherwise it may get hung at interrupt
    **  level if the interrupt was from our device.
    */
    {
    long             intarg;
    intarg = (long) arg;
    if ((intarg < 0) || (intarg > BT_MAX_UNITS) || 
        (bt_unit_array_gp[intarg] == NULL)) {
        bt_logpri_g = CE_PANIC;
        cmn_err(LOG_FMT "%s", LOG_ARG, 
                "SBS ISR called with NULL unit pointer");
        FEXIT(0);
        return;
    } else {
        unit_p = bt_unit_array_gp[intarg];
    }
    SET_UNIT_NUMBER(unit_p->unit_number);
    }

#elif defined(__vxworks)
    /*
    **  Make sure we have a vaild unit pointer
    */
    if (NULL == unit_p) {
        WARN_STR("SBS ISR called with NULL unit pointer");
        FEXIT(0);
        return;
    } else {
        SET_UNIT_NUMBER(unit_p->unit_number);
    }

#elif defined(BT_NTDRIVER)
    /*
    **  Make sure we have a vaild unit pointer
    */
    unit_p = (bt_unit_t *) deferred_context_p;
    if (NULL == unit_p) {
        WARN_STR("SBS ISR called with NULL unit pointer");
        FEXIT(FALSE);
        return;
    } else {
        LOG_UNIT(unit_p);
    }

#elif defined(__sun)
    unit_p = (bt_unit_t *) isr_param_p;
    if (NULL == unit_p) {
        FATAL_STR("SBS ISR called with NULL unit pointer");
        FEXIT(DDI_INTR_UNCLAIMED);
        return DDI_INTR_UNCLAIMED;
    }
    SET_UNIT_NUMBER(unit_p->unit_number);

#elif defined(__linux__)
    unit_p = (bt_unit_t *) vunit_p;
    if ((NULL == unit_p) || (irq != unit_p->irq)) {
        FEXIT(0);
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,69)
        return(handled);
#else
        return;
#endif
    }
    SET_UNIT_NUMBER(unit_p->unit_number);

#elif defined (__lynxos)
    /*
    **  Make sure we have a vaild unit pointer
    */
    if (unit_p == NULL) {
        WARN_STR("SBS ISR called with NULL unit pointer");
        FEXIT(0);
        return;
    } else {
        SET_UNIT_NUMBER(unit_p->unit_number);
    }

#else /* Unknown OS */
#error Unknown OS
#endif

    /*
    ** Get the ISR lock to prevent two instances of our ISR running
    ** on different processor and to prevent conflicts with manipulating
    ** data that is shared between the ISR and other parts of the driver
    */
    error_irq = prog_irq = vme_irq = dma_irq = FALSE;
#if !defined(BT_NTDRIVER)
    /*
    ** Note: 983 and 984
    **
    ** Don't get ISR lock, Windows does this for you.  Calling this
    ** function again actually drops your IRQL down and the OS is
    ** able to call and enter this ISR twice.  This failure mode
    ** showed up running irqs -h 2000, as a wrong vector when 
    ** the ISR was entered twice the first instance was interrupted
    ** right before cable interrupts, the second instance handled the
    ** cable interrupt then the first instance did another IACK and
    ** got a bus error.
    */
    BTK_LOCK_ISR(unit_p, isr_pl);


#endif /* BT_NTDRIVER */

    /*
    ** Get and check current value of local interrupt control register.
    ** If the adaptor shows no interrupt(s) pending, exit immediately.
    */
    reg_value = btk_get_io(unit_p,BT_LOC_INT_CTRL);
    TRC_MSG(BT_TRC_ISR, 
           (LOG_FMT "ISR invoked: loc_int_ctrl 0x%x\n",
           LOG_ARG, reg_value));
    if (IS_CLR(reg_value, LIC_INT_PENDING)) {
#if defined(__sun)
        sun_retval = DDI_INTR_UNCLAIMED;
#endif /* __sun */
#if defined(BT_NTDRIVER)
        nt_retval = FALSE;
#endif /* BT_NTDRIVER */
        goto end_intr;
    }
#if defined(__linux__)
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,69)
    handled = IRQ_HANDLED;
#endif
#endif

    /*
    ** We need to read a remote register in case this is a remote bus
    ** time out- if we don't, and we try to do a remote access, the
    ** 413 gets terminally confused.  Just do it early and get it out
    ** of the way.
    */
    if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
        /* Doing this during a DMA would be bad, but 620-3 and 618-3 */
        /* block interrupts during a DMA, so this must be DMA done   */
        /* or an error in which case we can do this remote register  */
        /* read */

        /* Just to be safe let's first read the local status register */
        /* and then only try to read the remote status if the cable   */
        /* local status register reports the cable as on. */

        if (IS_CLR(btk_get_io(unit_p, BT_LOC_STATUS), LSR_NO_CONNECT)) {
            (void) btk_get_io(unit_p, BT_REM_STATUS);
        }
    }


    /*
    ** Record which cable and error interrupts are pending
    */
    status = (bt_data8_t) btk_get_io(unit_p, BT_LOC_STATUS);
    int_status = (bt_data8_t) btk_get_io(unit_p, BT_LOC_INT_STATUS);
    eint_pending = status & LSR_CERROR_MASK;
    cint_pending = int_status  & LIS_CINT_MASK;

    /*
    ** If any error(s) are detected then process them below.
    ** Obtain current adaptor status and save any last error.
    ** If we're missing power, make note of this fact as well.
    */
    if (eint_pending) {

        TRC_MSG(BT_TRC_ISR_ERR, 
                (LOG_FMT "Error interrupt, status 0x%x, dma %s active\n", LOG_ARG, status, 
                (IS_SET(unit_p->bt_status, BT_DMA_ACTIVE)) ? "is" : "is not"));

        /* 
        ** Log errors to system error log if so configured 
        */
        if (IS_SET(unit_p->bt_status, BT_LOGERR)) {
            bt_logerr_ints(unit_p, eint_pending);
        }

        /*
        ** Set the error flags, note we are doing both PIO and
        ** DMA because if we are in part of the DMA setup
        ** routine we can't tell if the DMA setup cause the
        ** error or a PIO xfer caused the error
        ** The PIO error BT_ERROR is set when pio_count indicates
        ** an active PIO.  Keeps MMAP errors from setting BT_ERROR.
        */

        if (unit_p->pio_count)
            SET_BIT(unit_p->bt_status, BT_ERROR);

        SET_BIT(unit_p->bt_status, BT_DMA_ERROR);
        
        /*
        ** If no power reset rem_id, update last_error
        */
        if (IS_SET(eint_pending, LSR_NO_CONNECT)) {
            SET_BIT(unit_p->bt_status, BT_POWER);
            unit_p->rem_id = BT_PN_UNKNOWN;
        }
        unit_p->last_error |= eint_pending;

        /* 
        ** Note we've detected an error interrupt for later signal
        ** If any error interrupt handlers are registered, invoke them
        */
        error_irq = TRUE;
#if !defined(BT946)
        if (IS_SET(unit_p->bt_status, BT_ER_INTR)) {
            vector = bt_user_isr(
                         unit_p, 
                         &unit_p->qh_err_fn, 
                         BT_IRQ_ERROR, 0);
        }
#else /* !defined(BT946) */
        /* Just to be safe let's first read the local status register */
        /* and then only allow user isr if the cable is on.  */

        if (IS_CLR(btk_get_io(unit_p, BT_LOC_STATUS), LSR_NO_CONNECT)) {
            vector = btp_user_isr(
                         unit_p, 
                         &unit_p->qh_err_fn, 
                         BT_IRQ_ERROR, 0, 0);
        }
#endif /* !defined(BT946) */


        /* 
        ** Add the interrupt to the error q
        */
        btk_insert_irq(unit_p, unit_p->error_irq_q_p, (vector == 0) ? BT_ESTATUS : vector);

        /*
        ** Increment total interrupts as well as error interrupts
        ** specifically before clearing error interrupt condition.
        */
        unit_p->sig_count++, unit_p->sig_err_cnt++;
        btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);

        /*
        ** Disable interrupts if power is off and an interrupt is still 
        ** pending.
        */
        if (IS_SET(btk_get_io(unit_p,BT_LOC_STATUS),LSR_NO_CONNECT) &&
            IS_SET(btk_get_io(unit_p,BT_LOC_INT_CTRL),LIC_INT_PENDING)) {
            WARN_STR("Power off, can't clear interrupt. Disabling all device interrupts!");
            btk_put_io(unit_p, BT_LOC_INT_CTRL, LIC_DIS_INTR);
        }
    }

    /*
    ** If a DMA is active we need to check for DMA done interrupt
    */
    if (IS_SET(unit_p->bt_status, BT_DMA_ACTIVE)) {

        /*
        ** If DMA done interrupt is set and DMA done is set
        ** wakeup the btk_dma_xfer and cancel the timeout
        */
        TRC_STR(BT_TRC_ISR, "DMA interrupt");
        dma_irq = TRUE;
        dma_cmd = (bt_data8_t)btk_get_io(unit_p, BT_LDMA_CMD);
        if (IS_SET(dma_cmd, LDC_DMA_INT_ENABLE) &&
            IS_SET(dma_cmd, LDC_DMA_DONE)) {
            btk_put_io(unit_p, BT_LDMA_CMD, 0);
            CLR_BIT(unit_p->bt_status, BT_DMA_ACTIVE);
            btk_event_set(&unit_p->dma_event);
#if defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__)
            /*
            ** We do not use watchdog function to set event for
            ** NT or VxWorks instead use timeout feature of btk_event_wait()
            */      
#else /* defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__) */

            btk_untimeout(btk_dma_watchdog, (caddr_t) unit_p, unit_p->watchdog_id);
#endif /* defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__) */
        }
    }

    /*
    ** If the cable is disconnected or there is not another interrupt
    **  pending then exit ISR
    */
    if (IS_SET(unit_p->bt_status, BT_POWER) ||
        IS_CLR(btk_get_io(unit_p,BT_LOC_INT_CTRL),LIC_INT_PENDING)) {
        goto end_intr1;
    }

    /* 
    ** Handle PT/PR interrupts
    */
    if (IS_SET(unit_p->bt_status, BT_NEXT_GEN) ||
        IS_CLR(unit_p->bt_status, BT_DMA_ACTIVE)) {

        /* 
        ** Check for PT if the cable is connected
        */
        rstatus = (bt_data8_t) btk_get_io(unit_p, BT_REM_STATUS);
        if (IS_SET(rstatus, RSR_PT_STATUS)) {
            
            TRC_MSG(BT_TRC_ISR, 
                   (LOG_FMT "PT interrupt, rstatus 0x%x\n",
                   LOG_ARG, rstatus));

            /*
            ** See if a vector was sent from the other driver or a remote
            ** application
            */
            mreg_vector = btk_get_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_LMREG_PCI_2_CABLE);
            btk_put_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_RMREG_PCI_2_CABLE, 0);

            /*
            ** Handle driver message interrupts
            */
            if (mreg_vector >= BT_DRV_VECTOR_BASE) {
                bt_drv_vector(unit_p, mreg_vector);

            /*
            ** Handle application level programmed interrupts
            */
            } else {
  
                /* 
                ** If any prgm'd interrupt handlers are registered, invoke them
                */
#if !defined(BT946)
                if (IS_SET(unit_p->bt_status, BT_PT_INTR)) {
                    vector = bt_user_isr(unit_p, &unit_p->qh_prg_fn, BT_IRQ_PRG, 0);
                }
#else /* !defined(BT946) */
               vector = btp_user_isr(unit_p, &unit_p->qh_prg_fn, BT_IRQ_PRG, 0, 0);
#endif /* !defined(BT946) */
                if (vector == 0) {
                    vector = mreg_vector;
                }
                
                /*
                ** Note we've detected a PT interrupt for later signal
                ** Add the interrupt to the programmed q
                */
                prog_irq = TRUE;
                btk_insert_irq(unit_p, unit_p->prog_irq_q_p, vector);
  
                /*
                ** Increment total interrupts as well as prgm'd interrupts
                */
                unit_p->sig_count++, unit_p->sig_prg_cnt++;
            }

            /*
            ** Clear out the PT interrupt condition.
            */
            btk_put_io(unit_p, BT_REM_CMD1, RC1_CLR_PT_INT);
            /*
            ** Bug to fix loopback local card diagnostics, bt_lcard_diag()
            ** PT interrupt was not going away till second remote register access
            */
            if (IS_SET(unit_p->bt_status, BT_LOOPBACK)) {
                (void) btk_get_io(unit_p, BT_REM_STATUS);
            }
            cint_pending = 0;
        }

        /* 
        ** Check for PR interrupt
        */
        if (IS_SET(status, LSR_PR_STATUS)) {
  
            TRC_MSG(BT_TRC_ISR, 
                   (LOG_FMT "PR interrupt, loc_status 0x%x\n",
                   LOG_ARG, status));

            /*
            ** See if a vector was sent from the other driver or a remote
            ** application
            */
            mreg_vector = btk_get_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_LMREG_PCI_2_CABLE);
            btk_put_mreg(unit_p, BT_MREG_PRG_VECTOR, BT_RMREG_PCI_2_CABLE, 0);

            /*
            ** Handle driver message interrupts
            */
            if (mreg_vector >= BT_DRV_VECTOR_BASE) {
                bt_drv_vector(unit_p, mreg_vector);

            /*
            ** Handle application level programmed interrupts
            */
            } else {
  
                /* 
                ** If any prgm'd interrupt handlers are registered, invoke them
                */
#if !defined(BT946)
                if (IS_SET(unit_p->bt_status, BT_PT_INTR)) {
                    vector = bt_user_isr(unit_p, &unit_p->qh_prg_fn, BT_IRQ_PRG, 0);
                }
#else /* !defined(BT946) */
               vector = btp_user_isr(unit_p, &unit_p->qh_prg_fn, BT_IRQ_PRG, 0, 0);
#endif /* !defined(BT946) */
                if (vector == 0) {
                    vector = mreg_vector;
                }
                
                /*
                ** Note we've detected a PR interrupt for later signal
                ** Add the interrupt to the programmed q
                */
                prog_irq = TRUE;
                btk_insert_irq(unit_p, unit_p->prog_irq_q_p, vector);
  
                /*
                ** Increment total interrupts as well as prgm'd interrupts
                */
                unit_p->sig_count++, unit_p->sig_prg_cnt++;
            }

            /*
            ** Clear out the PR interrupt condition.
            */
            btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_PR_INT);
        }

        /*
        ** Check if there is a cable interrupt(s) still outstanding.
        ** PT interrupt processing will have cleared cint_pending.
        */
        if (cint_pending) {
  
            TRC_MSG(BT_TRC_ISR, 
                   (LOG_FMT "Cable interrupt, int_status 0x%x\n",
                   LOG_ARG, int_status));

            /*
            ** Loop for each cable interrupt
            */
            for (vme_level = 1; vme_level < 8; vme_level++) {
                if (cint_pending & (1u << vme_level)) {
  
                    /* 
                    ** Note we've detected a cable interrupt for later signal
                    ** If any cable interrupt handlers are registered, invoke them
                    */
#if !defined(BT946)
                    vme_irq = TRUE;
                    if (unit_p->bt_status & BT_RB_INTR) {
                        vector = bt_user_isr(unit_p, &unit_p->qh_iack_fn, BT_IRQ_IACK, (bt_data8_t) (1 << vme_level));
                    }
  
                    /*
                    ** If no user ISR handled the interrupt, then do the IACK
                    ** This allows users to not have to install a user ISR
                    ** for ROAK VMEbus devices, however, we can not
                    ** tell if the interrupt was cleared and another devices
                    ** interrupt was pending on the same irq line, or the
                    ** interrupt was not cleared and it is actually a
                    ** RORA device without a user ISR installed.
                    **
                    ** Since to disable interrupt when two interrupts were
                    ** pending is a bug in our ISR we do not disable 
                    ** interrupt any more.  This means that it will be possible
                    ** for the driver to hang at interrupt level, but that
                    ** will only happen if the user has made a programming
                    ** error and not installed a user ISR for a RORA device
                    */
                    if (vector == 0) {

                        /* 
                        ** Do the VMEbus IACK cycle 
                        */
                        btk_put_io(unit_p, BT_REM_CMD1, vme_level);
                        vector = btk_get_io(unit_p, BT_REM_IACK);
                        if (VECTOR_8 == (VECTOR_8 & vector)) {
                            /* Adjust for 8-bit vectors */
                            vector &= ~VECTOR_8;
                        }
                    }
#else /* !defined(BT946) */
                    btk_put_io(unit_p, BT_REM_CMD1, vme_level);
                    vector = btk_get_io(unit_p, BT_REM_IACK);
                    if (VECTOR_8 == (VECTOR_8 & vector)) {
                        vector &= ~VECTOR_8;
                    }
                    vme_irq = TRUE;
                    /* 
                    ** (void *) cast is because most Solaris DDI isr return either
                    ** DDI_INTR_UNCLAIMED or DDI_INTR_CLAIMED, thus we would lose 
                    ** the actual vector to be passed back to the application level
                    ** so instead we pass back the IACK vector
                    */

                    (void *) btp_user_isr(unit_p, &unit_p->qh_iack_fn, BT_IRQ_IACK, vector, vme_level);
                    /*  
                    ** Returning a value from btp_user_isr() is a custom modification 
                    ** for Raytheon.  This change must not be made to the SBS standard
                    ** product.  For Raytheon build replace above line with the following.     
                    ** vector = btp_user_isr(unit_p, &unit_p->qh_iack_fn, 
                    **                       BT_IRQ_IACK, vector, vme_level); 
                    */

#endif /* !defined(BT946) */
                    
                    /* 
                    ** Add the interrupt to the vme q
                    */
                    btk_insert_irq(unit_p, unit_p->vme_irq_q_p, vector);

                    /*
                    ** Increment total interrupts as well as other interrupts
                    */
                    unit_p->sig_count++, unit_p->sig_other_cnt++;
                }
            }
        }
    }

end_intr1:
    /*
    ** Hack for dataBLIZZARD rev  <= E
    ** Card sometimes (with slow ramping power supply) may generate IRQ
    ** for cable disconnected but not register it in status register.  This causing
    ** Octane to hang (testing GEMS at Plexus).  So if dB and rev <= E and no
    ** irq found we write 0x80 to command register and don't log error else
    ** disable IRQs and log warning
    */
    if ((!error_irq) &&
        (!dma_irq) &&
        (!prog_irq) &&
        (!vme_irq)) {
        if (IS_SET(unit_p->bt_status, BT_NEXT_GEN) &&
            (unit_p->board_revision <= (bt_data8_t) 'E')) {
            btk_put_io(unit_p, BT_LOC_CMD1, LC1_CLR_ERROR);
        } else {
            if (IS_SET(btk_get_io(unit_p,BT_LOC_INT_CTRL),LIC_INT_PENDING)) {
                WARN_STR("No IRQ found, IRQ still pending, IRQs being disabled");
                btk_put_io(unit_p, BT_LOC_INT_CTRL, LIC_DIS_INTR);
            }
        }
    }
    /*
    ** Wake up the sleeping ICBR threads
    */
    btk_wakeup_thread(unit_p, error_irq, prog_irq, vme_irq);

    
end_intr:
#if !defined(BT_NTDRIVER)
    /* See note on BTK_LOCK_ISR  call */
    BTK_UNLOCK_ISR(unit_p, isr_pl);
#endif /* BT_NTDRIVER */

#if defined(__sgi)
    FEXIT(0);
    return;

#elif defined(__vxworks)
    FEXIT(0);
    return;

#elif defined(BT_NTDRIVER)
    /* allow the dpc to run again by setting the flag 
    ** used in btw_isr() to start the dpc.  
    ** This flag keeps the btw_isr() routine from 
    ** polling the interrupt pending reg.    
    */
    unit_p->dpc_busy_flag = 0;

    /* re-enable the interrupt on our card */
    btk_put_io(unit_p, BT_LOC_INT_CTRL, LIC_EN_INTR);

    FEXIT(nt_retval);
    return;
    
#elif defined(__sun)
    FEXIT(sun_retval);
    return sun_retval;

#elif defined(__linux__)
    FEXIT(0);
#if     LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,69)
    return(handled);
#else
    return;
#endif
    
#elif defined (__lynxos)

    FEXIT(0);
    return;

#else /* Unknown OS */
#error Unknown OS
#endif /* Unknown OS */
}


/****************************************************************************
**
**      Function:   btk_dma_watchdog()
**
**      Purpose:    Terminate a DMA transfer that takes too long.
**
**      Args:       buf_p   Pointer to buf structure for current DMA xfer.
**
**      Returns:    void
**
**      Notes:      Runs at interrupt level, so it's an AIX 4.1
**                  interrupt-thread critical section.
**
****************************************************************************/
void btk_dma_watchdog(
  caddr_t arg_p)
{

  bt_unit_t         *unit_p = (bt_unit_t *) arg_p;
  BTK_LOCK_RETURN_T saved_pl;
  LOG_UNIT(unit_p->unit_number);

  FUNCTION("btk_dma_watchdog");
  FENTRY;

  /*
  ** Hold the same lock as the interrupt handler so we do not 
  ** enter the timeout function before we start sleeping and
  ** accidently (not sure how it would happen) enter the  
  ** timeout function and the interrupt handler
  */
  BTK_LOCK_ISR(unit_p, saved_pl);

  if (IS_SET(unit_p->bt_status, BT_DMA_ACTIVE)) {

    INFO_STR("IRQ DMA failed: DMA watchdog timeout");
    
    /*
    ** Stop the DMA and wake up strategy
    */
    btk_dma_stop(unit_p);
    CLR_BIT(unit_p->bt_status, BT_DMA_ACTIVE);
    SET_BIT(unit_p->bt_status, BT_DMA_ERROR);
#if defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__)
    /*
    ** We do not use watchdog function to set event for
    ** NT || VxWorks instead use timeout feature of btk_event_wait()
    */      
#else /* defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__) */

    btk_event_set(&unit_p->dma_event);

#endif /* defined(BT_NTDRIVER) || defined(__vxworks) || defined(__sun) || defined(__linux__) */
    
  }
  BTK_UNLOCK_ISR(unit_p, saved_pl);

  FEXIT(0);
  return;
}


/******************************************************************************
**
**      Function:   bt_user_isr()
**
**      Purpose:    Call all the queued kernel interrupt handlers.
**
**      Args:       unit_p          Unit interrupting.
**                  qhead_p         Pointer to queue head of handlers.
**                  irq_type        Flag specifying what type of interrupt.
**                  cint            Local interrupt status register value
**
**      Returns:    0               Continue processing rest of queue.
**                  Otherwise       Serviced the interrupt, do not call
**                                  any more queued handlers.
**
**      Notes:      Runs at interrupt level, but the caller has already
**                  acquired the ISR lock
**
******************************************************************************/
static bt_data32_t bt_user_isr(
    bt_unit_t *unit_p, 
    btk_llist_t *qhead_p, 
    bt_irq_t irq_type,
    bt_data8_t cint)
{
    FUNCTION("bt_user_isr");
    LOG_UNIT(unit_p->unit_number);

    bt_data32_t         vector = 0;
#if !defined(BT946)
    btk_llist_elem_t    *curr_p;
    bt_rembus_intr_t    *data_p;

    FENTRY;

    TRC_MSG((BT_TRC_IRQ_DISP | BT_TRC_DETAIL),
            (LOG_FMT "irq type 0x%x; cint 0x%x\n", 
            LOG_ARG, (int) irq_type, (int) cint));

    /*
    ** Step through each user ISR and call until one returns != 0
    */
    curr_p = btk_llist_first(qhead_p);
    while (curr_p != (btk_llist_elem_t *) NULL) {
        data_p = (bt_rembus_intr_t *) btk_llist_elem_data(curr_p);

        /*
        ** For IACK registers we also look at interrupt level
        */
        if ((irq_type == BT_IRQ_IACK) && 
            !((1<<data_p->level) & cint)) {
            curr_p = btk_llist_next(curr_p);
            continue;
        }

        /* 
        ** Call the registered custom Interrupt Service Routine 
        */
        TRC_STR(BT_TRC_IRQ_DISP, "custom kernel isr entry");
        vector = (*data_p->interrupt_routine)((u_short) unit_p->unit_number, data_p->param, irq_type);
        TRC_MSG(BT_TRC_IRQ_DISP, 
                (LOG_FMT "custom kernel isr returns 0x%x\n",
                LOG_ARG, vector));

        /*
        ** If the user's isr handled the interrupt then don't call 
        ** any others
        */
        if (vector != 0) {
            break;
        }
        curr_p = btk_llist_next(curr_p);
    }
#endif /* !defined(BT946) */
    FEXIT(vector);
    return(vector);
}

/******************************************************************************
**
**  Function:   bt_logerr_ints()
**
**  Purpose:    Log currently outstanding error interrupt(s) to
**              the system's error logging facility as necessary.
**
**  Args:       unit_p          Pointer to unit structure.
**              unit_status     Unit's local status.
**
**  Returns:    Void
**
**  Notes:      To allow logging at special severity level 'CE_NOTE'
**              we'll invoke cmn_err() directly instead of using the
**              normal Bit3 tracing macros.
**
******************************************************************************/
static void bt_logerr_ints(
  bt_unit_t *unit_p, 
  u_char unit_status)
{
    FUNCTION("bt_logerr_ints");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    /* 
    **  If no power on remote end, log a cable disconnect message only 
    */
    if (IS_SET(unit_status, LSR_NO_CONNECT)) {
        WARN_STR("Cable disconnected or power off.");

    /* 
    **  Otherwise, since errors are latched we must check for all others 
    */
    } else {

        /* 
        **  If parity error, log the appropriate parity error message 
        */
        if (IS_SET(unit_status, LSR_PARITY_ERR)) {
            WARN_STR("Interface parity error (Transmit).");
        }

        /* 
        **  If remote bus error, log a remote bus error message 
        */
        if (IS_SET(unit_status, LSR_REMBUS_ERR)) {
            WARN_STR("Remote bus error.");
        }

        /* 
        **  If timeout error, log an interface timeout error message 
        */
        if (IS_SET(unit_status, LSR_TIMEOUT_ERR)) {
            WARN_STR("Interface timeout.");
        }

        /* 
        **  If LRC error, log an LRC error message 
        */
        if (IS_SET(unit_status, LSR_LRC_ERR)) {
            WARN_STR("LRC error.");
        }

    }
    
    FEXIT(0);
    return;
}

/******************************************************************************
**
**  Function:   bt_drv_vector()
**
**  Purpose:    Implement the given vector passed by the remote driver
**
**  Args:       unit_p          Pointer to unit structure.
**              vector          Remote driver's vector
**
**  Returns:    Void
**
**  Notes:      
**
******************************************************************************/
static void bt_drv_vector(
  bt_unit_t *unit_p, 
  bt_data32_t vector)
{
    FUNCTION("bt_drv_vector");
    LOG_UNIT(unit_p->unit_number);

    FENTRY;

    switch (vector) {

      /* 
      **   
      */
      case BT_DRV_VECTOR_PEER2PEER:
        SET_BIT(unit_p->bt_status, BT_PEER2PEER);
        break;

      /* 
      ** Other driver has released semaphore 
      */
      case BT_DRV_VECTOR_SEMA_RELEASE:
        btk_event_set(&unit_p->dma_event);
        break;

      default:
        WARN_MSG((LOG_FMT "Unknown driver message %x\n", LOG_ARG, vector));
        break;

    }
    
    FEXIT(0);
    return;
}

