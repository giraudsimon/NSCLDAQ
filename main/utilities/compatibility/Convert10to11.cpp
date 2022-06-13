/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

     @file Convert10to11.cpp
     @brief Filter to convert nscldaq-10.x to nscldaq-11+ data.
*/
#include <DataFormat.h>
#include "OldDataFormat.h"
#include <CFileDataSource.h>
#include <CRingItem.h>
#include <io.h>
#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>


#ifndef TRUE
#define TRUE 1
#endif

/**
 * mustSwap
 * 
 *  Determines if a ring item is byte swapped relative to this host.
 *
 * @param p - Pointer to the ring item.
 *
 * @return bool
 *
 */
static bool
mustSwap(pRingItem p)
{
  return ((p->s_header.s_type & 0xffff) == 0);
}


/**
 * itemSize
 *
 *   Returns the size of a ring item in bytes, taking into account the possibility to need
 *   to do byte order swapping.
 *
 * @param pItem - Pointer to the ring item.
 *
 * @return uint32_t -size.
 */
static uint32_t
itemSize(pRingItem pItem)
{
  uint32_t n = pItem->s_header.s_size;
  if (mustSwap(pItem)) {
    n = swal(n);
  }
  return n;
}

/**
 * setItemSize 
 *
 *  Set the size of a ring buffer in bytes, taking into account the possibility we need
 *  to byte swap.
 *
 * @param pItem - Ring item modified.
 * @param n     - new number of bytes
 *
 */
static void
setItemSize(pRingItem pItem, uint32_t n)
{
  if(mustSwap(pItem)) {
    n = swal(n);
  }
  pItem->s_header.s_size = n;
}

/**
 * simpleWrite
 *
 *  Writes a ring item whose shape is already correct.
 *
 *  @param pItem - Pointer to the ring item.
 */
static void
simpleWrite(pRingItem pItem)
{
    uint32_t nbytes = itemSize(pItem);
    try {
        io::writeData(STDOUT_FILENO, pItem, nbytes);
    }
    catch (std::string msg){
        std::cerr << "Ring item could not be written: " << msg << std::endl;
        exit(EXIT_FAILURE);
    }
}

/**
 * writeItem
 *
 *   Write an untranslated 10.x ring item out in 11 format
 *   - Add sizeof(uint32_t) to the ring item size).
 *   - Write the header.
 *   - Write a uint32_t of zero (body header size).
 *   - Write the body.
 */
static void writeItem(pRingItem pOld)
{ 
  uint32_t size = itemSize(pOld);
  uint32_t zero(0);
  setItemSize(pOld, size+sizeof(uint32_t));
  uint8_t* pBody = reinterpret_cast<uint8_t*>(pOld) + sizeof(RingItemHeader);

  try {
    
        io::writeData(STDOUT_FILENO, pOld, sizeof(RingItemHeader));
        io::writeData(STDOUT_FILENO, &zero, sizeof(uint32_t));
        io::writeData(
            STDOUT_FILENO, pBody, size - sizeof(RingItemHeader) 
        );
 
  }
  catch (std::string msg) {
        std::cerr << "Error writing data: " << msg << std::endl;
        exit(EXIT_FAILURE);
  }
}

