/****************************************************************************
**
**      Filename:   btp_bind.c
**
**      Purpose:    Functions supporting BIOC_BIND and BIOC_UNBIND.
**                  for the Model 946 Support Software.
**
**      $Revision: 2330 $
**
****************************************************************************/
/****************************************************************************
**
**        Copyrighy (c) 1999-2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 2330 $ ";
#endif  /* LINT */

#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/pagemap.h>      /* Needed for page_address() */
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include "btdd.h"

/*
**  Imported function prototypes
*/
extern bool_t btk_invalid_op(bt_unit_t *unit_p, bt_dev_t type, 
             bt_operation_t op);

/*
**  Local private function prototypes
*/
static int  bind_search(void *qcurr_p, void *comp_p);
static int  bind_release(void *qcurr_p, void *comp_p);

/*
**  Given local PCI addr & length => number of mreg bits required
*/
#define CALC_LEN_BITS(addr,length)	(((length) / BT_617_PAGE_SIZE) + \
   ((((bt_devaddr_t) (addr) % BT_617_PAGE_SIZE) + ((length) % BT_617_PAGE_SIZE)) / BT_617_PAGE_SIZE) + \
  (((((bt_devaddr_t) (addr) % BT_617_PAGE_SIZE) + ((length) % BT_617_PAGE_SIZE)) % BT_617_PAGE_SIZE) ? 1 : 0))

/*
**  Structure to search bind requests for
*/
typedef struct {
    int         phys_len;
    int         phys_off;
    bt_dev_t    axs_type;
} search_t;

/*
**  Static variables
*/
BT_FILE_NUMBER(TRACE_BTP_BIND_C);


/****************************************************************************
**
**      Function:   btp_bind()
**
**      Purpose:    Implement BIOC_BIND functions.
**
**      Args:       unit_p  pointer to unit struct
**                  bind_p  pointer to bt_bind_t user parameters
**                  type    logical device type
**
**      Returns:    BT_SUCCESS      Success.
**                  Otherwise       Value indicating failure type.
**
****************************************************************************/
bt_error_t btp_bind(
    bt_unit_t *unit_p, 
    bt_bind_t *bind_p, 
    bt_dev_t type)
{
    FUNCTION("btp_bind");
    LOG_UNIT(unit_p->unit_number);

    caddr_t             addr;       /* address of kernel allocated buffer */
    unsigned int        inx;                                    /* index */
    int                     len;        /* length of kernel allocated buffer */
    unsigned int        start, need;
    bt_error_t          retval = BT_SUCCESS;                    /* set retval for success initially */
    bt_dev_t            axs_type = BT_AXSTYP(type);             /* set logical device type */
    bt_data32_t         mreg_value;                             /* memory mapped register value */
    bt_btrack_t         *new_bind_p;                            /* track binding pointer */
    btk_llist_elem_t    *element_p;                             /* binding element list */
    unsigned int nr_pages;
    struct page **pages;
    int ret, i;
    bt_data32_t usr_curr_offset;
    unsigned long pci_addr;

    FENTRY;

    /*
    ** Initialize bind structure in case of fail
    */
    if (bind_p == (bt_bind_t *) NULL) {
        INFO_STR("Invalid bind information structure");
        retval = BT_EIO;
        goto exit_bind;
    }
    bind_p->sysinfo_p = (bt_devaddr_t) NULL;
    addr = (caddr_t) (unsigned long) (bind_p->host_addr);
    len = (int) bind_p->length;

    TRC_MSG((BT_TRC_BIND), 
           (LOG_FMT "user buffer %p; len 0x%x; type 0x%x %s wait\n",
           LOG_ARG, 
           addr, len, (unsigned int) type, ((bind_p->nowait) ? "no ":"")));

    /* 
    ** Verify the logical device supports this operation 
    */
    if(btk_invalid_op(unit_p, axs_type, BT_OP_BIND)) {
        INFO_STR("Binds are not supported for this logical device\n");
        retval = BT_EACCESS;
        goto exit_bind;
    }

    /* 
    ** Perform all necessary parameter validation here
    */
    if (len == 0) {
        INFO_STR("User buffer length zero")
        retval = BT_EINVAL;
        goto exit_bind;
    }

    /* 
    ** The user buffer must be aligned on a adapter page size
    */
    if ((bt_devaddr_t) addr & (BT_617_PAGE_SIZE - 1)) {
        INFO_MSG((LOG_FMT "Buffer must be aligned on a %d byte boundary\n",
                  LOG_ARG, BT_617_PAGE_SIZE));
        retval = BT_EINVAL;
        goto exit_bind;
    }

    /*
    ** The user buffer must have a length modulo the adapter page size 
    */
    if (len & (BT_617_PAGE_SIZE - 1)) {
        INFO_MSG((LOG_FMT "Buffer length must be a multipe of %d\n",
                LOG_ARG, BT_617_PAGE_SIZE));
        retval = BT_EINVAL;
        goto exit_bind;
    }

    /*
    ** Malloc a scatter/gather list
    */
    usr_curr_offset = (bt_data32_t) ((unsigned long) addr & (PAGE_SIZE -1));
    nr_pages = ((len + usr_curr_offset) / PAGE_SIZE) + 1;
    pages = kmalloc(nr_pages * sizeof(struct page *), GFP_KERNEL);
    if (!pages) {
        WARN_STR("Failed to kmalloc scatter/gather list.\n");
        retval = BT_ENOMEM;
        goto exit_bind;
    }

    /* 
    ** Allocate a new bind requested list item
    */
    element_p = btk_llist_elem_create(sizeof(bt_btrack_t), 0);
    if (element_p == (btk_llist_elem_t *) NULL) {
        INFO_STR("Not enough resources to track bind request");
        kfree(pages);
        retval = BT_ENOMEM;
        goto exit_bind;
    }
    new_bind_p = (bt_btrack_t *) btk_llist_elem_data(element_p);

    /*
    ** Either find a set of open mapping regs large enough 
    ** for the request or try to get the one specified by the user
    */
    need = (int) CALC_LEN_BITS(addr, len);
    btk_mutex_enter(&(unit_p->mreg_mutex));
    if (bind_p->phys_addr == BT_BIND_NO_CARE) {
      retval = btk_bit_alloc(unit_p->sdma_aval_p, need, 1, &start);
    } else {
      start = bind_p->phys_addr / BT_PAGE_SIZE;
      retval = btk_bit_specify(unit_p->sdma_aval_p, start, need);
    }
    btk_mutex_exit(&(unit_p->mreg_mutex));
    if (retval != BT_SUCCESS) {
        INFO_STR("Not enough resources to bind buffer");
        btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t));
        kfree(pages);
        retval = BT_ENOMEM;
        goto exit_bind;
    }
