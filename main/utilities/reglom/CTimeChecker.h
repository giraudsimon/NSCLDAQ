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

/** @file:  CTimeChecker.h
 *  @brief: Defines the CRingItemDecoder class used to check timestamp ordering.
 */

#ifndef CTIMECHECKER_H
#define CTIMECHECKER_H
#include "CRingItemDecoder.h"
class CFragmentHandler;
class CEndOfEventHandler;
class CRingItem;
class CPhysicsEventItem;

struct FragmentInfo;

#include <cstdint>

/**
 * CRingItemDecoder
 *    This class checks an event file and reports when timestamps for physics
 *    events are out of order.  It's a ringitemdecoder like class that can
 *    be used by the analysis sample framework.
 */
class CTimeCheckDecoder : public CRingItemDecoder
{
private:
    std::uint64_t m_lastStamp;
    unsigned      m_ooCount;
    
public:
    CTimeCheckDecoder();
    
    // Required interfaces
    
    void operator()(CRingItem* pItem);           
    void onEndFile();
    
};
#endif