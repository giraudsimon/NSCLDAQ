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

/** @file:  fakedatasource.h
 *  @brief: Defines a fake data source for tests.
 */

#ifndef fakedatasource_h
#define fakedatasource_h
#include <CDataSource.h>
#include <list>

class CRingItem;

class CFakeDataSource : public CDataSource
{
private:
  std::list<CRingItem*> m_items;
public:
  virtual ~CFakeDataSource() {}
  void addItem(CRingItem* pItem)     // Must have been new'd.
  {
    m_items.push_back(pItem);
  }
  CRingItem* getItem() {
    CRingItem* result(nullptr);    // result if there's nothing left.
    
    if(!m_items.empty()) {         // but if there is...
      result = m_items.front();
      m_items.pop_front();
    }
    
    return result;
  }
  void read(char* pBuffer, size_t nBytes) {}
};

#endif