#define BTP_FREE_MREG \
    { \
        int i; \
        btk_mutex_enter(&(unit_p->mreg_mutex)); \
        (void) btk_bit_free(unit_p->sdma_aval_p, start, need); \
        btk_mutex_exit(&(unit_p->mreg_mutex)); \
        btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t)); \
        for (i = 0; i < nr_pages; i++) { \
            set_page_dirty_lock(pages[i]); \
            page_cache_release(pages[i]); \
        } \
        kfree(pages); \
    }

    /*
     **  Setup PCI address, and function code
    */
    mreg_value = 0;
    btk_setup_mreg(unit_p, axs_type, &mreg_value, BT_OP_BIND);

    TRC_MSG((BT_TRC_BIND | BT_TRC_DETAIL), 
           (LOG_FMT "Binding: phy addr=%p start=0x%x; need=0x%x\n",
           LOG_ARG, addr, start, need));

    /*
    ** Translate the user pages to physical addresses
    ** store in scatter/gather list
    */
    down_read(&current->mm->mmap_sem);
#if LINUX_VERSION_CODE < LINUX_VERSION(4,9,0)    
    ret = get_user_pages(current, current->mm, (unsigned long) addr,
        nr_pages, 1, 0, pages, NULL);
#else
    ret = get_user_pages(current, current->mm, (unsigned long) addr,
    nr_pages,FOLL_WRITE , pages, NULL);