/**
* translateStateChange
*   Translate a 10.x statechange item to an 11.x item and write it to file.
*   The only real translation required is to set the offset divisor to 1.
*   indicating a 1 second timebase.
*
* @param pOld Pointer to a NSCLDAQ10::TextItem
*/
static void
translateStateChange(pRingItem pOld)
{
    // Final ring item shape:
    struct {
        RingItemHeader      s_hdr;
        uint32_t            s_mbz;
        StateChangeItemBody s_body;
    } dest;
    
    NSCLDAQ10::pStateChangeItem pSource =
        reinterpret_cast<NSCLDAQ10::pStateChangeItem>(pOld);
    pStateChangeItemBody pDest = &(dest.s_body);
    pRingItemHeader      pHdr  = &(dest.s_hdr);
    
    bool                 swap    = mustSwap(pOld);
    
    // Copy the header and items in the body we already have.
    
    pDest->s_runNumber    = pSource->s_runNumber;
    pDest->s_timeOffset   = pSource->s_timeOffset;
    pDest->s_Timestamp    = pSource->s_Timestamp;
    memcpy(pDest->s_title, pSource->s_title, TITLE_MAXSIZE+1);
    
    // the timestamp divisor:
    
    pDest->s_offsetDivisor = swap ? swal(1) : 1;
    
    // The header.
    
    pHdr->s_size = swap ? swal(sizeof(dest)) : sizeof(dest);
    pHdr->s_type = pOld->s_header.s_type;
    
    // Finally the mbz field -- this is to V11 not V12.
    
    dest.s_mbz = 0;
    
    // Write the final item:
    
    simpleWrite(reinterpret_cast<pRingItem>(&dest));
    
}
/**
 * writeDataFormatItem
 *
 *   Write the appropriate data format item.
 */
static void
writeDataFormatItem()
{
    pDataFormat pFormatItem = formatDataFormat();
    simpleWrite(reinterpret_cast<pRingItem>(pFormatItem));
    
    free(pFormatItem);
}
/**
 * translateIncrementalScalers
 *    Translate an incremental scaler item into the new generic
 *    scaler item.  Incremental scalers will have an offset divisor of
 *    1 and the incremental flag true.
 *
 * @param pOld - Pointer to the old item.
 */
static void
translateIncrementalScalers(pRingItem pOld)
{
    struct _Scaler {
        RingItemHeader s_hdr;
        uint32_t       s_mbz;
        ScalerItemBody s_body;
    }* pDest;
    
    bool                   swap = mustSwap(pOld);
    NSCLDAQ10::pScalerItem pSrc = reinterpret_cast<NSCLDAQ10::pScalerItem>(pOld);
    
    // Figure out how big the actual item is and allocate it:
    
    uint32_t nScalers = pSrc->s_scalerCount;
    if (swap)nScalers = swal(nScalers);
    
    uint32_t nBytes = sizeof(RingItemHeader) + sizeof(uint32_t)
        + sizeof(ScalerItemBody) + nScalers*sizeof(uint32_t);
    pDest = reinterpret_cast<struct _Scaler*>(malloc(nBytes));
        
    if (!pDest) {
        perror("Allocation failed for incremental scaler output ring item");
        exit(EXIT_FAILURE);
    }
    // Fill in the item body:
    
    pScalerItemBody pBody = &(pDest->s_body);
    pBody->s_intervalStartOffset = pSrc->s_intervalStartOffset;
    pBody->s_intervalEndOffset   = pSrc->s_intervalEndOffset;
    pBody->s_timestamp           = pSrc->s_timestamp;
    pBody->s_scalerCount         = pSrc->s_scalerCount;
    memcpy(pBody->s_scalers, pSrc->s_scalers, nScalers*sizeof(uint32_t));
    
    pBody->s_intervalDivisor = swap ? swal(1) : 1;
    pBody->s_isIncremental   = swap ? swal(TRUE) : TRUE;
    
    // The header:
    
    pDest->s_hdr.s_size = swap ? swal(nBytes) : nBytes;
    pDest->s_hdr.s_type = PERIODIC_SCALERS;
    
    // last the mbz field.  -- this is to V11 not V12.
    
    pDest->s_mbz = 0;
    
    // Write and free.
    
    simpleWrite(reinterpret_cast<pRingItem>(pDest));
    free(pDest);
    
}

/**
 * translateTimestampedScaler
 *
 *    Translate an old timestamped scaler item into a generic scaler item.
 *    The only difference between this and the previous function is that
 *    the translated ring item has a body header.  The source is 0, and the
 *    timestamp comes from the source item.
 *
 * @param pOld - NSCLDAQ10.x ring item.
 */
