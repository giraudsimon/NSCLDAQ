/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CTimeChecker.cpp
 *  @brief: Implement the ringbuffer decoder for checking event file time ordering.
 */

#include "CTimeChecker.h"

#include <CRingItem.h>
#include <DataFormat.h>

#include <iostream>


/**
 *  Constructor
 *     Just initialize stuff:
 */
CTimeCheckDecoder::CTimeCheckDecoder() :
    m_lastStamp(0), m_ooCount(0)
{}

/**
 *  operator()
 *     If the ring item is a PHYSICS_EVENT, then check its timestamp
 *     if it's earlier than the lawt one, then yell and count an out of order
 *     event.
 *   @param pItem - undifferentiated ring item.
 */
void
CTimeCheckDecoder::operator()(CRingItem* pItem)
{
    if (pItem->type() == PHYSICS_EVENT) {
        if (!pItem->hasBodyHeader()) {
            std::cerr << "Got a physics item without a body header\n";
            return;
        }
        std::uint64_t tstamp = pItem->getEventTimestamp();
        if (tstamp < m_lastStamp) {
            std::cerr << "Timestamp out of order; Last was: " << m_lastStamp
                << "  Next one was: " << tstamp <<   " dt = " << (m_lastStamp - tstamp) << std::endl;
                
            m_ooCount++;
        }
        m_lastStamp = tstamp;
    }
}
/**
 * onEndFile
 *    Called at end of input file.
 *    Reports the number of errors:
 */
void
CTimeCheckDecoder::onEndFile()
{
    std::cerr << " End of file we saw " << m_ooCount << " out of order timestamps\n";
}