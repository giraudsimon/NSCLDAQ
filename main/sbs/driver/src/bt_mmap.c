/******************************************************************************
**
**      Filename:       bt_mmap.c
**
**      Purpose:        IRIX implementation of mmap for NanoBus hardware.       
**
**      $Revision: 1.12 $
**
******************************************************************************/
/*****************************************************************************
**
**        Copyright (c) 2000 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#ifndef LINT
static const char revcntrl[] = "@(#)"__FILE__"  $Revision: 1.12 $" __DATE__;
#endif  /* LINT */

#include <unistd.h>
#include <sys/types.h>

#if defined (BT951)
#include <sys/file.h>
#include <fcntl.h>
#endif /* BT951 */

#include <sys/mman.h>

#if defined(__sun)
#include <unistd.h>
#endif /* defined(__sun) */
 
#include "btapi.h"
#include "btpiflib.h"

#if defined(__sun)
extern int getpagesize(void);
#endif /* defined(__sun) */

#include <unistd.h>
 

/******************************************************************************
**
**  Function:   bt_mmap()
**
**  Purpose:    Provide Mirror API access to device mmap() routine.
**
**  Args:
**      btd             Device descriptor returned by the call to bt_open
**      mmap_p          Address to hold memory map pointer is returned
**      map_off         Offset from beginning of logical device
**      map_len         Number of bytes to memory map into the application's
**                      memory space.
**      flags           Special flags indicating the read/write permissions
**                      for this memory mapped section.  Flags supported:
**                         BT_RD - allows reads from the mapped location
**                         BT_WR - allows writing from the mapped location
**                         BT_RDWR - allows read/write from mapped location 
**      swapping        Type of swapping to do for memory mapped access
**
**  Returns:
**      BT_SUCCESS      Everything went swell, region mapped successfully.
**      otherwise       Error value, mapping failed.
**
**  Comments:
**
******************************************************************************/

bt_error_t bt_mmap(
    bt_desc_t btd,                 /* descriptor  */
    void **mmap_p,                 /* address to return mmap ptr in */
    bt_devaddr_t map_off,          /* logical device addres to mmap */
    size_t map_len,                /* number of bytes to mmap       */
    bt_accessflag_t flags,         /* permissions for mapped region */
    bt_swap_t swapping)            /* swapping mode */
{
    int prot_flags = 0;             /* protection flags             */
    void *memory_p = NULL;
    int ret_val = BT_SUCCESS;
    long size_of_page;              /* Not size_t */
    bt_devaddr_t req_offset;
    bt_devaddr_t req_length;
    bt_devdata_t save_swap;
   
#if defined (BT951)
    void    *map_addr = NULL;
#endif

    /*
    ** Check for bad descriptor or invalid pointer value
    */
    if (BT_DESC_BAD(btd)) {
        ret_val = BT_EDESC;
        goto bt_mmap_end;
    }
    if (BT_PTR_BAD(mmap_p, sizeof(void *))) {
        ret_val = BT_EINVAL;
        goto bt_mmap_end;
    }
        
    /* 
    ** verify R/W permissions against the open 
    */
    if ((( flags & BT_WR) && !(btd->access_flags & BT_WR))  ||
        (( flags & BT_RD) && !(btd->access_flags & BT_RD))) {
        ret_val = BT_EACCESS;
        goto bt_mmap_end;
    }

    /* 
    ** convert the mirror API access flags to LINUX mmap prot flags 
    */
    if( flags & BT_WR) {
        prot_flags |= PROT_WRITE;
    }
    if (flags & BT_WR) {
        prot_flags |= PROT_READ;
    }

    /*
    ** Align the mmap to a page boundary
    */
    size_of_page = sysconf(_SC_PAGESIZE);
    req_offset = map_off - (map_off % size_of_page);
    req_length = map_len + (map_off % size_of_page);

    /*
    ** Set special swapping if requested
    */
    if (swapping != BT_SWAP_DEFAULT) {
        ret_val = bt_get_info(btd, BT_INFO_SWAP, &save_swap);
        if (ret_val != BT_SUCCESS) {
            goto bt_mmap_end;
        }
        ret_val = bt_set_info(btd, BT_INFO_SWAP, swapping);
        if (ret_val != BT_SUCCESS) {
            goto bt_mmap_end;
        }
    }

    /*
    ** Do the mmap
    */
    memory_p = (void *) mmap( 0,
                     req_length,  /* number of bytes to map           */
                     prot_flags,  /* memory protection                */
                     MAP_SHARED,  /* MAP_SHARED needed for PROT_WRITE */
                     btd->fd,     /* file descrpitor                  */
                     req_offset); /* starting point for map           */

    /*
    ** Restore special swapping if required
    */
    if (swapping != BT_SWAP_DEFAULT) {
        ret_val = bt_set_info(btd, BT_INFO_SWAP, save_swap);
        if (ret_val != BT_SUCCESS) {
            munmap(memory_p, req_length);
            goto bt_mmap_end;
        }
    }

    /*
    ** Adjust the mmap back to the original mmap request 
    */
    if (memory_p == MAP_FAILED) {
#if defined(BT965)
        ret_val = (bt_error_t) oserror();
#else /* BT965 */
        ret_val = (bt_error_t) errno;
#endif /* BT965 */
    } else {
#if defined (BT951)
    /* We have an address the driver wants us to use, but first it needs to 
     * be mapped into this virtual user space.
     */
        /*
         * Open the memory device
         */
        btd->dev_mem_fd = open("/dev/mem", O_RDWR);
        if (btd->dev_mem_fd < 0) {
            DBG_MSG ((stderr, "Unable to open /dev/mem device. fd = %d errno = %d\n", btd->dev_mem_fd, errno));
            /* Unmap the address and close the device*/
            (void)bt_unmmap(btd, &memory_p, req_length);
            (void)bt_close(btd);
            return errno;
        }

        /* Map the resource */
        map_addr = (bt_data32_t*)mmap(0, req_length, 
                    (PROT_READ | PROT_WRITE | PROT_UNCACHE),
                    (MAP_SHARED),
                    btd->dev_mem_fd,
                    (off_t)memory_p);
        if (map_addr == MAP_FAILED) {
            DBG_MSG ((stderr, "Unable to map fd (%d) with the address (%08Xh), errno = %d\n", btd->dev_mem_fd, (unsigned int)memory_p, errno));
            /* Unmap the address and close the device*/
            (void) bt_unmmap(btd, &memory_p, req_length);
            (void)bt_close(btd);
            close(btd->dev_mem_fd);
            return errno;
        }


        map_addr = (bt_data8_t *) map_addr + (map_off % size_of_page);
        *mmap_p = (void *) map_addr;

        /* Save the address from the driver and the kernel virtual address */
        btd->driver_phys_addr = (unsigned int)memory_p;
        btd->map_virt_addr    = (unsigned int)map_addr;

#else /* BT951 */

        memory_p = (bt_data8_t *) memory_p + (map_off % size_of_page);
        *mmap_p = (void *) memory_p;

#endif /* BT951 */
    }


bt_mmap_end:
    return ret_val;
}