static void
translateTimestampedScaler(pRingItem pOld)
{
    struct _Scaler {
        RingItemHeader s_hdr;
        BodyHeader     s_bodyHeader;
        ScalerItemBody s_body;
    }* pDest;
    
    NSCLDAQ10::pNonIncrTimestampedScaler pSrc =
        reinterpret_cast<NSCLDAQ10::pNonIncrTimestampedScaler>(pOld);
    
    // Figure out how bit the destination ring item is and alloate it:
    // sizeof(BodyHeader) is ok b/c we're generating a body header.
    
    bool     swap     = mustSwap(pOld);
    uint32_t nScalers = swap ? swal(pSrc->s_scalerCount) : pSrc->s_scalerCount;
    size_t   itemBytes=
        sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(ScalerItemBody)
        + nScalers * sizeof(uint32_t);
    pDest = reinterpret_cast<struct _Scaler*>(malloc(itemBytes));
    if (!pDest) {
        perror("Failed to allocate translated item for non incremental scaler");
        exit(EXIT_FAILURE);
    }
    // Fill in the body of pDest:
    
    pScalerItemBody pBody = &(pDest->s_body);
    pBody->s_intervalStartOffset = pSrc->s_intervalStartOffset;
    pBody->s_intervalEndOffset   = pSrc->s_intervalEndOffset;
    pBody->s_timestamp           = pSrc->s_clockTimestamp;
    pBody->s_intervalDivisor     = pSrc->s_intervalDivisor;
    pBody->s_scalerCount         = pSrc->s_scalerCount;
    pBody->s_isIncremental       = 0;
    memcpy(pBody->s_scalers, pSrc->s_scalers, nScalers*sizeof(uint32_t));
    
    // Fill in they body header:
    
    pDest->s_bodyHeader.s_timestamp = pSrc->s_eventTimestamp;
    pDest->s_bodyHeader.s_sourceId = 0;
    pDest->s_bodyHeader.s_barrier  = 0;
    pDest->s_bodyHeader.s_size     =
        swap ? swal(sizeof(BodyHeader)) : sizeof(BodyHeader);
    
    // Fill in the item header:
    
    pDest->s_hdr.s_size = swap ? swal(itemBytes) : itemBytes;
    pDest->s_hdr.s_type = pOld->s_header.s_type;
    
    // Write and free:
    
    simpleWrite(reinterpret_cast<pRingItem>(pDest));
    free(pDest);
}
/**
 * translateTextItem
 *    Translate and write an NSCLDAQ-10.0 text ring item.
 *
 * @param pOld - Pointer to the old text ring item.
 */
static void
translateTextItem(pRingItem pOld)
{
    struct _Text {
        RingItemHeader   s_header;
        uint32_t         s_mbz;
        TextItemBody     s_body;
    }* pDest;
    NSCLDAQ10::pTextItem pSrc= reinterpret_cast<NSCLDAQ10::pTextItem>(pOld);
    
    // Figure out the size of the destination ring item and
    // allocate it.
    
    bool     swap      = mustSwap(pOld);
    uint32_t oldSize   = itemSize(pOld);
    uint32_t textBytes =
        oldSize  - sizeof(NSCLDAQ10::RingItemHeader) - sizeof(NSCLDAQ10::TextItem)
        + 1;
    
    size_t newItemSize =
        sizeof(RingItemHeader) + sizeof(uint32_t) + sizeof(TextItemBody)
        + textBytes;
    pDest = reinterpret_cast<struct _Text*>(malloc(newItemSize));
    if (!pDest) {
        perror("Unable to allocate translated text item");
        exit(EXIT_FAILURE);
    }
    // Fill in the destination body (offset divisor is propely ordered 1)
    
    pTextItemBody pBody = &(pDest->s_body);
    pBody->s_timeOffset = pSrc->s_timeOffset;
    pBody->s_timestamp  = pSrc->s_timestamp;
    pBody->s_stringCount= pSrc->s_stringCount;
    pBody->s_offsetDivisor = swap ? swal(1) : 1;
    memcpy(pBody->s_strings, pSrc->s_strings, textBytes);
    
    //  Fill in the ring item header.
    
    pDest->s_header.s_type = pSrc->s_header.s_type;
    pDest->s_header.s_size = swap ? swal(newItemSize) : newItemSize;
    
    // Fill in the mbz, write and free the dest item:  -- this is to V11 not V12.
    
    pDest->s_mbz = 0;
    simpleWrite(reinterpret_cast<pRingItem>(pDest));
    free(pDest);
}
/**
 * translateTriggerCount
 *   Translate an NSCLDAQ-10.0 trigger count ring item into an NSCLDAQ-11+
 *   item.
 *
 *   @param pOld - Pointer to an NSCLDAQ10::PhysicsEventCountItem.
 */
