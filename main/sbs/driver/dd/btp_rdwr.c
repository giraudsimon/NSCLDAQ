/******************************************************************************
**
**      Filename:       btp_rdwr.c
**
**      Purpose:        Device read and write entry points for Linux driver.
**
**      Functions:      btp_llseek(), btp_read(), btp_write(), 
**                      btp_lm_rd(), btp_lm_wr()
**
**      $Revision: 2330 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 1999 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $" ;
#endif  /* LINT */

#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/pagemap.h>      /* Needed for page_address() */
#include "btdd.h"

const char *no_unit_p = "Unit not initialized.\n";
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
# define page_cache_release(a) put_page(a)
#endif
/*
**  Function prototypes of external functions used within
*/
extern void btk_dma_pio(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr_p, bt_data32_t raddr, int *xfer_length_p, int *dma_flag_p, int *data_width_p, unsigned *start_p, unsigned *need_p);
extern bt_error_t btk_pio_xfer(bt_unit_t *unit_p, bt_dev_t ldev, caddr_t laddr_p, bt_data32_t ldev_addr, int *xfer_length_p, size_t xfer_dir);
#if defined(__linux__)
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int *xfer_length, int xfer_dir, int data_width);
#else
extern bt_error_t btk_dma_xfer(bt_unit_t *unit_p, bt_dev_t ldev, bt_data32_t laddr, bt_data32_t raddr, int xfer_length, int xfer_dir, int data_width);
#endif
extern bt_error_t btk_take_drv_sema(bt_unit_t *unit_p);
extern void btk_give_drv_sema(bt_unit_t *unit_p);

/*
**  Function prototypes of static functions used within
*/
static int btp_xfer(bt_unit_t *unit_p, bt_dev_t type, bt_accessflag_t dir, void * data_p, unsigned long dest_addr, size_t length, size_t *xferred_bytes);
static ssize_t btp_lm_rd(struct file *file_p, char *data_p, size_t length, loff_t *new_fpos_p);
static ssize_t btp_lm_wr(struct file *file_p, const char *data_p, size_t length, loff_t *new_fpos_p);

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_RDWR_C);


/*****************************************************************************
**
**      Name:           btp_llseek()
**
**      Purpose:        Repositions read/write() position in device
**
**      Args:
**          file_p      Pointer to file structure
**          offset      File offset to position to
**          which       Method for positioning: SEEK_CUR, SEEK_END, SEEK_SET
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

loff_t btp_llseek(
    struct file *file_p,
    loff_t offset,
    int which
    )
{
    FUNCTION("btp_llseek");
    LOG_DEVID(file_p);
    bt_dev_t type = GET_LDEV_TYPE(file_p);

    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    loff_t retval = 0;
    loff_t f_pos, end_pos;

    FENTRY;

    /* Determine where the end of the various logical devices are */
    switch(type) {
      case BT_DEV_RDP:
        if (IS_CLR(unit_p->bt_status, BT_PCI2PCI)) {
            end_pos = 8 * SIZE_1MB;
        } else if (IS_SET(unit_p->bt_status, BT_NEXT_GEN)) {
            end_pos = 192 * SIZE_1KB;
        } else {
            end_pos = 0;
        }
        break;
      case BT_DEV_LDP:
        if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
            end_pos = 0;
        } else {
            end_pos = 192 * SIZE_1KB;
        }
        break;
      case BT_DEV_A24:
        /* 24 address bits */
        end_pos = 16 * SIZE_1MB;
        break;

      case BT_DEV_IO:
        /* 16 address bits */
        end_pos = 64 * SIZE_1KB;
        break;

      case BT_DEV_A32:
        /* 32 address bits */
        end_pos = ((loff_t) 1)<<32;
        break;

      case BT_DEV_RE:
        /* 31 address bits, only provided for compatibility */
        end_pos = ((loff_t) 1)<<31;
        break;

      case BT_DEV_LM:
        /* User determined when device was loaded */
        end_pos = unit_p->lm_size;
        if (0 == end_pos) {
            INFO_STR("Local Memory device not enabled.\n");
            retval = -ENXIO;
            goto llseek_end;
        }
        break;

      default:
        TRC_MSG(BT_TRC_RD_WR, (LOG_FMT "Unrecognized device type: %d.\n",
            LOG_ARG, type));
        retval = -ENXIO;
        goto llseek_end;
    }

    /* Determine what the new address would be */
    switch(which) {
      case SEEK_SET:
        f_pos = offset;
        break;

      case SEEK_CUR:
        f_pos = file_p->f_pos + offset;
        break;

      case SEEK_END:
        f_pos = end_pos + offset;       /* Better be a negative offset */
        break;

      default:  /* Should never happen */
        INFO_STR("Invalid SEEK type.\n");
        retval = -EINVAL;
        goto llseek_end;
    }

    if (f_pos >= end_pos) {
        INFO_STR("Seek past end of logical device.\n");
        retval = -EINVAL;
        goto llseek_end;
    }

    file_p->f_pos = f_pos;
    retval = f_pos;