#endif
    up_read(&current->mm->mmap_sem);
    if (ret < nr_pages) {
        WARN_STR("Failed to kmalloc scatter/gather list.\n");
        btk_mutex_enter(&(unit_p->mreg_mutex));
        (void) btk_bit_free(unit_p->sdma_aval_p, start, need);
        btk_mutex_exit(&(unit_p->mreg_mutex));
        btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t));
        for (i = 0; i < ret; i++) {
            put_page(pages[i]);
        }
        kfree(pages);
        retval = BT_EIO;
        goto exit_bind;
    }

    /*
    ** Load the mapping registers per 4k kernel buffer range
    */
    for (inx = start, i = 0; inx < (start + need); inx++, i++) {

        pci_addr = (unsigned long) page_to_phys(pages[i]);
        if (0 == pci_addr) {
            WARN_STR("Kernel to PCI address translation failed.\n");
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
            BTP_FREE_MREG;
            retval = BT_EIO;
            goto exit_bind;
        }

        mreg_value &= ~BT_MREG_ADDR_MASK;
        mreg_value |= (pci_addr & BT_MREG_ADDR_MASK);

        btk_put_mreg(unit_p, inx, BT_LMREG_CABLE_2_PCI, mreg_value);
        if ( (btk_get_mreg(unit_p, inx, BT_LMREG_CABLE_2_PCI)) != mreg_value ) {
            WARN_MSG((LOG_FMT "Verify Write BT_LMREG_CABLE_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                             LOG_ARG, inx));
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
            BTP_FREE_MREG;
            retval = BT_EIO;
            goto exit_bind;
        }

        btk_put_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI, mreg_value);
        if ( (btk_get_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI)) != mreg_value ) {
            WARN_MSG((LOG_FMT "Verify Write BT_LMREG_DMA_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                             LOG_ARG, inx));
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
            btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
            BTP_FREE_MREG;
            retval = BT_EIO;
            goto exit_bind;
        }

    }

    /* 
    ** Fill in the slave DMA tracking fields
    */
    new_bind_p->start_mreg = start;
    new_bind_p->num_mreg = need;
    new_bind_p->phys_off = start * BT_PAGE_SIZE;
    new_bind_p->phys_len = len;
    new_bind_p->bind_addr = addr;
    new_bind_p->axs_type = axs_type;
    new_bind_p->page_info = pages;
    new_bind_p->nr_pages = nr_pages;

    TRC_MSG(BT_TRC_BIND, 
           (LOG_FMT "BOUND: user addr %p; offset 0x%x; len 0x%x; start %d, need %d\n",
           LOG_ARG, addr, new_bind_p->phys_off, len, 
           start, need));

    /*
    ** Record the information for unbind
    ** Note clink_un/lock are not used since the interrpt routine
    ** does not touch the queue.
    */
    btk_mutex_enter(&unit_p->llist_bind_mutex);
    btk_llist_insert_first(&unit_p->qh_bind_requests, element_p);
    btk_mutex_exit(&unit_p->llist_bind_mutex);

    /* 
    ** Fill out bind structure to be returned to user
    */
    bind_p->phys_addr = new_bind_p->phys_off;
                        /* start * BT_PAGE_SIZE; - old code */
    bind_p->sysinfo_p = (bt_devaddr_t) new_bind_p;
#undef BTP_FREE_MREG

exit_bind:
    FEXIT(retval);
    return(retval);
}

/****************************************************************************
**
**      Function:   btp_unbind()
**
**      Purpose:    Implement BIOC_UNBIND functions.
**
**      Args:       unit_p  pointer to unit struct
**                  bind_p  pointer to bt_bind_t user parameters
**                  type    logical device type
**
**      Returns:    BT_SUCC         Success.
**                  Otherwise       Value indicating failure type.
**
****************************************************************************/

bt_error_t btp_unbind(
    bt_unit_t *unit_p, 
    bt_bind_t *bind_p, 
    bt_dev_t type)
{
    FUNCTION("btp_unbind");
    LOG_UNIT(unit_p->unit_number);
    
    bt_error_t          retval = BT_SUCCESS;
    unsigned int        start, need, nr_pages;
    search_t            search_val;
    bt_btrack_t         *found_p = (bt_btrack_t *) NULL; 
    btk_llist_elem_t    *element_p;
    struct page         **pages;
    int                 i;

    FENTRY;

    /* 
    **  Perform limited sanity checking on the buffer being unbound
    */
    found_p = (bt_btrack_t *) (unsigned long) (bind_p->sysinfo_p); 
    if (found_p == (bt_btrack_t *) NULL) {
        INFO_STR("Buffer's system information is corrupted");
        retval = BT_EINVAL;
        goto exit_unbind;
    }
      
    TRC_MSG(BT_TRC_BIND, 
           (LOG_FMT "UNBIND: off 0x%x; start %d; need %d\n",
           LOG_ARG, found_p->phys_off, found_p->start_mreg, 
           found_p->num_mreg));

    /* 
    **  Find the queue item corresponding to this unbind request
    **  Note clink_un/lock are not used since the interrupt routine
    **  does not touch these lists.
    */
    search_val.phys_off = found_p->phys_off;
    search_val.phys_len = found_p->phys_len;
    search_val.axs_type = found_p->axs_type;
    btk_mutex_enter(&unit_p->llist_bind_mutex);
    element_p = btk_llist_find_first(&unit_p->qh_bind_requests, 
                                     bind_search,
                                     (void *) &search_val);
    if (element_p == (btk_llist_elem_t *) NULL) {
        INFO_STR("Failled to find bind tracking info");
        btk_mutex_exit(&unit_p->llist_bind_mutex);
        retval = BT_EINVAL;
        goto exit_unbind;
    } else {
        btk_llist_remove(element_p);
        btk_mutex_exit(&unit_p->llist_bind_mutex);
        found_p = btk_llist_elem_data(element_p);
        start = found_p->start_mreg;
        need = found_p->num_mreg;
        pages = found_p->page_info;
        nr_pages = found_p->nr_pages;
    }

    /*
    ** Invalidate mapping registers
    */
    btk_put_mreg_range(unit_p, start, need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
    btk_put_mreg_range(unit_p, start, need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
    btk_mutex_enter(&(unit_p->mreg_mutex));
    (void) btk_bit_free(unit_p->sdma_aval_p, start, need);
    btk_mutex_exit(&(unit_p->mreg_mutex));

    /*
    ** Flush pages, mark as dirty and release scatter/gather pages
    */
    for (i = 0; i < nr_pages; i++) {
        set_page_dirty_lock(pages[i]);
        page_cache_release(pages[i]);
    }
    kfree(pages);

    /*
    ** Get rid of the queue element
    */
    btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t));
   
    /*
    **  Finish by invalidating the buffer we have since unbound,
    **  waking sleeping processes to race for buffer acquisition,
    **  then unlocking the device for use by others.
    */
    bind_p->phys_addr = 0;
    bind_p->sysinfo_p = (bt_devaddr_t) NULL;

exit_unbind:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btp_hw_bind()
**
**      Purpose:        Performs hardware binds.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      bind_p          Pointer to BIND structure
**                      type            logical device type
**
**      Modifies:       Void
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/
bt_error_t btp_hw_bind(
    bt_unit_t *unit_p,
    bt_bind_t *bind_p,
    bt_dev_t axs_type)

{
    bt_error_t              retval = BT_SUCCESS;
    unsigned                start, need;
    bt_btrack_t             *new_bind_p;
    btk_llist_elem_t        *element_p;
    bt_data32_t             mreg_value;
    
    FUNCTION("btp_hw_bind");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;
    
    TRC_MSG(BT_TRC_BIND, 
            (LOG_FMT "HW_BIND: bus addr 0x%llx; len 0x%x\n",
            LOG_ARG, (long long unsigned) bind_p->host_addr, bind_p->length));

    /*
    ** Initialize bind structure in case of fail
    */
    bind_p->sysinfo_p = (bt_devaddr_t) NULL;

    /* 
    ** Perform all necessary parameter validation here
    */
    if (bind_p->length == 0) {
        INFO_STR("Length zero")
        retval = BT_EINVAL;
        goto btk_hw_bind;
    }

    /*
    ** If user is trying to specify a bind offset make sure it is 
    ** less then max bind allowed
    */
    if ((bind_p->phys_addr != BT_BIND_NO_CARE) &&
        ((bind_p->phys_addr >= BT_MAX_SDMA_LEN) ||
         ((bind_p->phys_addr & (BT_PAGE_SIZE - 1)) != 0))) {
        INFO_STR("Invalid bind offset specified, not page aligned or less than BT_INFO_BIND_SIZE");
        retval = BT_EINVAL;
        goto btk_hw_bind;
    }

    /* 
    **  The hardware address must be aligned on a adapter page boundary
    */
    if (bind_p->host_addr & (BT_617_PAGE_SIZE - 1)) {
        INFO_MSG((LOG_FMT "Hardware address must be aligned on a %d byte boundary\n",
                  LOG_ARG, BT_617_PAGE_SIZE));
        retval = BT_EINVAL;
        goto btk_hw_bind;
    }

    /*
    ** The hardware address must have a length modulo the adapter page size 
    */
    if (bind_p->length & (BT_617_PAGE_SIZE - 1)) {
        INFO_MSG((LOG_FMT "Bind length must be a multipe of %d\n",
                  LOG_ARG, BT_617_PAGE_SIZE));
        retval = BT_EINVAL;
        goto btk_hw_bind;
    }

    /*
    ** Either find a set of open mapping regs large enough 
    ** for the request or try to get the one specified by the user
    */
    need = (int) CALC_LEN_BITS(bind_p->host_addr, bind_p->length);
    btk_mutex_enter(&(unit_p->mreg_mutex));
    if (bind_p->phys_addr == BT_BIND_NO_CARE) {
        retval = btk_bit_alloc(unit_p->sdma_aval_p, need, 1, &start);
    } else {
        start = bind_p->phys_addr / BT_PAGE_SIZE;
        retval = btk_bit_specify(unit_p->sdma_aval_p, start, need);
    }
    btk_mutex_exit(&(unit_p->mreg_mutex));
    if (retval != BT_SUCCESS) {
        INFO_STR("Not enough resources to bind buffer");
        goto btk_hw_bind;
    }
    
    /*
    **  Setup vme address, address modifier, and function code
    */
    mreg_value = (bt_data32_t) ((bt_devaddr_t) bind_p->host_addr & BT_MREG_ADDR_MASK);
    btk_setup_mreg(unit_p, axs_type, &mreg_value, BT_OP_BIND);

    /*
    ** Load the mapping registers
    */
    {
        unsigned int inx;
        for (inx = start; inx < (start + need); inx++, mreg_value += BT_PAGE_SIZE ) {

            btk_put_mreg(unit_p, inx, BT_LMREG_CABLE_2_PCI, mreg_value);
            if ( (btk_get_mreg(unit_p, inx, BT_LMREG_CABLE_2_PCI)) != mreg_value ) {
                WARN_MSG((LOG_FMT "Verify Write BT_LMREG_CABLE_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                                 LOG_ARG, inx));
                btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
                btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
                btk_mutex_enter(&unit_p->mreg_mutex);
                btk_bit_free(unit_p->sdma_aval_p, start, need);
                btk_mutex_exit(&unit_p->mreg_mutex);
                retval = BT_EIO;
                goto btk_hw_bind;
            }

            btk_put_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI, mreg_value);
            if ( (btk_get_mreg(unit_p, inx, BT_LMREG_DMA_2_PCI)) != mreg_value ) {
                WARN_MSG((LOG_FMT "Verify Write BT_LMREG_DMA_2_PCI mapping register mr_idx = 0x%.1X failed.\n",
                                                                                 LOG_ARG, inx));
                btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
                btk_put_mreg_range(unit_p, start, inx+1-start, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
                btk_mutex_enter(&unit_p->mreg_mutex);
                btk_bit_free(unit_p->sdma_aval_p, start, need);
                btk_mutex_exit(&unit_p->mreg_mutex);
                retval = BT_EIO;
                goto btk_hw_bind;
            }

        }
    }

    /* 
    ** Allocate a new bind requested list item
    */
    element_p = btk_llist_elem_create(sizeof(bt_btrack_t), 0);
    if (element_p == (btk_llist_elem_t *) NULL) {
        INFO_STR("Not enough resources to track bind request");
        btk_put_mreg_range(unit_p, start, need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
        btk_put_mreg_range(unit_p, start, need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
        btk_mutex_enter(&(unit_p->mreg_mutex)); \
        btk_bit_free(unit_p->sdma_aval_p, start, need); \
        btk_mutex_exit(&(unit_p->mreg_mutex));
        retval = BT_ENOMEM;
        goto btk_hw_bind;
    }
    new_bind_p = (bt_btrack_t *) btk_llist_elem_data(element_p);

    /* 
    ** Initialize queue item for slave dma tracking
    */
    new_bind_p->start_mreg = start;
    new_bind_p->num_mreg = need;
    new_bind_p->phys_off = new_bind_p->start_mreg * BT_PAGE_SIZE;
    new_bind_p->phys_len = bind_p->length;
    new_bind_p->bind_addr = NULL;
    new_bind_p->axs_type = axs_type;

    TRC_MSG(BT_TRC_BIND, 
           (LOG_FMT "HW_BIND: bus addr 0x%llx; off 0x%x; len 0x%x\n",
           LOG_ARG, (long long unsigned)bind_p->host_addr, 
		   new_bind_p->phys_off, new_bind_p->phys_len));
    TRC_MSG(BT_TRC_BIND, 
           (LOG_FMT "BIND: start %d; need %d\n", LOG_ARG, start, need));

    /*
    **  Record the information for unbind
    **  Note clink_un/lock are not used since the interrpt routine
    **  does not touch the queue.
    */
    btk_mutex_enter(&unit_p->llist_mutex);
    btk_llist_insert_first(&unit_p->qh_bind_requests, element_p);
    btk_mutex_exit(&unit_p->llist_mutex);

    /* 
    **  Fill out bind structure to be returned to user
    */
    bind_p->phys_addr = new_bind_p->phys_off;
    bind_p->sysinfo_p = (bt_devaddr_t) new_bind_p;

btk_hw_bind:
    FEXIT(retval);
    return(retval);
}

/******************************************************************************
**
**      Function:       btp_hw_unbind()
**
**      Purpose:        Performs hardware binds.
**
**      Args:
**                      unit_p          Pointer to unit structure.
**                      bind_p          Pointer to BIND structure
**                      axs_type        Device to perform UNBIND on
**
**      Modifies:       Void
**
**      Returns:        Void
**
**      Notes:          None.
**
******************************************************************************/
bt_error_t btp_hw_unbind(
    bt_unit_t *unit_p,
    bt_bind_t *bind_p,
    bt_dev_t axs_type)

{
    bt_error_t              retval = BT_SUCCESS;
    unsigned int            start, need;
    search_t                search_val;
    btk_llist_elem_t        *element_p; 
    bt_btrack_t             *found_p = (bt_btrack_t *) NULL; 
    
    FUNCTION("btp_hw_unbind");
    LOG_UNIT(unit_p->unit_number);
    
    FENTRY;

    /* 
    **  Perform limited sanity checking on the buffer being unbound
    */
    found_p = (bt_btrack_t *) (unsigned long) (bind_p->sysinfo_p); 
    if (found_p == (bt_btrack_t *) NULL) {
      INFO_STR("Buffer's system information is corrupted");
      retval = BT_EINVAL;
      goto btk_hw_unbind;
    }
    
    TRC_MSG(BT_TRC_BIND, 
           (LOG_FMT "HW_UNBIND: off 0x%x; start %d; need %d\n",
           LOG_ARG, found_p->phys_off, found_p->start_mreg, 
           found_p->num_mreg));

    /* 
    **  Find the queue item corresponding to this unbind request
    **  Note clink_un/lock are not used since the interrupt routine
    **  does not touch these lists. A unit lock is used instead.
    */
    search_val.phys_off = found_p->phys_off;
    search_val.phys_len = found_p->phys_len;
    btk_mutex_enter(&unit_p->llist_mutex);
    element_p = btk_llist_find_first(&unit_p->qh_bind_requests, bind_search,
                                     (void *) &search_val);
    if (element_p == (btk_llist_elem_t *) NULL) {
      INFO_STR("Failled to find bind tracking info");
      btk_mutex_exit(&unit_p->llist_mutex);
      retval = BT_EINVAL;
      goto btk_hw_unbind;
    } else {
      btk_llist_remove(element_p);
      btk_mutex_exit(&unit_p->llist_mutex);
      found_p = (bt_btrack_t *) btk_llist_elem_data(element_p);
      start = found_p->start_mreg;
      need = found_p->num_mreg;
    }

    /*
    ** Invalidate mapping registers
    */
    btk_put_mreg_range(unit_p, start, need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
    btk_put_mreg_range(unit_p, start, need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
    btk_mutex_enter(&(unit_p->mreg_mutex));
    (void) btk_bit_free(unit_p->sdma_aval_p, start, need);
    btk_mutex_exit(&(unit_p->mreg_mutex));
    btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t));

    /*
    **  Finish by invalidating the buffer we have since unbound,
    **  waking sleeping processes to race for buffer acquisition,
    **  then unlocking the device for use by others.
    */
    bind_p->phys_addr = (bt_data32_t) NULL;
    bind_p->sysinfo_p = (bt_data64_t) NULL;

btk_hw_unbind:
    FEXIT(retval);
    return(retval);
}


/****************************************************************************
**
**      Function:   bt_unbind()
**
**      Purpose:    Do low-level cleanup to unbind outstanding buffers.
**                  Assume that if this routine is being called it's because
**                  the last process with the device open has called close()
**                  or the Sun kernel is doing it for him due to termination.
**
**                  Our intention here is to free any kernel-level resources
**                  which may have been allocated for slave DMA, but have not
**                  been released by the user application.  
**
**      Args:       unit_p  pointer to unit structure
**
**      Returns:    BT_SUCC         success
**
**      Notes:      The caller must have the unit locked.  
**
****************************************************************************/
void bt_unbind(
    bt_unit_t *unit_p,
    bt_dev_t axs_type)
{
    FUNCTION("bt_unbind");
    LOG_UNIT(unit_p->unit_number);

    unsigned int        start, need;
    bt_btrack_t         *bind_p;
    search_t            search_val;
    btk_llist_elem_t    *element_p;

    FENTRY;

    /*
    ** Loop, freeing each free sdma buffer found queued.  
    */
    btk_mutex_enter(&unit_p->llist_mutex);
    search_val.axs_type = axs_type;
    while ((element_p = btk_llist_find_first(&unit_p->qh_bind_requests, bind_release, &search_val)) != (btk_llist_elem_t *) NULL) {
        btk_llist_remove(element_p);
        bind_p = (bt_btrack_t *) btk_llist_elem_data(element_p);
        start = bind_p->start_mreg;
        need = bind_p->num_mreg;

        TRC_MSG((BT_TRC_OPEN | BT_TRC_DETAIL), 
                (LOG_FMT "found dead bind: start %d; need %d\n", 
                LOG_ARG, bind_p->start_mreg, bind_p->num_mreg));

        /*
        **  Invalidate the mapping regs and relese bit map bits
        */
        btk_put_mreg_range(unit_p, start, need, BT_LMREG_CABLE_2_PCI, BT_MREG_INVALID);
        btk_put_mreg_range(unit_p, start, need, BT_LMREG_DMA_2_PCI, BT_MREG_INVALID);
        btk_mutex_enter(&(unit_p->mreg_mutex));
        (void) btk_bit_free(unit_p->sdma_aval_p, start, need);
        btk_mutex_exit(&(unit_p->mreg_mutex));
        btk_llist_elem_destroy(element_p, sizeof(bt_btrack_t));
    }
    btk_mutex_exit(&unit_p->llist_mutex);

    FEXIT(0);
    return;
}

/******************************************************************************
**
**  Function:   bind_search()
**
**  Purpose:    Compares the current queue item to the values passed
**              with the compare item and returns 0 if it was a match.
**
**  Args:       current_p   Pointer to current data element
**              find_p      Pointer to data element we are looking for.
**
**  Returns:    0           Match.
**              Otherwise   No match.
**
**  Notes:      Should be used with btk_llist_?() routines.
**
******************************************************************************/
static int bind_search(
    void *current_p, 
    void *find_p)
{
    FUNCTION("bind_search");
    LOG_UNKNOWN_UNIT;

    bt_btrack_t   *c_p = (bt_btrack_t *) current_p;
    search_t      *m_p = (search_t *) find_p;
    int           found = 1;

    FENTRY;
    
    if ((c_p->phys_off == m_p->phys_off) &&
        (c_p->phys_len == m_p->phys_len)) { 
        found = 0;
    }

    FEXIT(found);
    return(found);
}

/******************************************************************************
**
**  Function:   bind_release()
**
**  Purpose:    Compares the current queue item to the values passed
**              with the compare item and returns 0 if it was a match.
**
**  Args:       current_p   Pointer to current data element
**              find_p      Pointer to data element we are looking for.
**
**  Returns:    0           Match.
**              Otherwise   No match.
**
**  Notes:      Should be used with btk_llist_?() routines.
**
******************************************************************************/
static int bind_release(
    void *current_p, 
    void *find_p)
{
    FUNCTION("bind_release");
    LOG_UNKNOWN_UNIT;

    bt_btrack_t   *c_p = (bt_btrack_t *) current_p;
    search_t      *m_p = (search_t *) find_p;
    int           found = 1;

    FENTRY;
    
    if (c_p->axs_type == m_p->axs_type) { 
        found = 0;
    }

    FEXIT(found);
    return(found);
}