/******************************************************************************
**
**  Function:   bt_unmmap()
**
**  Purpose:    Removes memory mapped access
**
**  Args:
**      btd             Device descriptor obtained from bt_open
**      mmap_p          Original memory map pointer returned by bt_mmap()
**      map_len         Number of bytes requested in origional bt_mmap() call
**
**  Returns:
**      BT_SUCCESS      Everything went swell
**      otherwise       Error value
**
**  Comments:
**
******************************************************************************/

bt_error_t bt_unmmap(
    bt_desc_t btd,                  /* descriptor from bt_open     */
    void *mmap_p,                   /* pointer from bt_mmap to unmap */
    size_t map_len                  /* number of bytes requested in map call */
    )
{
    int ret_val = BT_SUCCESS;
    long size_of_page;
    bt_devaddr_t req_length;
    bt_data8_t *req_p;
#if defined (BT951)
    bt_map_block_t map_block;
#endif
   
    /*
    ** Check for bad descriptors and pointers
    */
    if (BT_DESC_BAD(btd)) {
        ret_val = BT_EDESC;
        goto bt_mmap_end;
    }
    if (BT_PTR_BAD(mmap_p, sizeof(void *))) {
        ret_val = BT_EINVAL;
        goto bt_mmap_end;
    }

    /*
    ** Adjust the length based on whether we needed to
    ** adjust remote offset during the mmap call
    */
    size_of_page = sysconf(_SC_PAGESIZE);
    req_p = ((bt_data8_t *) mmap_p - ((bt_devaddr_t) mmap_p % size_of_page));
    req_length = map_len + ((bt_devaddr_t) mmap_p % size_of_page);

    /*
     * In order to unmap, LynxOS needs to use an ioctl command. LynxOS does 
     * not have an unmap entry point for device drivers.
     */
#if defined (BT951)

    map_block.retval = ret_val;
    /* Check to make sure we release the correct address */
    if (btd->map_virt_addr == (unsigned int)mmap_p) {
        map_block.map_p = (void*)btd->driver_phys_addr;
        ret_val = bt_ctrl(btd, BIOC_UNMMAP, (void*)&map_block);
        if (ret_val != 0) {
            perror("Unable to unmap driver address\n");
            ret_val = BT_EACCESS;
            goto bt_mmap_end;
        }

        /* Unmap the virtual address given to us by LynxOS */
        ret_val = munmap(mmap_p, req_length);
        if (ret_val != 0) {
            perror("Unable to unmap user address\n");
            ret_val = BT_EACCESS;
            goto bt_mmap_end;
        }
    } 

#else

    ret_val = munmap((void *) req_p,  req_length);

#endif

    /* This statement is new too */
    if (ret_val != BT_SUCCESS) {
#if defined(BT965)
        ret_val = (bt_error_t) oserror();
#else /* BT965 */
        ret_val = (bt_error_t) errno;
#if defined (BT951)
    /* Close the memory device opened for this mmap system call. */
        close(btd->dev_mem_fd);
#endif
        
#endif /* BT965 */
    }

bt_mmap_end:
    return ret_val;
}
