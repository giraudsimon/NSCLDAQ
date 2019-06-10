/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   tsextract.c
# @brief  Timestamp extractor for events that have the timestamp in the first position.
#         
# @author Ron Fox<fox@nscl.msu.edu>
*/

/**
 * @note scaler timestamps will be one larger than the last event timestamp or
 *       0xffffffffffffffff if there is no last timestamp.
 *
 */

#include <stdint.h>

static uint64_t lastTimestamp = 0xffffffffffffffff;

uint64_t getEventTimestamp(void* pEvent)
{
    int i;
    uint16_t *pBody = (uint16_t*)(pEvent);
    uint64_t result = 0;
    
    pBody++;                      // Skip size.
    for (i =3; i >= 0; i--)
    {
        uint64_t part = pBody[i];
        result |= (part << i*16);
    }
    lastTimestamp = result;
    return result;
}
uint64_t getScalerTimestamp(void* pScaler)
{
    if (lastTimestamp == 0xffffffffffffffff) {
        return lastTimestamp;
    } else {
        return ++lastTimestamp;
    }
}