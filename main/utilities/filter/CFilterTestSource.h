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

/** @file:  CFilterTestSource.h
 *  @brief: Provide a test data source for filters.
 */
#ifndef CFILTERTESTSOURCE_H
#define CFILTERTESTSOURCE_H
#include <CDataSource.h>
#include <vector>

class CRingItem;

/**
 * @class CFilterTestSource
 *    Provides a test data source that can be stocked by test frameworks
 *    with ring  items that are then returned from it when requested.
 *    Note this is a struct -- everything is public because it's for testing
 *  
 */
struct CFilterTestSource : public CDataSource
{
    std::vector<CRingItem*> m_source;
    
    ~CFilterTestSource();
    virtual CRingItem* getItem();
    virtual void read(char* pBuffer, size_t nBytes); //*sigh*
    void addItem(CRingItem* pItem);      // Gets copy constructed into the source.
     
    
    
};

#endif