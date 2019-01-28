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
#include <CRingBlockReader.h>
#include <string.h>
#include <stdlib.h>


#include <list>

// Fake individual item data source.

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

// Fake block source:

// A fake block data source:

class CFakeRingBlockReader : public CRingBlockReader
{
private:
  void*  m_pData;
  
  size_t m_nTotalBytes;
  size_t m_nBytesRead;
  

  void*  m_pRead;
public:
  CFakeRingBlockReader() :
    m_pData(nullptr), m_nTotalBytes(0), m_nBytesRead(0),
    m_pRead(nullptr)
  {}
  virtual ~CFakeRingBlockReader() {free(m_pData);}
  
  void addData(void* pBytes, size_t nBytes)
  {
    m_pData = realloc(m_pData, m_nTotalBytes + nBytes);
    
    // COpy the new data in:
    
    uint8_t* pWriteCursor = static_cast<uint8_t*>(m_pData);
    pWriteCursor += m_nTotalBytes;
    memcpy(pWriteCursor, pBytes, nBytes);
    
    m_nTotalBytes += nBytes;
    
    // Now set the new read pointer as the base address may have been
    // modified:
    
    uint8_t* pRead = static_cast<uint8_t*>(m_pData);
    pRead += m_nBytesRead;
    m_pRead = pRead;
    
    
  }

protected:
  virtual ssize_t readBlock(void* pBuffer, size_t nBytes)
  {
    ssize_t result  = 0;
    size_t bytesLeft = m_nTotalBytes - m_nBytesRead;
    if (bytesLeft) {
      nBytes =  nBytes <= bytesLeft ? nBytes : bytesLeft; // Don't over read.
      memcpy(pBuffer, m_pRead, nBytes);
      result = nBytes;
      
      // Do the book keeping
    
      m_nBytesRead += nBytes;
      uint8_t*   p  = static_cast<uint8_t*>(m_pRead);
      m_pRead       = p + nBytes;
    }
  
    return result;
  }

  
};



#endif