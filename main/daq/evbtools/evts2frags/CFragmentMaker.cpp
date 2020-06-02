/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CFragmentMaker.cpp
 *  @brief: Implement code to create fragment headers for ring items.
 */
#include "CFragmentMaker.h"
#include <fragment.h>
#include <DataFormat.h>

/**
 * constructor
 *    Just initializes the data elements.
 *  @param defaultSourceId - id of the source when there's no body header.
 */
CFragmentMaker::CFragmentMaker(int defaultSourceId) :
    m_nLastTimestamp(0), m_nEndRunsRemaining(0), m_nDefaultSourceId(defaultSourceId)
{}

/**
 * makeHeader
 *    This is the core method of this class.  Given a ring item, the
 *    method produces an appropriate fragment header which, when prepended
 *    makes the ring item look like it's come from the event builder.
 *
 * @param pItem
 *    Pointer to the raw ring item
 * @return FragmentHeader - the appropriate fragment header.
 */
EVB::FragmentHeader
CFragmentMaker::makeHeader(RingItem* pItem)
{
    EVB::FragmentHeader result = {0, 0, 0, 0};
    
    // The payload size is always the size of the ring item.
    
    result.s_size = itemSize(pItem);
    
    // Some actions depend on the item type:
    
    uint32_t type = itemType(pItem);
    typeDependentProcessing(type);
    
    // If there's a body header (nonzero body header size), that
    // determines the remainder of the contents of the header.
    
    
    if (hasBodyHeader(pItem)) {
        pBodyHeader pB = reinterpret_cast<pBodyHeader>(bodyHeader(pItem));
        result.s_timestamp = pB->s_timestamp;
        result.s_sourceId  = pB->s_sourceId;
        result.s_barrier   = pB->s_barrier;
    } else {
        result.s_timestamp = NULL_TIMESTAMP; // That gets fixed up below.
        result.s_sourceId  = m_nDefaultSourceId;
        result.s_barrier   = barrierType(type);
    }
    // If the timestamp is NULL_TIMESTAMP then we must adjust it to be the
    // most recent one:
    
    if (result.s_timestamp == NULL_TIMESTAMP) {
        result.s_timestamp = m_nLastTimestamp;
    } else {
        m_nLastTimestamp = result.s_timestamp;   // otherwise update the last
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Private utilities.

/**
 * typeDependentProcessing
 *     Performs any procesing that depends on a  ring item type:
 *
 *  @param type - type of ring item.
 */
void
CFragmentMaker::typeDependentProcessing(uint32_t type)
{
    switch (type) {
    case BEGIN_RUN:
        m_nEndRunsRemaining++;
        break;
    case END_RUN:
        m_nEndRunsRemaining--;
        break;
    }
}
/**
 * barrierType
 *    Returns the barrier type code depending on the item type:
 *
 *  @param type - item type.
 *  @return uint32_t
 */
uint32_t
CFragmentMaker::barrierType(uint32_t type)
{
    switch(type) {
    case BEGIN_RUN:
        return 1;
    case END_RUN:
        return 2;
    default:
        return 0;
    }
}