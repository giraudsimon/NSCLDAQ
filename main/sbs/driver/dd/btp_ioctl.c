/******************************************************************************
**
**      Filename:       btp_ioctl.c
**
**      Purpose:        Device special functions for Linux driver.
**
**      Functions:      btp_ioctl()
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $" ;
#endif  /* LINT */

#include "btdd.h"

#include <asm/uaccess.h> /* Required for copy_to/from_user() */

/* 
** Some constants used in this module 
*/
const char *inv_parm_val_p = "Invalid parameter value.";
const char *inv_parm_length_p = "Invalid parameter length.";
const char *no_parms_p = "Command should not have any parameters.";

/* 
** Some macros used in this module 
*/
#define MAX_IOCTL_ARG   (256)   /* We are using less than the maximum size */

#define CHKPARM(parmstruc)  if (cmd_len != sizeof(parmstruc)) { \
    INFO_STR(inv_parm_length_p); \
    retval = -EINVAL; break; }

#define NOPARM  if (cmd_len != 0) { \
    INFO_STR(no_parms_p); \
    retval = -EINVAL; break; }

#define LOCK_DEVICE(u_p)    btk_mutex_enter(&(u_p)->dma_mutex); \
                            btk_rwlock_wr_enter(&(u_p)->hw_rwlock);

#define UNLOCK_DEVICE(u_p)  btk_rwlock_wr_exit(&(u_p)->hw_rwlock); \
                            btk_mutex_exit(&(u_p)->dma_mutex);

/*
** External functions
*/
extern void btk_setup(bt_unit_t *unit_p);
extern bt_error_t btk_cas(bt_unit_t *unit_p, bt_dev_t axs_type, bt_cas_t *cas_p);
extern bt_error_t btk_tas(bt_unit_t *unit_p, bt_dev_t axs_type, bt_tas_t *tas_p);
extern bt_error_t btk_ioreg(bt_unit_t *unit_p, bt_dev_t axs_type, bt_ioaccess_t *ioreg_p);

extern bt_error_t btk_thread_register(bt_unit_t *unit_p, bt_thread_reg_t *reg_p);
extern bt_error_t btk_thread_unregister(bt_unit_t *unit_p, bt_thread_reg_t *reg_p);
extern bt_error_t btk_thread_add(bt_unit_t *unit_p, bt_thread_add_t *add_p);
extern bt_error_t btk_thread_delete(bt_unit_t *unit_p, bt_thread_add_t *add_p);
extern bt_error_t btk_thread_wait(bt_unit_t *unit_p, bt_thread_wait_t *wait_p);
extern bt_error_t btk_thread_wake(bt_unit_t *unit_p, bt_thread_wait_t *wake_p);

extern bt_error_t btk_hw_xfer(bt_unit_t *unit_p, bt_dev_t ldev, int xfer_dir, bt_hw_xfer_t *hw_xfer_p);

extern bt_error_t btp_hw_bind(bt_unit_t *unit_p, bt_bind_t *bind_p, bt_dev_t axs_type);
extern bt_error_t btp_hw_unbind(bt_unit_t *unit_p, bt_bind_t *bind_p, bt_dev_t axs_type);
extern bt_error_t btp_bind(bt_unit_t *unit_p, bt_bind_t *bind_p, bt_dev_t type);
extern bt_error_t btp_unbind(bt_unit_t *unit_p, bt_bind_t *bind_p, bt_dev_t type);

extern bt_error_t btk_send_irq(bt_unit_t *unit_p, bt_data32_t vector, bool_t driver);

extern bt_error_t btk_set_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, bt_data64_t value);
extern bt_error_t btk_get_info(bt_unit_t *unit_p, bt_dev_t ldev, bt_info_t param, bt_data64_t *value_p);

extern bt_error_t btk_lcard_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);
extern bt_error_t btk_cable_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);
extern bt_error_t btk_rcard_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);
extern bt_error_t btk_pair_diag(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);
extern bt_error_t btk_driver_version(bt_unit_t *unit_p, bt_hw_diag_t *diag_p);

/*
** External varaibles
*/
extern volatile int     btk_alloc_total_g;

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_IOCTL_C);


