#ifndef NSCLDAQ10_CRINGITEM_H
#define NSCLDAQ10_CRINGITEM_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <unistd.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace DAQ
{
  namespace V10 {

struct _RingItem;

// Constants:

static const uint32_t CRingItemStaticBufferSize(8192);

/*!  
  This class is a base class for objects that encapsulate ring buffer items
  (as defined in DataFormat.h).  One interesting wrinkle is used to optimize.
  Most items will be small.  For bunches of small items, data allocation/free can
  dominate the performance over the transfer of data into smaller items.
  Therefore, each object will have a 'reasonably size' local, static storage
  for data.  If the requested body size fits in this local static storage, it will
  be used rather than doing an extra memory allocation on construction and free on
  destruction.  

  The body is meant to be filled in by getting the cursor, referencing/incrementing
  it and then storing the cursor back.

*/
class CRingItem {
  // Private data:

private:
  _RingItem*    m_pItem;
  uint8_t*      m_pCursor;
  uint32_t      m_storageSize;
  bool          m_swapNeeded;
  uint8_t       m_staticBuffer[CRingItemStaticBufferSize + 100];

  // Constructors and canonicals.

public:
  CRingItem(uint16_t type, size_t maxBody = 8192);
  CRingItem(const CRingItem& rhs);
  virtual ~CRingItem();
  
  CRingItem& operator=(const CRingItem& rhs);
  int operator==(const CRingItem& rhs) const;
  int operator!=(const CRingItem& rhs) const;



  // Selectors:

public:
  size_t getStorageSize() const;
  size_t getBodySize()    const;
  void*  getBodyPointer();
  const char* getBodyPointer() const;
  void*  getBodyCursor();
  const char* getBodyCursor() const;
  _RingItem*  getItemPointer();
  const _RingItem* getItemPointer() const;

  /*! \brief Conveniently set the body of the event to some data
   *
   * This copies the entire container into the body, sets the cursor,
   * and updates the size.
   */
  template<class T> void fillBody(const std::vector<T>& data);

  uint32_t type() const;
  uint32_t size() const;

  // Mutators:

public:
  void setBodyCursor(void* pNewCursor);

  // Object actions:

  bool mustSwap() const;
  void updateSize();		/* Set the header size given the cursor. */

  // Virtual methods that all ring items must provide:

  virtual std::string typeName() const;	// Textual type of item.
  virtual std::string toString() const; // Provide string dump of the item.

  // Utilities derived classes might want:

protected:
  
  void deleteIfNecessary();
  void newIfNecessary(size_t size);
  static std::string timeString(time_t theTime);


  // Private Utilities.
private:
  void copyIn(const CRingItem& rhs);

  
};


// Template implementation
template<class T>
void CRingItem::fillBody(const std::vector<T>& data)
{
    size_t nBytesPerItem = sizeof(typename std::vector<T>::value_type);
    size_t nBytesToCopy = nBytesPerItem*data.size();

    newIfNecessary(nBytesToCopy);

    T* pCursor = reinterpret_cast<T*>(getBodyPointer());

    size_t nElements = data.size();
    for (size_t element=0; element<nElements; ++element) {
        *pCursor = data.at(element);
        ++pCursor;
    }

    setBodyCursor(pCursor);
    updateSize();

}



} // end of V10 namespace
} // end of DAQ
#endif
