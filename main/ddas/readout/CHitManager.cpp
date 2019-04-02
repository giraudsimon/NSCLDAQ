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

/** @file:  CHitMnager.cpp
 *  @brief: Implements the CHitManager class, see CHitManager.h
 */

#include "CHitManager.h"
#include "ZeroCopyHit.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <map>

namespace DDASReadout {

/**
 * Constructor.  We just need to save the emit window.
 *   @param window - seconds in the emit window.  This will be turned into
 *                   ns.
 */
CHitManager::CHitManager(double window) :
    m_emitWindow(window * 1.0e9), m_flushing(false)
{}
/**
 *  destructor
 *    Kills off any remaining hits in the sorted hits dequeue.
 *
 */
CHitManager::~CHitManager()
{
    if (m_sortedHits.size() > 0) {
        std::cerr << "CHitManager - killing off " << m_sortedHits.size()
            << " residual hits\n";
        clear();
    }
}
/**
 * addHits
 *    Adds hits from a set of modules.  The hits are sorted and then
 *    merged into the sorted hits.  I _think_ my big O analysis says that
 *    the fastest way to do this is to first sort each dequeue and then
 *    merge them using the minheap algorithm.  Those then get
 *    insertion merged into the main sorted hits deque.
 *
 *    Note that the merging is all done is private methods.
 *
 * @param newHits - this is a vetor of dequeues of hit information.
 *                  the idea is that each of the deques in the vector
 *                  is data from one module.  Note that maintaining
 *                  module separation makes the module sorting several
 *                  small sorts rather than, potentially one big sort
 *                  and I think that comes out ahead.
 * @note - the deques in the vector will be emptied.
 * @note - m_sortedHits will be extended by all the hits in the system.
 */
void
CHitManager::addHits(std::vector<std::deque<ModuleReader::HitInfo>>& newHits)
{
    // Sort hits from each module:
    
    for (int i =0; i < newHits.size(); i++ ) {
        std::deque<ModuleReader::HitInfo>& m(newHits[i]);
#ifdef SORTING
        std::sort(m.begin(), m.end(), lessThan);
#else
        m_sortedHits.insert(
            m_sortedHits.end(), newHits[i].begin(), newHits[i].end()
        );
#endif
    }
#ifdef SORTING    
    // Now do the minheap merge into one grand new dequeue:
    
    std::deque<ModuleReader::HitInfo> newSortedHits;
    merge(newSortedHits, newHits);
    
    // merge those into the sorted hits:
    
    merge(m_sortedHits, newSortedHits);
#endif
}
/**
 * haveHits
 *    @return bool - true if there's at least one hit that can be output.
 */
bool
CHitManager::haveHit() const
{
    // If there are not at least two hits we can't check the timewindow so
    // there's nothing to output... unless we're flushing.
    
    
    if (m_sortedHits.size() > 1) {
        return (m_flushing || (timeStamp(m_sortedHits.back()) -
                timeStamp(m_sortedHits.front()) > m_emitWindow));
    } else if (m_flushing && m_sortedHits.size()) {
        return true;                       // One hit only....
    } else  {
        return false;
    }
}
/**
 * getHit
 *    Returns the hit info at the front of the deque of sorted hits,
 *    dropping it from the deque.  Throws a logic_error exception if the
 *    deque is empty.
 *
 *    Normally this should be called after a acll to haveHits returns true.
 *
 *  @return ModuleReader::HitInfo
 */
ModuleReader::HitInfo
CHitManager::getHit()
{
    if (m_sortedHits.empty()) {
        throw std::logic_error("CHitManager trying to get hits from an empty sortlist");
    }
    ModuleReader::HitInfo result = m_sortedHits.front();
    m_sortedHits.pop_front();
    return result;
}
/**
 * clear
 *    Clear the sorted hit deque.  This means dereferenceing each hit
 *    as it comes off the dequeue.
 */
void
CHitManager::clear()
{
    while (!m_sortedHits.empty()) {
        ModuleReader::freeHit(m_sortedHits.front());
        m_sortedHits.pop_front();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Private utility methods.

/**
 * merge
 *   This overload merges a vector of sorted dequeues onto the back end of
 *   a dequeue.  The merge is done by using the method of min-heaps
 *   until only one dequeue has data at which point in time, all elements
 *   of that remaining queue are just appended to the output deque.
 *
 *   Complexity analysis  Suppose there are m dequeues in the input array
 *   and each of them has n/m elements where n is the total number of elements
 *   The insertions into the minheap are O(log(m)) and this has to be done
 *   n times for O(nlog(m)).  Since typically n << m all of this is much
 *   faster than the O(nLog(n)) required to just merge them all and sort them.
 *
 *   For a simple descriptino of minheap merging see e.g.
 *   https://www.geeksforgeeks.org/merge-k-sorted-linked-lists-set-2-using-min-heap/
 *
 * @param[out]  result    - The dequeue into which the elements will be put.
 * @param[inout] newHits  - Vector of input dequeues.  These will be emptied
 *                          by this method.
 * @note - this method assumes that on input at least one of the
 *         newHits deques has at least one element.
 * @note -currently we use an std::multimap for the minheap... that may require
 *        costly new/free's.  If that turns out to be the case, we can
 *        just do std::sort's on an std::vector of std::pair<double, dequeue*>
 *        Note that multimap must be used because there may be duplicate timestamps.
 */
void
CHitManager::merge(
    std::deque<ModuleReader::HitInfo>& result,
    std::vector<std::deque<ModuleReader::HitInfo>>& newHits
)
{
    // The minheap will be a tree that's keyed by the timestamps of the hits
    // at the front of the queues to which element bodies point.
    
    std::multimap<double, std::deque<ModuleReader::HitInfo>*> minheap;
    
    // Stock the minheap initially:
    
    for (int i = 0; i < newHits.size(); i++) {
        if (newHits[i].size() > 0) {
            minheap.emplace(std::make_pair(timeStamp(newHits[i].front()), &(newHits[i])));
        }
    }
    // Here's the main part of the minheap merge:
    
    // If the minheap is empty we got handed a vector of empty deques:
    
    if (minheap.empty()) return;
    
    while (minheap.size() > 1) {
        auto earliest = minheap.begin();          // Smallest timestamp.
        std::deque<ModuleReader::HitInfo>* pQ = earliest->second;
        result.push_back(pQ->front());
        pQ->pop_front();
        minheap.erase(earliest);
        
        // If pQ has more hits enter it back in the map.
        
        if (!pQ->empty()) {
            minheap.emplace(std::make_pair(timeStamp(pQ->front()), pQ));
        }
        
    }
    // At this point, the map has only one or element.  Put all of the remaining
    // items in that queue into the result:
    
    std::deque<ModuleReader::HitInfo>& lastQ(*minheap.begin()->second);
    
    while(!lastQ.empty()) {
        result.push_back(lastQ.front());
        lastQ.pop_front();
    }
}
/**
 *  This merge merges new hits into an existing sorted hits list.
 *  We assume that, other than when starting up, the time range covered by the
 *  output deque is much larger than that of the new hits (the window is
 *  seconds where at high rates, which is what we care about optimizing, the
 *  data read from a module will be milliseconds?).
 *
 *  Here are the cases:
 *  -  If the ouptut is empty, - just set the outut to the input.
 *  -  IF the last element of the output has a timestamp that's less than or
 *     equal to that of the front of the new hits, we can just append the new hits.
 *  - Otherwise, pull elements off the back of the existing deque into a temporary
 *    dequeue until the timestamp on the back of the existing dequeue is greater
 *    than that of the front of the new elements then use std::merge
 *    to merge the resulting dequeues into the end of the existing deque.
 *
 *  @param result - the resulting merged hits.
 *  @param newHits - sorted input hits.
 */
void
CHitManager::merge(
    std::deque<ModuleReader::HitInfo>& result,
    std::deque<ModuleReader::HitInfo>& newHits
)
{
    // case 0... if there are no new hits do nothing:
    
    if (newHits.empty()) return;
    
    // case 1... the result is empty:
    
    if (result.empty()) {
        result = newHits;
        return;
    }
    // Case 2... the back of the result has a timestamp < front of newhits:
    //           append newHits to result.
    if (lessThan(result.back(), newHits.front())) {
        result.insert(result.end(), newHits.begin(), newHits.end());
        return;
    }
    // Case 3 .. we need to work by making a new deque of the
    //           elements to merge.  One sub case we glossed over is that,
    //           especially in early going, we may need to put all of the
    //           elements in result in the merge struct.
    
    std::deque<ModuleReader::HitInfo> tempQueue;
    while (!result.empty() && !(lessThan(result.back(), newHits.front()))) {
        tempQueue.push_front(result.back());         // This preserves ordering
        result.pop_back();
    }
    // merge the lists into the end of the temp queue since
    // the back of the result queue has a timstamp that's < the timestamps
    // in the front of temp and newHits:
    
    std::merge(
        tempQueue.begin(), tempQueue.end(),
        newHits.begin(), newHits.end(),
        std::back_inserter(result),
        lessThan
    );
}
/**
 * lessThan
 *    Given two hit info references, returns true if the first one
 *    has a timestamp strictly less than the second.
 *
 * @param q1 - first hit
 * @param q2 - second hit.
 * @return bool
 */
bool
CHitManager::lessThan(
    const ModuleReader::HitInfo& q1,
    const ModuleReader::HitInfo& q2
    
)
{
    return timeStamp(q1) < timeStamp(q2);
}


/**
 * timeStamp
 *    Given a hit info returns the timestamp of the zero copy hit it contains.
 *
 * @param hit - hit information pair.
 */
double
CHitManager::timeStamp(const ModuleReader::HitInfo& hit)
{
    return hit.second->s_time;
}


}                                   // Namespace.