static void
translateTriggerCount(pRingItem pOld)
{
    struct {
        RingItemHeader            s_header;
        uint32_t                  s_mbz;
        PhysicsEventCountItemBody s_body;
    } dest;
    NSCLDAQ10::pPhysicsEventCountItem pSrc =
        reinterpret_cast<NSCLDAQ10::pPhysicsEventCountItem>(pOld);
    bool swap = mustSwap(pOld);
    
    // Translate the body:
    
    pPhysicsEventCountItemBody pDest = &(dest.s_body);
    pDest->s_timeOffset              = pSrc->s_timeOffset;
    pDest->s_timestamp               = pSrc->s_timestamp;
    pDest->s_eventCount              = pSrc->s_eventCount;
    pDest->s_offsetDivisor           = swap ? swal(1) : 1;
    
    // Translate the header:
    
    dest.s_header.s_size = sizeof(dest);
    dest.s_header.s_type = pSrc->s_header.s_type;
    
    dest.s_mbz = 0;   //  -- this is to V11 not V12.
    // Write the output:
    
    simpleWrite(reinterpret_cast<pRingItem>(&dest));
}

/**
 * main
 *
 *  Simple loop to get event, put event.
 *
 * @param argc[in] - Unsed number of command line parameters.
 * @param argv[in] - Unused array of pointers to the command words.
 *
 * @return int - Status of the command, should be EXIT_SUCCESS.
 */

int
main(int argc, char** argv)
{
  std::vector<uint16_t> exclude;
  CFileDataSource ds(STDIN_FILENO, exclude);
  
  writeDataFormatItem();                   // Prepend with data format item.

  CRingItem* pItem;
  while (pItem = ds.getItem()) {
    pRingItem pOld = pItem->getItemPointer();
    
    /*
     * Most item types just get written with the mbz word added by writeItem.
     * some, however are a bit more complicated
     */
    uint32_t type = pItem->type();   // Anything that touches RingItemHeader is safe.
    
    switch (type) {
    case NSCLDAQ10::BEGIN_RUN:
    case NSCLDAQ10::END_RUN:
    case NSCLDAQ10::PAUSE_RUN:
    case NSCLDAQ10::RESUME_RUN:
        translateStateChange(pOld);
        break;
    case NSCLDAQ10::INCREMENTAL_SCALERS:
        translateIncrementalScalers(pOld);
        break;
    case NSCLDAQ10::TIMESTAMPED_NONINCR_SCALERS:
        translateTimestampedScaler(pOld);
        break;
    case NSCLDAQ10::PACKET_TYPES:
    case NSCLDAQ10::MONITORED_VARIABLES:
        translateTextItem(pOld);
        break;
    case NSCLDAQ10::PHYSICS_EVENT_COUNT:
        translateTriggerCount(pOld);
        break;  
    default:
        writeItem(pOld);
    }
    delete pItem;
  }
}


