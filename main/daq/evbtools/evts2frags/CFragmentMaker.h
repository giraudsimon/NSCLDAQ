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

/** @file:  CFragmentMaker.h
 *  @brief: Given ring items, create fragment headers for them.
 */

#ifndef CFRAGMENTMAKER_H
#define CFRAGMENTMAKER_H
#include <stdint.h>

typedef struct _RingItem RingItem;
namespace EVB {
    typedef struct _FragmentHeader FragmentHeader;
}
class CFragmentMaker
{
private:
    uint64_t m_nLastTimestamp;
    int m_nEndRunsRemaining;
    int m_nDefaultSourceId;

public:
    explicit CFragmentMaker(int defaultSourceId);
    
    EVB::FragmentHeader makeHeader(RingItem* pItem);
    
    // Getters:
    
    uint64_t getLastTimestamp() const {return m_nLastTimestamp; }
    int getEndRunsRemaining() const {return m_nEndRunsRemaining; }
private:
    void typeDependentProcessing(uint32_t type);
    uint32_t barrierType(uint32_t type);
};


#endif