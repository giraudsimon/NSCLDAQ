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

#include <config.h>
#include "V10/CRingItem.h"
#include "V10/DataFormatV10.h"
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <utils.h>

namespace DAQ {
  namespace V10 {


////////////////////////////////////////////////////////////////////////////////
//
// Constructors and other canonicals.
//

/*!
   Construct the ring item:
   - If the maxbody is larger than CRingItemStaticBufferSize, allocate
     a new buffer and point the item at that, otherwise point it at 
     m_staticBuffer.
   - Pointer m_cursor at the body of the ring item.
   - Calculate and fill in the storage size.
   - Set m_swapNeeded to false.
   - Set the ring item type.

   \param type - The ring item type.  This is only 16 bits wide but stored in a
                 32 bit word so receivers can determine if bytes were swapped.
   \param maxBody - Largest body we can hold

*/
CRingItem::CRingItem(uint16_t type, size_t maxBody) :
  m_pItem(reinterpret_cast<RingItem*>(&m_staticBuffer)),
  m_storageSize(maxBody),
  m_swapNeeded(false)
{

  // If necessary, dynamically allocate (big max item).

  newIfNecessary(maxBody);
  m_pItem->s_header.s_type = type;
  m_pItem->s_header.s_size = 0;
}
/*!
  Copy construct.  This is actually the same as the construction above, 
  however the item contents get memcpied into our ring body. The caller has to
  ensure that m_pCursor reflects the amount of data copied into the ring body.

  \param rhs  - The source of the copy.
*/
CRingItem::CRingItem(const CRingItem& rhs)
{
  // If the storage size is big enough, we need to dynamically allocate
  // our storage

  newIfNecessary(rhs.m_storageSize);

  copyIn(rhs);
}
/*!
    Destroy the item. If the storage size was big, we need to delete the 
    storage as it was dynamically allocated.
*/
CRingItem::~CRingItem()
{
  deleteIfNecessary();
}

/*!
   Assignment.  If necessary destroy the body.  If necessary re-create the body.
   After that it's just a copy in.

   \param rhs  - The object that we are duplicating into *this.
*/
CRingItem&
CRingItem::operator=(const CRingItem& rhs)
{
  if (this != &rhs) {
    deleteIfNecessary();
    newIfNecessary(rhs.m_storageSize);
    copyIn(rhs);
  }

  return *this;
}

/*!
   Comparison for equality.. note that true equality may be time consuming
   to determine, as it requires the contents of the items to be equal which will
   require linear time in the size of the item.
   \param rhs  - refers to the item to be compared with *this>
   \return int
   \retval 0   - Not equal\
   \retval 1   - equal

*/
int
CRingItem::operator==(const CRingItem& rhs) const
{
  // short cut by looking at storage size and swap characteristics first:

  if (m_storageSize != rhs.m_storageSize) return 0;
  if (m_swapNeeded  != rhs.m_swapNeeded ) return 0;

  // Now there's nothing for it but to compare the contents:

  return (memcmp(m_pItem, rhs.m_pItem, sizeof(RingItemHeader) + m_storageSize) == 0);
}
/*!
  Inequality is just the logical inverse of equality.  This can take time, see
  operator==

  \param rhs  - Reference to the object *this will be compared to.
  \return int
  \retval 0   - Objects are not unequal
  \retval 1   - Objects are unequal
*/
int
CRingItem::operator!=(const CRingItem& rhs) const
{
  return !(*this == rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Selectors:

/*!

   \return size_t
   \retval The size allocated to the ring item's body.
*/
size_t
CRingItem::getStorageSize() const
{
  return m_storageSize;
}

/*!
   \return size_t
   \retval Amount of data in the body.  This is the difference between the 
           cursor and the start of the body.
*/
size_t
CRingItem::getBodySize() const
{
  return (m_pCursor - m_pItem->s_body);
}
/*!
  \return void*
  \retval Pointer to the body of the ring item. To get the next insertion point,
          see getBodyCursor.
*/
void*
CRingItem::getBodyPointer() 
{
  return m_pItem->s_body;
}

const char*
CRingItem::getBodyPointer() const
{
  return reinterpret_cast<const char*>(m_pItem->s_body);
}
/*!
   \return void*
   \retval Pointer to the next insertion point of the body
*/
void*
CRingItem::getBodyCursor()
{
  return m_pCursor;
}
const char*
CRingItem::getBodyCursor() const
{
  return reinterpret_cast<const char*>(m_pCursor);
}

/*!
   \return void*
   \retval Pointer to the body.  To be usually used by derived classes but...
*/
pRingItem
CRingItem::getItemPointer()
{
  return m_pItem;
}

/*!
   \return void*
   \retval Pointer to the body.  To be usually used by derived classes but...
*/
const _RingItem*
CRingItem::getItemPointer() const 
{
  return m_pItem;
}

/*!
   \return uint32_t
   \retval Current type of the item.
*/
uint32_t
CRingItem::type() const
{
  uint32_t rawType = m_pItem->s_header.s_type;
  if (mustSwap()) {
    return swal(rawType);
  }
  else {
    return rawType;
  }
}

uint32_t
CRingItem::size() const
{
  uint32_t size = m_pItem->s_header.s_size;
  if (mustSwap()) {
    return swal(size);
  }
  else {
    return size; 
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Mutators.

/*!
  Update the body cursor to reflect data that was put in the item.
  \param pNewCursor - New vlaue for the body cursor.
 
*/
void
CRingItem::setBodyCursor(void* pNewCursor)
{
  m_pCursor = reinterpret_cast<uint8_t*>(pNewCursor);
}

/*!
** Given the current item cursor set the size of the item.
*/
void
CRingItem::updateSize()
{
  m_pItem->s_header.s_size = sizeof(RingItemHeader) + getBodySize();
}
///////////////////////////////////////////////////////////////////////////////////////
//
//   Object operations.


/*!
   This is primarily intended for items that have been constructed via
   the getFromRing static member.
   \return bool
   \retval true - The byte order on the generating system is different from that of
                  this system.
   \retval false - The byte order on the generating system is the same as that of this
                  system.
*/
bool
CRingItem::mustSwap() const
{
  return m_swapNeeded;
}


//////////////////////////////////////////////////////////////////////////////////////
//
// Default implementations of virtual methods:
//

/**
 * typeName
 *
 *  Return an std::string that contains the name of the ring item type
 *  (e.g. "Physics data").  The default produces:
 *  "Unknown (0xnn) where 0xnn is the hexadecimal ring item type.
 *
 * @return std::string containing the type as described above.
 */

std::string
CRingItem::typeName() const
{
  std::stringstream typeStr;
  typeStr << "Unknown (" << std::hex << type() << ")"; 
  return typeStr.str();
}   
/**
 * toString
 *
 * Return an std::string that contains a formatted dump of the ring item
 * body. Default implementation just produces a hex-dump of the data
 * the dump has 8 elements per line with spaces between each element
 * and the format of each element is %02x.
 *
 * @return std::string - the dump described above.
 */
std::string
CRingItem::toString() const
{
  std::stringstream  dump;
  const uint8_t*      p     = m_pItem->s_body;
  size_t              n     = getBodySize(); 
  int                 nPerLine(8);

  dump << std::hex << std::setw(2) << std::setfill('0');

  for (int i = 0; i < n; i++) {
    dump << static_cast<unsigned int>(*p++) << " ";
    if ((i > 0) && ((i % nPerLine) == 0)) {
      dump << std::endl;
    }
  }
  // If there's no trailing endl put one in.

  if (n % nPerLine) {
    dump << std::endl;
  }
  

  return dump.str();
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  Static class methods.


///////////////////////////////////////////////////////////////////////////////////////
//
// Private utilities.
//


/*
 * Common code for copy construction and assignment,
 * copies the contents of some source item into *this.
 * We assume that the final values for m_pCursor and m_pItem
 * have already been set.
 */
void
CRingItem::copyIn(const CRingItem& rhs)
{
  m_storageSize   = rhs.m_storageSize;
  m_swapNeeded  = rhs.m_swapNeeded;
  memcpy(m_pItem, rhs.m_pItem, 
	 rhs.m_pItem->s_header.s_size + sizeof(RingItemHeader)); ///   m_storageSize + sizeof(RingItemHeader));

  // where copyin is used, our cursor is already pointing at the body of the item.
  // therefore when updating it we need to allow for that in the arithmetic below.

  m_pCursor    =  reinterpret_cast<uint8_t*>(getBodyPointer())
                 +  m_pItem->s_header.s_size 
                 - sizeof(RingItemHeader); // Add the size of the body to the cursor position.
}


/*
 *   If necessary, delete dynamically allocated buffer space.
 */
void 
CRingItem::deleteIfNecessary()
{
  if (m_storageSize > CRingItemStaticBufferSize) {
    delete [](reinterpret_cast<uint8_t*>(m_pItem));
  }
}
/*
 *  If necessary, create dynamically allocated buffer space and point
 * m_pItem at it.
 */
void
CRingItem::newIfNecessary(size_t size)
{
  if (size > CRingItemStaticBufferSize) {
    m_pItem  = reinterpret_cast<RingItem*>(new uint8_t[size + sizeof(RingItemHeader) + 100]);
  }
  else {
    m_pItem = reinterpret_cast<RingItem*>(m_staticBuffer);
  }
  m_pCursor= m_pItem->s_body;

}

/**
 * timeString
 *
 * Given a time_t time, returns a string that is the textual time (ctime()).
 *
 * @param theTime - time gotten from e.g. time(2).
 * 
 * @return std::string textified time
 */
std::string
CRingItem::timeString(time_t theTime) 
{

  std::string result(ctime(&theTime));
  
  // For whatever reason, ctime appends a '\n' on the end.
  // We need to remove that.

  result.erase(result.size()-1);

  return result;
}

} // end of V10 namespace
} // end DAQ
