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

/** @file:  HitManager.cpp
 *  @brief: Implement the hit manager class.
 */
#include "HitManager.h"
#include <algorithm>
#include <ZeroCopyHit.h>
#include <RawChannel.h>
/**
 * construtor:
 *    @param window  - Difference in timestamp to allow hits to be output (ns)
 */
HitManager::HitManager(uint64_t window) : m_nWindow(window)
{
    
}

/**
 * addHits
 *    Adds a new set of hits to the m_sortedHits deque maintaining
 *    total ordering by calibrated timestamp.
 *
 * @param newHits - references the new hits to be added.
 * @note          - On return, this deque will be empty.
 */
void
HitManager::addHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    sortHits(newHits);            // First sort the incoming hits.
    mergeHits(newHits);           // then merge them into the newHits deque.
}
/**
 * haveHit
 *    @return bool - true if there is at least one hit that can be output because
 *                   older than the window of the most recent hit.
 */
bool
HitManager::haveHit()
{
    auto pfront = m_sortedHits.front();
    auto pback  = m_sortedHits.back();
    
    return ((pback->s_time - pfront->s_time) > m_nWindow);
}
/**
 * nextHit
 *   @return DDASReadout::ZeroCopyHit* - pointer to the oldest hit in the
 *                     m_sortedHits deque.
 *   @retval nullptr - if there are no hits in m_sortedHits
 *   @note on exit, if a hit is returned it has been popped off the
 *                  deque.
 */
DDASReadout::ZeroCopyHit*
HitManager::nextHit()
{
    DDASReadout::ZeroCopyHit* result;
    if (m_sortedHits.empty()) {
        result = nullptr;
    } else {
        result = m_sortedHits.front();
        m_sortedHits.pop_front();
    }
    
    return result;
}
///////////////////////////////////////////////////////////////////////////////
//  Private members.
//

//  static local comparison function to provide < when we have pointers
//  to hits.

static bool
hitCompare(DDASReadout::ZeroCopyHit* p1, DDASReadout::ZeroCopyHit* p2)
{
    return *p1 < *p2;
}
/**
 * sortHits
 *    Given a reference to a deque of hits, sorts that deque in place by
 *    increasing timestamp
 *  @param newHits - the hits to sort.
 */
void
HitManager::sortHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    std::sort(newHits.begin(), newHits.end(), hitCompare);
}
/**
 * mergeHits
 *   Merge a sorted deque of new hits into the existing set of sorted
 *   hits.
 *
 *  @param newHits - sorted dequeue of new hits
 *
 *  Special cases:
 *  -  m_sortedHits is empty - assign it to new hits.
 *  -  m_sortedHits.last() <= newHits.front() - just insert the new hits
 *     at the end of the current sorted list.
 *  -  m_sortedHits.front() >= newHits.back()  - prepend the new hits.
 *  -  Otherwise the following steps are performed:
 *     1. append the new hits to sorted hits.
 *     2. Search backwards in the existing hits until either we come to the first
 *        existing hit or we come to an element who's time is <= to newhit's front.
 *     3. do an in-placd merge of those two ranges of the sorted_list
 *
 */
void
HitManager::mergeHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits)
{
    if (m_sortedHits.empty()) {
        m_sortedHits = newHits;                         // assign
    } else {
        auto oldFront = m_sortedHits.front();
        auto newBack  = m_sortedHits.back();
        if (hitCompare(newBack, oldFront)) {            // prepend
            m_sortedHits.insert(m_sortedHits.begin(), newHits.begin(), newHits.end());
        } else {
            // Start by appending:
            
            auto newPosition =
                m_sortedHits.insert(m_sortedHits.end(), newHits.begin(), newHits.end());
            auto oldEnd = newPosition;
            --oldEnd;
            if (!hitCompare(*oldEnd, *newPosition)) {
                // Must merge too.
                
                std::inplace_merge(m_sortedHits.begin(), newPosition, m_sortedHits.end());
            }
        }
    }
}