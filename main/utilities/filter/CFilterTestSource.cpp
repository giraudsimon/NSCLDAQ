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

/** @file:  CFilterTestSource.cpp
 *  @brief: Implement a test data source for filters.
 */
#include "CFilterTestSource.h"
#include "CRingItem.h"

/**
 * destructor -- delete any remaining items in m_source.
 */
CFilterTestSource::~CFilterTestSource()
{
    // Deleting from the back for a vector is notionally faster though
    // it probably does not matter in this setting.
    
    while (!m_source.empty()) {
        CRingItem* pBack = m_source.back();
        delete pBack;
        m_source.pop_back(); 
    }
}

/**
 * getItem
 *   @return CRingItem* - front item from the vector.
 *   @retval nullptr    - m_source is empty.
 *   
 */
CRingItem*
CFilterTestSource::getItem()
{
    if (m_source.empty()) {
        return nullptr;
    } else {
        CRingItem* result = m_source.front();
        m_source.erase(m_source.begin());
        return result;
    }
}
/**
 * read
 *    unused by this but needed to make the class non-abstract
 *    *sigh*
 */
void
CFilterTestSource::read(char* pBuffer, size_t nBytes)
{
    
}
/**
 * addItem
 *    Adds an item to the back of the source data.
 *
 * @param pItem - ring item to copy construct to and append to
 *                the vector.
 */
void
CFilterTestSource::addItem(CRingItem* pItem)
{
    CRingItem* pNewItem = new CRingItem(*pItem); // Copy construct
    m_source.push_back(pNewItem);
}

