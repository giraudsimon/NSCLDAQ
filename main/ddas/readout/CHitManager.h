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

/** @file:  CHitManager.h
 *  @brief: Collects and sorts hits from modules and outputs hits in a
 *          specified time window.
 */
#ifndef CHITMANAGER_H
#define CHITMANAGER_H
#include "ModuleReader.h"
#include <deque>
#include <vector>
namespace DDASReadout {
/**
 * @class CHitManager
 *     Collects hits from modules and retains them in a sorted deque
 *     On request, provides hits that were accepted within some sliding
 *     time interval.  The time interval is defined at construction time
 *     and is in units of seconds (1.0E9 timestamp ticks as timestamps are
 *     in ns).
 *
 *     This module does no storage manager, the receiver of all hits is expected
 *     to release any events that have been output.
 */
class CHitManager {
private:
    double                               m_emitWindow;
    std::deque<ModuleReader::HitInfo>    m_sortedHits;
    bool                                 m_flushing;
public:
    CHitManager(double window);
    ~CHitManager();
    
    void addHits(std::vector<std::deque<ModuleReader::HitInfo>>& newHits);
    bool haveHit() const;
    ModuleReader::HitInfo getHit();
    
    void clear();                 // Clear/release all stored hits.
    void flushing(bool amI) { m_flushing = amI; }
private:
    
    // Sorting and merging support.
    
    void merge(
        std::deque<ModuleReader::HitInfo>& result,
        std::vector<std::deque<ModuleReader::HitInfo>>& newHits
    );
    void merge(
        std::deque<ModuleReader::HitInfo>& result,
        std::deque<ModuleReader::HitInfo>& newHits
    );

    static bool lessThan(
        const ModuleReader::HitInfo& q1,
        const ModuleReader::HitInfo& q2
    );
    
    static double timeStamp(const ModuleReader::HitInfo& hit);
};


}                                // namespace.

#endif