llseek_end:
    FEXIT(retval);
    return retval;
}


/*****************************************************************************
**
**      Name:           btp_read()
**
**      Purpose:        Read data from the specified logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data to
**          length      Number of bytes to transfer
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

ssize_t btp_read(
    struct file *file_p,
    char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_read");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    int             ret_val;
    long            xferred_bytes = 0;
    unsigned long   dest_addr;
    size_t          amount_xferred;


    FENTRY;

    /*
    ** Handle local memory
    */
    if (type == BT_DEV_LM) {
        xferred_bytes = btp_lm_rd(file_p, data_p, length, new_fpos_p);
        goto btp_read_end;
    }

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /* 
    ** Check if this device supports reads
    */
    if ((unit_p->logstat[type] & STAT_READ) == 0) {
        INFO_STR("Logical device does not support reading");
        xferred_bytes = -ENXIO;
        goto btp_read_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Read  logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Do the transfer by either PIO or DMA 
    */
    ret_val = btp_xfer(unit_p, type, BT_RD, data_p, dest_addr, length, &amount_xferred);
    if (BT_SUCCESS == ret_val) {
        *new_fpos_p = dest_addr + amount_xferred;
        xferred_bytes = amount_xferred;
    } else {
        xferred_bytes = -ret_val;
    }

btp_read_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_write()
**
**      Purpose:        Write data to the specified logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data from
**          length      Number of bytes to transfer
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

ssize_t btp_write(
    struct file *file_p,
    const char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_write");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    int             ret_val;
    long            xferred_bytes = 0;
    unsigned long   dest_addr;
    size_t          amount_xferred;


    FENTRY;

    /*
    ** Handle local memory
    */
    if (type == BT_DEV_LM) {
        xferred_bytes = btp_lm_wr(file_p, data_p, length, new_fpos_p);
        goto btp_write_end;
    }

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /* 
    ** Check if this device supports writes
    */
    if ((unit_p->logstat[type] & STAT_WRITE) == 0) {
        INFO_STR("Logical device does not support writing");
        xferred_bytes = -ENXIO;
        goto btp_write_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Write logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Do the transfer by either PIO or DMA 
    */
    ret_val = btp_xfer(unit_p, type, BT_WR, (char *) data_p, dest_addr, length, &amount_xferred);
    if (BT_SUCCESS == ret_val) {
        *new_fpos_p = dest_addr + amount_xferred;
        xferred_bytes = amount_xferred;
    } else {
        xferred_bytes = -ret_val;
    }

btp_write_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_lm_rd()
**
**      Purpose:        Read data from the local memory logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data to
**          length      Number of bytes to transfer
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

static ssize_t btp_lm_rd(
    struct file *file_p,
    char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_lm_rd");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    long            xferred_bytes = 0; 
    unsigned long   dest_addr;
    char            *curr_buf_p;
    size_t          len;
    loff_t          f_pos = file_p->f_pos;

    FENTRY;

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /* 
    ** Check if this device supports reads
    */
    if ((unit_p->logstat[type] & STAT_READ) == 0) {
        INFO_STR("Logical device does not support reading");
        xferred_bytes = -ENXIO;
        goto btp_lm_rd_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;
    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Read  logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Copy to user from kernel memory 
    */
    len = f_pos + length > unit_p->lm_size
	   ? unit_p->lm_size - f_pos : length;
    curr_buf_p = data_p;
    do {
        if (copy_to_user(curr_buf_p, unit_p->lm_kaddr + f_pos, len)) {
            if (0 == xferred_bytes) {
                xferred_bytes = -EFAULT;
            }
            break;
        }
        
        /* 
        ** Update counts and such 
        */
        xferred_bytes += len;
        curr_buf_p += len;
        f_pos += len;
        if (f_pos >= unit_p->lm_size) {
            f_pos = 0;
        }
        if (length - xferred_bytes > unit_p->lm_size) {
            len = unit_p->lm_size;
        } else {
            len = length - xferred_bytes;
        }
        *new_fpos_p = f_pos;
    } while ((xferred_bytes > 0) && (xferred_bytes < length));

btp_lm_rd_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_lm_wr()
**
**      Purpose:        Write data to the local memory logical device.
**
**      Args:
**          file_p      Pointer to file structure
**          data_p      Pointer to data buffer in user space to move data from
**          length      Number of bytes to transfer
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

static ssize_t btp_lm_wr(
    struct file *file_p,
    const char *data_p,
    size_t length,
    loff_t *new_fpos_p
    )
{
    FUNCTION("btp_lm_wr");
    LOG_DEVID(file_p);

    bt_dev_t type = GET_LDEV_TYPE(file_p);
    bt_unit_t *unit_p = GET_UNIT_PTR(file_p);

    char            *curr_buf_p;
    size_t          len;
    loff_t          f_pos = file_p->f_pos;
    long            xferred_bytes = 0; 
    unsigned long   dest_addr;

    FENTRY;

    /*
    ** Check for valid unit 
    */
    if (NULL == unit_p) {
        WARN_STR(no_unit_p);
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /* 
    ** Check if this device is currently on-line
    */
    if ((unit_p->logstat[type] & STAT_ONLINE) == 0) {
        INFO_STR("Logical device not on-line");
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /* 
    ** Check if this device supports writes
    */
    if ((unit_p->logstat[type] & STAT_WRITE) == 0) {
        INFO_STR("Logical device does not support writing");
        xferred_bytes = -ENXIO;
        goto btp_lm_wr_end;
    }

    /*
    ** Get the device address
    */
    dest_addr = file_p->f_pos;

    TRC_MSG(BT_TRC_RD_WR, (LOG_FMT
        "Write logical device %d, transfer length %ld.\n",
        LOG_ARG, type, (long) length));

    /* 
    ** Copy to user from kernel memory 
    */
    len = f_pos + length > unit_p->lm_size
	   ? unit_p->lm_size - f_pos : length;
    curr_buf_p = (char *) data_p;
    do {
        if (copy_from_user(unit_p->lm_kaddr + f_pos, curr_buf_p, len)) {
            if (0 == xferred_bytes) {
                xferred_bytes = -EFAULT;
            }
            break;
        }
        /* Update counts and such */
        xferred_bytes += len;
        curr_buf_p += len;
        f_pos += len;
        if (f_pos >= unit_p->lm_size) {
            f_pos = 0;
        }
        if (length - xferred_bytes > unit_p->lm_size) {
            len = unit_p->lm_size;
        } else {
            len = length - xferred_bytes;
        }
        *new_fpos_p = f_pos;
    } while ((xferred_bytes > 0) && (xferred_bytes < length));


btp_lm_wr_end:
    FEXIT(xferred_bytes);
    return xferred_bytes;
}


/*****************************************************************************
**
**      Name:           btp_xfer()
**
**      Purpose:        Transfers data using either PIO or DMA between user
**                      space and the device.
**
**      Args:
**          unit_p      Device unit pointer.
**          type        Logical device type.
**          dir         Direction: BT_RD or BT_WR.
**          usr_data_p  Address in user space of original data buffer.
**          dest_addr   Address to move data to on the device.
**          length      Number of bytes to tranfer.
**          xferred_bytes_p     Pointer to location holding number of
**                      bytes transferred.
**          
**      Modifies:
**          *xfer_byte_p     Number of bytes transferred
**          
**      Returns:
**          bt_error_t retval     BT_SUCCESS or Error number
**
**      Notes:
**      Currently uses an intermediate kernel buffer to copy data in/out of.
**
*****************************************************************************/

static int btp_xfer(
    bt_unit_t *unit_p, 
    bt_dev_t type, 
    bt_accessflag_t dir, 
    void * usr_data_p, 
    unsigned long dest_addr, 
    size_t length,
    size_t *xferred_bytes_p
    )
{
    FUNCTION("btp_xfer");
    LOG_UNIT(unit_p->unit_number);

    void            *data_p;
    int             dma_flag; 
    int             data_width; 
    size_t          length_remaining = length;
    unsigned int    start, need;
    bt_data32_t     mreg_value;
    bt_error_t      retval = BT_SUCCESS;
    unsigned int    inx;
    unsigned long   pci_addr;
    bt_data32_t     ldma_addr;
    struct page     **pages;
    int ret, i,     write;
    bt_data32_t     usr_curr_offset;
    caddr_t         kbuf_p;
    unsigned int    nr_pages;

    FENTRY;

    /* 
    ** Haven't transferred any data yet 
    */
    *xferred_bytes_p = 0;       

    /*
    **  Adjust for the extended remote ram window.
    */
    if (type == BT_AXSRE) {
        dest_addr |= RE_ADJUST;
    }
    

    /*
    ** Normally you would have the while loop in the read and write routines.
    **
    ** Since both would require the same loop, I've decided to just put it
    ** in this routine instead.
    */
    while ((length_remaining > 0) && (BT_SUCCESS == retval)) {
        int  xferred_length = 0;
        int  requested_length = length_remaining;

        /*
        ** Setup direction and current offset
        */
        if (dir == BT_WR) {
            write = 1;
        } else {
            write = 0;
        }
        usr_curr_offset = (bt_data32_t) ((bt_devaddr_t) usr_data_p & (PAGE_SIZE -1));

        /*
        ** Malloc a scatter/gather list
        */
        nr_pages = (usr_curr_offset + requested_length + ~PAGE_MASK) >> PAGE_SHIFT;
        pages = kmalloc(nr_pages * sizeof(struct page *), GFP_KERNEL);
        if (!pages) {
            WARN_STR("Failed to kmalloc scatter/gather list.\n");
            retval = BT_EIO;
            goto btp_xfer_end;
        }

        /*
        ** Translate the user pages to physical addresses
        ** store in scatter/gather list
        */
        down_read(&current->mm->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        ret = get_user_pages(current, current->mm, (unsigned long) usr_data_p,
            nr_pages, write, 1, pages, NULL);
#else
	ret = get_user_pages(usr_data_p, nr_pages,
			     write ? FOLL_WRITE : 0, pages, NULL);
 #endif
        up_read(&current->mm->mmap_sem);
        if (ret < nr_pages) {
            WARN_STR("Failed to create scatter/gather list for user buffer.\n");
            for (i = 0; i < ret; i++) {
                page_cache_release(pages[i]);
            }
            kfree(pages);
            retval = BT_EIO;
            goto btp_xfer_end;
        }
        
        /*
        ** Determine whether we do DMA or PIO
        */
        btk_dma_pio(unit_p, type, (bt_devaddr_t) usr_data_p, dest_addr, &requested_length, &dma_flag, &data_width, &start, &need);
#define BTP_FREE_MREG  if (dma_flag) { \
                           btk_mutex_enter(&unit_p->mreg_mutex); \
                           (void) btk_bit_free(unit_p->sdma_aval_p, start, need); \
                           btk_mutex_exit(&unit_p->mreg_mutex); \
                           btk_mutex_exit(&unit_p->dma_mutex);  \
                       } \
                       for (i = 0; i < nr_pages; i++) { \
                           if (BT_RD == dir) { \
                               set_page_dirty_lock(pages[i]); \
                           } \
                           page_cache_release(pages[i]); \
                       } \
                       kfree(pages);
                       

        /*
        ** Can't let PIO go past one page
        */
        if (!dma_flag) {
            if ((usr_curr_offset + requested_length) > PAGE_SIZE) {
                requested_length = PAGE_SIZE - usr_curr_offset;
            }
            need = 1;
        }
        
        TRC_MSG(BT_TRC_RD_WR, 
                (LOG_FMT "Transferring %d bytes data to 0x%lx using %s.\n",
                LOG_ARG, requested_length, dest_addr, ((dma_flag) ? "DMA" : "PIO")));
            
        if (dma_flag) {

            /*
            **  Setup vme address, address modifier, and function code
            */
            mreg_value = 0;
            btk_setup_mreg(unit_p, type, &mreg_value, BT_OP_DMA);

            /*
            ** Program up mapping RAM
            */
            for (inx = start, i = 0; inx < (start + need); inx++, i++) {
                pci_addr = (unsigned long) page_to_phys(pages[i]);
                if (0 == pci_addr) {
                    WARN_STR("Kernel to PCI address translation failed.\n");
                    retval = BT_EIO;
                    goto end_xfer_loop;
                }
                mreg_value &= ~BT_MREG_ADDR_MASK;
                mreg_value |= (pci_addr & BT_MREG_ADDR_MASK);
                btk_put_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI, mreg_value);
                if ( (btk_get_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI)) != mreg_value ) {
                    WARN_MSG((LOG_FMT "Verify Write BT_LMREG_DMA_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                                 LOG_ARG, inx));
                    retval = BT_EIO;
                    goto end_xfer_loop;
                }
            }

            /*
            ** Now we need to get the DMA semaphore 
            ** Note this routines does nothing in a single driver situtation
            */
            retval = btk_take_drv_sema(unit_p);
            if (retval != BT_SUCCESS) {
                goto end_xfer_loop;
            }

            /*
            ** If old Nanobus card, we must stop PIO from occuring
            */
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                btk_rwlock_wr_enter(&unit_p->hw_rwlock);
            }
        
            /*
            ** Do the DMA
            */
            ldma_addr = (bt_data32_t) ((start * BT_PAGE_SIZE) + usr_curr_offset);
            xferred_length = requested_length;
            retval = btk_dma_xfer(unit_p, type, ldma_addr, (bt_data32_t) dest_addr, &xferred_length, (dir == BT_RD) ? BT_READ : BT_WRITE, data_width);
            if (IS_CLR(unit_p->bt_status, BT_NEXT_GEN)) {
                btk_rwlock_wr_exit(&unit_p->hw_rwlock);
            }
            btk_give_drv_sema(unit_p);

        /*
        ** Do a PIO
        */
        } else {

            /* 
            ** Perform the proper direction PIO data transfer 
            */
            kbuf_p = kmap(pages[0]);
            if (kbuf_p == NULL) {
                INFO_STR("Failed to get kernel pointer to PIO user buffer");
                retval = BT_EIO;
            } else {
                data_p = (void *) (kbuf_p + usr_curr_offset); 
                xferred_length = requested_length;
                retval = btk_pio_xfer(unit_p, type, data_p, (unsigned long) dest_addr,
                                    &xferred_length, (dir == BT_RD) ? BT_READ : BT_WRITE);
                kunmap(pages[0]);
            }
        }

end_xfer_loop:
        BTP_FREE_MREG;
        TRC_MSG(BT_TRC_RD_WR, 
                (LOG_FMT "%s transfer done 0x%x bytes transferred, retval %d\n",
                LOG_ARG, ((dma_flag) ? "DMA" : "PIO"), xferred_length, retval));

        usr_data_p = (caddr_t) usr_data_p + xferred_length;
        dest_addr += xferred_length;
        length_remaining -= xferred_length;
    }

btp_xfer_end:
    *xferred_bytes_p = length - length_remaining;

    FEXIT(retval);
    return retval;
}

