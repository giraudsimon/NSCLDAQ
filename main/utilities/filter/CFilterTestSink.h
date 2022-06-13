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

/** @file:  CFilterTestSink.h
 *  @brief: Data Sink used by CFilter tests.
 */
#ifndef CFILTERTESTSINK_H
#define CFILTERTESTSINK_H
#include <CDataSink.h>
#include <vector>

class CRingItem;


/**
 * @class CFilterTestSink
 *    Provides a test data sink that just takes data put into
 *    it and turns it into a vector of dynamically constructed
 *    ring items.  This is a struct since  it's intended for testing
 *    anyway.
 */
struct CFilterTestSink : public CDataSink
{
    std::vector<CRingItem*>   m_sink;  // where data goes when sunk.
    virtual ~CFilterTestSink();
    
    void putItem(const CRingItem& item);
    void put(const void* pData, size_t nBytes);
};

#endif