/*****************************************************************************
**
**      Name:           btp_ioctl()
**
**      Purpose:        Device special functions
**
**      Args:
**          inode_p     Pointer to inode structure
**          file_p      Pointer to file structure
**          cmd         Which ioctl() command was given
**          args        Pointer to data structure for this command.
**                      Has to be recast on entry.
**          
**      Modifies:
**          None
**          
**      Returns:
**          <0          Error number
**          Otherwise   Number of bytes transferred
**
**      Notes:
**
*****************************************************************************/


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
int btp_ioctl(
    struct inode *inode_p,
#endif
long btp_ioctl(
    struct file *file_p,
    unsigned int raw_cmd,
    unsigned long args
    )
{
    int         retval = 0;
    bt_error_t  bt_err_ret = BT_SUCCESS;

    bt_unit_t  *unit_p = GET_UNIT_PTR(file_p);

    int         cmd = (_IOC_NR(raw_cmd) & ~BT_IO_ERET_FLAG);
    bool_t      eret_flag = (0 != (_IOC_NR(raw_cmd) & BT_IO_ERET_FLAG));
    size_t      cmd_len = _IOC_SIZE(raw_cmd);
    bool_t      do_copyin = (0 != (_IOC_DIR(raw_cmd)  & _IOC_WRITE) );
    bool_t      do_copyout = (0 != (_IOC_DIR(raw_cmd) & _IOC_READ) );

    bt_dev_t    type = GET_LDEV_TYPE(file_p);

    char        arg_buf[MAX_IOCTL_ARG];
    void        *arg_p = &arg_buf[0];

    FUNCTION("btp_ioctl");
    LOG_DEVID(file_p);
    FENTRY;

    TRC_MSG(BT_TRC_CTRL, 
         (LOG_FMT "logical dev = %d ioctl_cmd = 0x%x\n  len %d  copy %c%c\n",
         LOG_ARG, type, cmd, (int) cmd_len,
         (do_copyin) ? 'I' : ' ', (do_copyout) ? 'O' : ' '));

    if (NULL == unit_p) {
        WARN_STR("Can't access unit structure.");
        retval = -ENODEV;
        do_copyout = FALSE;
        goto ioctl_end;
    }

    /* 
    ** We are using less than the Linux limit on IOCTL arg size 
    */
    if (MAX_IOCTL_ARG < cmd_len) {
        INFO_STR(inv_parm_length_p);
        retval = -EINVAL;
        do_copyout = FALSE;
        goto ioctl_end;
    }

    /*
    ** Copy in ioctl parameter if required
    */
    BTK_BZERO(arg_p, MAX_IOCTL_ARG);

    if (do_copyin) {
        retval = copy_from_user(arg_p, (void *) args, cmd_len);
        if (retval != 0) {
            INFO_STR("Could not copy user data to kernel space.");
            retval = -EFAULT;
            do_copyout = FALSE;
            goto ioctl_end;
        }
    }

    /*
    ** Execute the ioctl command
    */
    switch (cmd) {

      /*
      **  BIOC_SETUP & BIOC_RESET
      */
      case BT_CTRL_RESET:
      case BT_CTRL_SETUP:
        TRC_STR(BT_TRC_CTRL, ((BT_CTRL_RESET == cmd) ? 
            "BIOC_RESET" : "BIOC_SETUP"));
        CHKPARM(bt_status_t);
        {
            bt_status_t           *data_p = arg_p;
            bt_status_t           device_status;
            clock_t               reset_time;
            BTK_LOCK_RETURN_T     isr_pl;

            /*
            ** Only VMEbus can be reset
            */
            if ((cmd == BT_CTRL_RESET) && 
                IS_SET(unit_p->bt_status, BT_PCI2PCI)) {
                INFO_STR("BT_CTRL_RESET not supported on PCI to PCI models");
                retval = -EINVAL;
                break;
            }

            /*
            ** Prevent PIO and DMA from occuring
            */
            LOCK_DEVICE(unit_p);

            if (cmd == BT_CTRL_RESET) {

                /* 
                ** Reset timer is in 10 mSec increments, make sure 
                ** its valid
                */
                reset_time = unit_p->reset_timer * 10000; 
                if (reset_time < 1) {
                    INFO_MSG((LOG_FMT "invalid RESET_TIMER = %d\n",
                             LOG_ARG, unit_p->reset_timer));
                    reset_time = DEFAULT_RESET_TIMER * 10000;
                }
        
                /*
                ** Make sure the interrupt handler is not executing
                ** and reset the remote bus
                */
                BTK_LOCK_ISR(unit_p, isr_pl);
                BTK_UNLOCK_ISR(unit_p, isr_pl);
                btk_put_io(unit_p, BT_REM_CMD1, RC1_RESET);

                /*
                ** Use the DMA event and timeout feature of btk_event_wait().
                */
                btk_event_wait(&unit_p->dma_event, btk_msec2tck(reset_time/1000));

            }


            /*
            ** Again lock out the interrupt handler while we do a setup
            */
            btk_mutex_enter(&(unit_p->mreg_mutex));
            BTK_LOCK_ISR(unit_p, isr_pl);
            btk_setup(unit_p);
            BTK_UNLOCK_ISR(unit_p, isr_pl);
            btk_mutex_exit(&(unit_p->mreg_mutex));

            unit_p->last_error |= btk_get_io(unit_p, BT_LOC_STATUS);
            device_status = (unit_p->last_error & LSR_CERROR_MASK) << BT_INTR_ERR_SHFT;

            if (cmd == BT_CTRL_RESET) {
                SET_BIT(device_status, BT_INTR_RRESET);
            }

            /*
            ** Release the unit lock
            */
            UNLOCK_DEVICE(unit_p);
      
            /*
            ** Return the status information
            */
            *data_p = device_status;

        }
        break;

      /*
      ** PCI Configuration register access
      */
      case BT_CTRL_CFG:
        TRC_STR(BT_TRC_CTRL, "command = BIOC_CFG");
        CHKPARM(bt_ioaccess_t);
        {
            bt_ioaccess_t         *data_p = (bt_ioaccess_t *) arg_p;
            size_t                reg_size;
            char *cfg_wr_msg_p =
                    LOG_FMT "PCI Configuration Write 0x%x Value 0x%x\n";
            char *cfg_rd_msg_p =
                    LOG_FMT "PCI Configuration Read 0x%x Value 0x%x\n";


            /*
            ** Determine the size of the register
            */
            switch(data_p->offset) {

              case PCI_VENDOR_ID:
              case PCI_DEVICE_ID:
              case PCI_COMMAND:
              case PCI_STATUS:
              case PCI_CLASS_DEVICE:
                reg_size = BT_WIDTH_D16;
                break;

              case PCI_REVISION_ID:
              case PCI_CLASS_PROG:
              case PCI_CACHE_LINE_SIZE:
              case PCI_LATENCY_TIMER:
              case PCI_HEADER_TYPE:
              case PCI_BIST:
              case PCI_INTERRUPT_LINE:
              case PCI_INTERRUPT_PIN:
              case PCI_MIN_GNT:
              case PCI_MAX_LAT:
                reg_size = BT_WIDTH_D8;
                break;

              case PCI_BASE_ADDRESS_0:
              case PCI_BASE_ADDRESS_1:
              case PCI_BASE_ADDRESS_2:
              case PCI_BASE_ADDRESS_3:
              case PCI_BASE_ADDRESS_4:
              case PCI_BASE_ADDRESS_5:
                reg_size = BT_WIDTH_D32;
                break;

              default:
                reg_size = (size_t) -1l;
                break;
            }
            if ((size_t) -1l == reg_size) {
                INFO_STR("Invalid register offset.");
                retval = -EINVAL;
                break;
            }

            /*
            **  Do the actual configuration read ro write
            */
#define ENCODE(size, op) ( ((size) & 0xf) | ((op)<<4))
            switch ENCODE(reg_size, data_p->operation) {
              case ENCODE(BT_WIDTH_D32, BT_IOREAD):
                {
                    bt_data32_t data_val;

                    retval = pci_read_config_dword(unit_p->dev_p,
                            data_p->offset, &data_val);
                    if (0 <= retval) {
                        data_p->value = data_val;
                    }
                    TRC_MSG(BT_TRC_LIO, (cfg_rd_msg_p, LOG_ARG,
                            data_p->offset, data_p->value));
                }
                break;

              case ENCODE(BT_WIDTH_D32, BT_IOWRITE):
                {
                    bt_data32_t data_val;

                    TRC_MSG(BT_TRC_LIO, (cfg_wr_msg_p, LOG_ARG,
                        data_p->offset, data_p->value));
                    data_val = data_p->value;
                    retval = pci_write_config_dword(unit_p->dev_p,
                            data_p->offset, data_val);
                }
                break;

              case ENCODE(BT_WIDTH_D16, BT_IOREAD):
                {
                    bt_data16_t data_val;

                    retval = pci_read_config_word(unit_p->dev_p,
                        data_p->offset, &data_val);
                    if (0 <= retval) {
                        data_p->value = data_val;
                    }
                    TRC_MSG(BT_TRC_LIO, (cfg_rd_msg_p, LOG_ARG,
                        data_p->offset, data_p->value));
                }
                break;

              case ENCODE(BT_WIDTH_D16, BT_IOWRITE):
                {
                    bt_data16_t data_val;

                    TRC_MSG(BT_TRC_LIO, (cfg_wr_msg_p, LOG_ARG,
                        data_p->offset, data_p->value));
                    data_val = data_p->value;
                    retval = pci_write_config_word(unit_p->dev_p,
                            data_p->offset, data_val);
                }
                break;

              case ENCODE(BT_WIDTH_D8, BT_IOREAD):
                {
                    bt_data8_t data_val;

                    retval = pci_read_config_byte(unit_p->dev_p,
                        data_p->offset, &data_val);
                    if (0 <= retval) {
                        data_p->value = data_val;
                    }
                    TRC_MSG(BT_TRC_LIO, (cfg_rd_msg_p, LOG_ARG,
                        data_p->offset, data_p->value));
                }
                break;

              case ENCODE(BT_WIDTH_D8, BT_IOWRITE):
                {
                    bt_data8_t data_val;

                    TRC_MSG(BT_TRC_LIO, (cfg_wr_msg_p, LOG_ARG,
                        data_p->offset, data_p->value));
                    data_val = data_p->value;
                    retval = pci_write_config_byte(unit_p->dev_p,
                        data_p->offset, data_val);
                }
                break;

              default:
                TRC_STR(BT_TRC_INFO, "Operation requested isn't supported.");
                retval = EINVAL;
                break;
            }
#undef  ENCODE
        }
        break;


      /*
      **  BIOC_TAS
      */
      case BT_CTRL_TAS:
        TRC_STR(BT_TRC_CTRL, "BIOC_TAS");
        CHKPARM(bt_tas_t);
        {
            bt_tas_t               *data_p = arg_p;
            
            retval = btk_tas(unit_p, type, data_p);

        }
        break;

      /*
      **  BIOC_CAS 
      */
      case BT_CTRL_CAS:
        TRC_STR(BT_TRC_CTRL, "BIOC_CAS");
        CHKPARM(bt_cas_t);
        {
            bt_cas_t              *data_p = arg_p;
      
            bt_err_ret = btk_cas(unit_p, type, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_REG
      */
      case BT_CTRL_THREAD_REG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_REG");
        CHKPARM(bt_thread_reg_t);
        {
            bt_thread_reg_t       *data_p = (bt_thread_reg_t *) arg_p;
      
            bt_err_ret = btk_thread_register(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_UNREG
      */
      case BT_CTRL_THREAD_UNREG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_UNREG");
        CHKPARM(bt_thread_reg_t);
        {
            bt_thread_reg_t       *data_p = (bt_thread_reg_t *) arg_p;
      
            bt_err_ret = btk_thread_unregister(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_ADD
      */
      case BT_CTRL_THREAD_ADD:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_ADD");
        CHKPARM(bt_thread_add_t);
        {
            bt_thread_add_t       *data_p = (bt_thread_add_t *) arg_p;
      
            bt_err_ret = btk_thread_add(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_DELETE
      */
      case BT_CTRL_THREAD_DELETE:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_DELETE");
        CHKPARM(bt_thread_add_t);
        {
            bt_thread_add_t       *data_p = (bt_thread_add_t *) arg_p;
      
            bt_err_ret = btk_thread_delete(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_WAIT
      */
      case BT_CTRL_THREAD_WAIT:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_WAIT");
        CHKPARM(bt_thread_wait_t);
        {
            bt_thread_wait_t      *data_p = (bt_thread_wait_t *) arg_p;
      
            bt_err_ret = btk_thread_wait(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_THREAD_WAKE
      */
      case BT_CTRL_THREAD_WAKE:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_THREAD_WAKE");
        CHKPARM(bt_thread_wait_t);
        {
            bt_thread_wait_t      *data_p = (bt_thread_wait_t *) arg_p;
      
            bt_err_ret = btk_thread_wake(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_HW_READ
      */
      case BT_CTRL_HW_READ:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_HW_READ");
        CHKPARM(bt_hw_xfer_t);
        {
            bt_hw_xfer_t          *data_p = (bt_hw_xfer_t *) arg_p;
      
            bt_err_ret = btk_hw_xfer(unit_p, type, BT_READ, data_p);
        }
        break;

      /*
      ** BIOC_HW_WRITE
      */
      case BT_CTRL_HW_WRITE:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_HW_WRITE");
        CHKPARM(bt_hw_xfer_t);
        {
            bt_hw_xfer_t          *data_p = (bt_hw_xfer_t *) arg_p;
      
            bt_err_ret = btk_hw_xfer(unit_p, type, BT_WRITE, data_p);
        }
        break;

      /*
      ** BIOC_HW_BIND
      */
      case BT_CTRL_HW_BIND:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_HW_BIND");
        CHKPARM(bt_bind_t);
        {
            bt_bind_t             *data_p = (bt_bind_t *) arg_p;
      
            bt_err_ret = btp_hw_bind(unit_p, data_p, type);
        }
        break;

      /*
      ** BIOC_HW_UNBIND
      */
      case BT_CTRL_HW_UNBIND:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_HW_UNBIND");
        CHKPARM(bt_bind_t);
        {
            bt_bind_t             *data_p = (bt_bind_t *) arg_p;
      
            bt_err_ret = btp_hw_unbind(unit_p, data_p, type);
        }
        break;

      /*
      ** BIOC_IOREG
      */
      case BT_CTRL_IOREG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_IOREG");
        {
        
            bt_ioaccess_t *data_p = (bt_ioaccess_t *) arg_p;
            CHKPARM(bt_ioaccess_t);

            bt_err_ret = btk_ioreg(unit_p, type, data_p);
        }
        break;

      /*
      **  BIOC_PARAM
      */
      case BT_CTRL_PARAM:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_PARAM");
        {
            bt_param_t *data_p = arg_p;

        CHKPARM(bt_param_t);

        /* D64 PIO doesn't work properly on a 32 bit PCI bus, so we       */
        /* change BT_WIDTH_ANY to BT_WIDTH_D32 for data_size[type!=LM].   */
        /* D64 DMA works properly on a 32 bit bus via proper D32 accesses.*/
        if (BT_AXSLM != type
                && BT_INFO_DATAWIDTH == data_p->param
                && BT_WIDTH_ANY == data_p->value) {
            data_p->value = BT_WIDTH_D32;
        }

        data_p->error = btk_set_info(unit_p, type, (bt_info_t) data_p->param,
                                     data_p->value);
        retval = -data_p->error;
        }
        break;

      /*
      **  BIOC_DEV_ATTRIB
      */
      case BT_CTRL_DEV_ATTRIB:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_DEV_ATTRIB");
        CHKPARM(bt_param_t);
        {
            bt_param_t *data_p = arg_p;

        CHKPARM(bt_param_t);
        data_p->error = btk_get_info(unit_p, type, (bt_info_t) data_p->param,
                                     &data_p->value);
        retval = -data_p->error;
        }
        break;

      /*
      **  BIOC_STATUS & BIOC_CLR_STATUS
      */
      case BT_CTRL_STATUS:
      case BT_CTRL_CLR_STATUS:
        TRC_STR(BT_TRC_CTRL, 
            ((BT_CTRL_STATUS == cmd) ? "BIOC_STATUS" : "BIOC_CLR_STATUS") );
        CHKPARM(bt_status_t);
        {
            bt_status_t     result_flags;
            bt_status_t     *data_p = arg_p;

            /*
            ** Clear any hardware errors and clear the software error 
            ** variables
            */
            if (cmd == BT_CTRL_CLR_STATUS) {
                unit_p->last_error = 0;
            }
            (void) btk_get_io(unit_p, BT_REM_STATUS);

            /*
            ** Get the current error status bits.  Read the register
            ** just in case the interrupts were shut off somehow.
            */
            unit_p->last_error |= btk_get_io(unit_p, BT_LOC_STATUS);
            result_flags = (unit_p->last_error & LSR_CERROR_MASK) << BT_INTR_ERR_SHFT;
            *data_p = result_flags;
        }
        break;

      /*
      **  BIOC_SND_INTR
      */
      case BT_CTRL_SND_INTR:
        TRC_STR(BT_TRC_CTRL, "BIOC_SND_INTR");
        {
        NOPARM;
        btk_rwlock_rd_enter(&unit_p->hw_rwlock);
        retval = btk_send_irq(unit_p, 0, FALSE);
        btk_rwlock_rd_exit(&unit_p->hw_rwlock);
        }
        break;

      /*
      ** BIOC_SLOCK
      */
      case BT_CTRL_SLOCK:
        TRC_STR(BT_TRC_CTRL, "command = BIOC_LOCK");
        /*
        ** Does not do anything, SGM 2/10/00
        ** Management Reviewed and O.K.ed
        */
        break;

      /*
      ** BIOC_SUNLOCK
      */
      case BT_CTRL_SUNLOCK:
        TRC_STR(BT_TRC_CTRL, "command = BIOC_UNLOCK");
        NOPARM;
        /*
        ** Does not do anything, SGM 2/10/00
        ** Management Reviewed and O.K.ed
        */
        break;

      /*
      ** BIOC_BIND
      */
      case BT_CTRL_BIND:
        TRC_STR(BT_TRC_CTRL, "BIOC_BIND");
        CHKPARM(bt_bind_t);

        bt_err_ret = btp_bind(unit_p, (bt_bind_t *) arg_p, type);
        break;
        
      /*
      ** BIOC_UNBIND
      */
      case BT_CTRL_UNBIND:
        TRC_STR(BT_TRC_CTRL, "BIOC_UNBIND");
        CHKPARM(bt_bind_t);

        bt_err_ret = btp_unbind(unit_p, (bt_bind_t *) arg_p, type);
        break;

      /*
      ** BIOC_D_LCARD_DIAG
      */
      case BT_CTRL_LCARD_DIAG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_LCARD_DIAG");
        CHKPARM(bt_hw_diag_t);
        {
        bt_hw_diag_t              *data_p = arg_p;
        
        bt_err_ret = btk_lcard_diag(unit_p, data_p);
        }
        break;


      /*
      ** BIOC_D_CABLE_DIAG
      */
      case BT_CTRL_CABLE_DIAG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_CABLE_DIAG");
        CHKPARM(bt_hw_diag_t);
        {
        bt_hw_diag_t              *data_p = arg_p;
        
        bt_err_ret = btk_cable_diag(unit_p, data_p);
        }
        break;


      /*
      ** BIOC_D_RCARD_DIAG
      */
      case BT_CTRL_RCARD_DIAG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_RCARD_DIAG");
        CHKPARM(bt_hw_diag_t);
        {
        bt_hw_diag_t              *data_p = arg_p;
        
        bt_err_ret = btk_rcard_diag(unit_p, data_p);
        }
        break;


      /*
      ** BIOC_D_PAIR_DIAG
      */
      case BT_CTRL_PAIR_DIAG:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_PAIR_DIAG");
        CHKPARM(bt_hw_diag_t);
        {
        bt_hw_diag_t              *data_p = arg_p;
        
        bt_err_ret = btk_pair_diag(unit_p, data_p);
        }
        break;

      /*
      ** BIOC_D_DRIVER_VERSION
      */
      case BT_CTRL_DRIVER_VERSION:
        TRC_STR(BT_TRC_CTRL, "Command = BIOC_DRIVER_VERSION");
        CHKPARM(bt_hw_diag_t);
        {
        bt_hw_diag_t              *data_p = arg_p;
      
        bt_err_ret = btk_driver_version(unit_p, data_p);
        }
        break;

      /*
      **  Catch all invalid ioctls
      */
      default:
        INFO_STR("Invalid ioctl command");
        retval = -EINVAL;
        do_copyout = FALSE; /* Prevent buffer from being copied back */
        break;
    }

    /* 
    ** If no errors, put our BT_EXXXX value in the structure 
    */
    if ((retval >= 0) && (eret_flag)) {
        /* If no errors, put our BT_EXXXX value in the structure */
        *((bt_data32_t *) arg_p) = bt_err_ret;
    }

ioctl_end:
    if (do_copyout) {
        int temp_ret;

        temp_ret = copy_to_user((void *) args, arg_p, cmd_len);
        if (temp_ret != 0) {
            INFO_STR("Could not copy data to user space.");
            if (0 == retval) {
                retval = -EFAULT;
            }
        }
    }

    FEXIT(retval);
    return retval;
}
