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

/** @file:  CFilterTestSink.cpp
 *  @brief: Implement a test data sink for filter testing.
 */
#include <CFilterTestSink.h>
#include <CRingItem.h>
#include <CRingItemFactory.h>

/**
 * destructor - delete all of the entries in the sink vector.
 */
CFilterTestSink::~CFilterTestSink()
{
    while(!m_sink.empty()) {
        CRingItem* pBack = m_sink.back();
        delete pBack;
        m_sink.pop_back();
    }
}
/**
 * putItem
 *   @item - ring item to put.
 */
void
CFilterTestSink::putItem(const CRingItem& item)
{
    CRingItem* p = new CRingItem(item);
    m_sink.push_back(p);
}

/**
 * put
 *   Normally this is an unstructured put.
 *   We are going to require/assume, however that
 *   pData points to a raw ring item.
 *
 *   @param pData - pointer to the raw ring item.
 *   @param nBytes - Pointer to the size of the ring item.
 *                  This is actually ignored with the size taken
 *                  from the raw item.
 *           
 */
void
CFilterTestSink::put(const void* pData, size_t nBytes)
{
    CRingItem* pItem = CRingItemFactory::createRingItem(pData);
    m_sink.push_back(pItem);
}