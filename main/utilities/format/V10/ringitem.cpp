/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "V10/DataFormatV10.h"

namespace DAQ {
  namespace V10 {

/**
 * Implement the functions prototypes in DataFormat.h
 * These are functions that produce ring items.
 *
 */

/*------------------------------------------------------------------------------------*/

/**
 * Format a physics event item.  The payload is put in the event body preceded by the uin32-T
 * nWords.
 *
 * @param nWords - Number of uint16_t items in the event payload.
 * @param pPayload - The payload of the event.
 *
 *  
 * @return pPhysicsEventitem - Pointer to the new item.
 *         at some point the caller must free(3) the item.
 */
 pPhysicsEventItem  formatEventItem(size_t nWords, void* pPayload)
 {
   /* The size of the resulting physics even item will be
      Size of a RingItemHeader +
      size of a uint32_t (leading word count)
      nWords*sizeof(uint16_t):
   */
   size_t itemSize = sizeof(RingItemHeader) + sizeof(uint32_t) + nWords*sizeof(uint16_t);
   pPhysicsEventItem pItem;
   uint32_t*              pBody;

   pItem = (pPhysicsEventItem)malloc(itemSize);
   if (!pItem) {
     return pItem;
   }
   pItem->s_header.s_size = itemSize;
   pItem->s_header.s_type = PHYSICS_EVENT;
   
   pBody = (uint32_t*)(pItem->s_body);
   *pBody++ = nWords + sizeof(uint32_t)/sizeof(uint16_t);
   if (nWords) {
     memcpy(pBody, pPayload, nWords * sizeof(uint16_t)); /* In case memcpy is ill defined for n = 0. */
   }

   return pItem;
 }

/**
 * Create/Format a trigger count item.
 * 
 * @param  runTime - Number of seconds into the run represented by this item.
 * @param  stamp   - Unix timestamp at which the item was emitted.
 * @param  triggerCount - Number of triggers accepted this run.
 *
 * @return pPhysicsEventCount - Pointer; to the formatted item.  This must be passed to
 *                               free(3) to reclaim storage.
*/
pPhysicsEventCountItem
formatTriggerCountItem(uint32_t runTime, time_t stamp, uint64_t triggerCount)
{
  pPhysicsEventCountItem pItem = (pPhysicsEventCountItem)malloc(sizeof(PhysicsEventCountItem));
  if (!pItem) {
    return pItem;
  }

  pItem->s_header.s_size = sizeof(PhysicsEventCountItem);
  pItem->s_header.s_type = PHYSICS_EVENT_COUNT;
  pItem->s_timeOffset    = runTime;
  pItem->s_timestamp     = stamp;
  pItem->s_eventCount    = triggerCount;

  return pItem;
}
/**
 * Create/format a scaler item.
 * @param scalerCount - Number of 32 bit scaler items in the item.
 * @param timestamp   - Absolute timestamp.
 * @param btime       - Offset to beginning of interval from start of run.
 * @param etime       - Offset to end of interval from start of run.
 * @param pCounters   - The counters.
 *
 * @return pScalerItem Pointer to the scaler aitme we are going to return.  Must be passed to
 *                     free(3) to release storage.
 */
pScalerItem
formatScalerItem(unsigned scalerCount, time_t timestamp, uint32_t btime, uint32_t etime, void* pCounters)
{
  size_t allocationSize = sizeof(ScalerItem) + (scalerCount)*sizeof(uint32_t) - sizeof(uint32_t);
  pScalerItem pItem = (pScalerItem)malloc(allocationSize);


  if(!pItem) {
    return pItem;
  }
  
  pItem->s_header.s_size = sizeof(ScalerItem) + (scalerCount)*sizeof(uint32_t) - sizeof(uint32_t);
  pItem->s_header.s_type = INCREMENTAL_SCALERS;
  pItem->s_intervalStartOffset = btime;
  pItem->s_intervalEndOffset   = etime;
  pItem->s_timestamp           = timestamp;
  pItem->s_scalerCount         = scalerCount;
  memcpy(pItem->s_scalers, pCounters, scalerCount*sizeof(uint32_t));

  return pItem;
}

/**
 * formatNonIncrTSScalerItem
 *
 *  Format a timestamped non incremental scaler item.  The timestamp in this case refers
 *  to a timestamp synched to the event trigger timestamps (e.g. for event building ordering
 *
 * @param scalerCount - Number of 32 bit scaler items in the item.
 * @param timestamp   - Absolute, unix, timestamp.
 * @param btime       - Offset to beginning of interval from start of run.
 * @param etime       - Offset to end of interval from start of run.
 * @param eventTimestamp - Event timestamp.
 * @param pCounters   - The counters.
 * @param timebasDivisor - What to divide the btime/etime values by to get seconds.
 *
 * @return pNonIncrTSScaleritem Pointer to the scaler aitme we are going to return.  Must be passed to
 *                     free(3) to release storage. 
 */
pNonIncrTimestampedScaler  
formatNonIncrTSScalerItem(unsigned scalerCount, time_t timestamp, 
			  uint32_t btime, uint32_t etime, 
			  uint64_t eventTimestamp, void* pCounters,
			  uint32_t timebaseDivisor)
{

  uint32_t itemSize = sizeof(NonIncrTimestampedScaler) + scalerCount*sizeof(uint32_t);

  /* allocate the item and return NULL if the malloc failed. */

  pNonIncrTimestampedScaler  pItem =  (pNonIncrTimestampedScaler)malloc(itemSize);
  if (!pItem) {
    return pItem;
  }

  /* Fill in the item fields except for the scalers which must be memcpy'd later. */

  pItem->s_header.s_size = itemSize;
  pItem->s_header.s_type = TIMESTAMPED_NONINCR_SCALERS;
  pItem->s_eventTimestamp      = eventTimestamp;
  pItem->s_intervalStartOffset = btime;
  pItem->s_intervalEndOffset   = etime;
  pItem->s_intervalDivisor     = timebaseDivisor;
  pItem->s_clockTimestamp      = timestamp;
  pItem->s_scalerCount         = scalerCount;

  /* Copy in the scalers */

  memcpy(pItem->s_scalers, pCounters, scalerCount * sizeof(uint32_t));

  return pItem;

}


/**
 * Format a set of text strings into a Textitem.
 *
 * @param nStrings - number of strings to format.
 * @param stamp    - Absolute timestamp.
 * @param runTime  - Time into run.
 * @param pStrings - Pointer to the array of string pointers.
 * @param type     - Type of item to create (e.g. MONITORED_VARIABLES).
 *
 * @return pTextItem - pointer to the formatted text item.  This is dynamically allocated and must be
 *                     passed to free(3) to release the storage.
 */

pTextItem
formatTextItem(unsigned nStrings, time_t stamp, uint32_t runTime,  const char** pStrings, int type)
{
  pTextItem pItem;
  size_t    stringSizes = 0;
  char*     pDest;
  int       i;
  /* Figure out the string storage size required: */

  for (i = 0; i < nStrings; i++) {
    stringSizes += strlen(pStrings[i]);
    stringSizes++;
  }

  pItem = (pTextItem)malloc(sizeof(TextItem) + stringSizes - 1);
  if (!pItem) {
    return pItem;
  }
  pItem->s_header.s_size = (sizeof(TextItem) + stringSizes -1);
  pItem->s_header.s_type = type;
  pItem->s_timeOffset    = runTime;
  pItem->s_timestamp     = stamp;
  pItem->s_stringCount   = nStrings;

  pDest  = pItem->s_strings;
  for (i = 0; i < nStrings; i++) {
    memcpy(pDest, pStrings[i], strlen(pStrings[i])  + 1);
    pDest += strlen(pStrings[i]) +1;
  }
  
  return pItem;
}

/**
 * Allocate and format a state change item
 *
 * @param stamp - Absolute timestamp.
 * @param offset - Seconds into the run of the state change.
 * @param runNumber - Number of the run.
 * @param pTitle  - Pointer to the null terminated title string.
 * @param type    - Type of item being created e.g. BEGIN_RUN.
 *
 * @return pStateChangeItem - Pointer to the storage allocated for the state change
 *                            item.. Must be passed to free(3) to reclaim storage.
 */
pStateChangeItem
formatStateChange(time_t stamp, uint32_t offset, uint32_t runNumber,
		  const char* pTitle, int type)
{
  pStateChangeItem pItem = reinterpret_cast<pStateChangeItem>(malloc(sizeof(StateChangeItem)));
  
  if (pItem) {
    pItem->s_header.s_size = sizeof(StateChangeItem);
    pItem->s_header.s_type = type;

    pItem->s_runNumber = runNumber;
    pItem->s_timeOffset = offset;
    pItem->s_Timestamp  = stamp;
    memset(pItem->s_title, 0, TITLE_MAXSIZE + 1);
    memcpy(pItem->s_title,  pTitle, TITLE_MAXSIZE) ; 
  }

  return pItem;
  
}

  } // end of V10 namespace
} // end of DAQ
