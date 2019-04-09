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

/** @file:  HitManager.h
 *  @brief: Maintain a sorted list of hits.
 */

#ifndef HITMANAGER_H
#define HITMANAGER_H

#include <deque>
#include <stdint.h>

namespace DDASReadout {
class ZeroCopyHit;
}
class HitManager
{
private:
    std::deque<DDASReadout::ZeroCopyHit*> m_sortedHits;
    uint64_t                 m_nWindow;    
public:
    HitManager(uint64_t window);
    void addHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits);
    bool haveHit();
    DDASReadout::ZeroCopyHit* nextHit();
private:
    void sortHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits);
    void mergeHits(std::deque<DDASReadout::ZeroCopyHit*>& newHits);
};


#endif