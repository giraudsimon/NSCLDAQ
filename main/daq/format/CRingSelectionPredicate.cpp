
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

#include "CRingSelectionPredicate.h"
#include <CRingBuffer.h>
#include "DataFormat.h"

using namespace std;

static const size_t DEFAULT_HIGH_WATER(2*1024*1024); // Default high water mark.

//////////////////////////////////////////////////////////////////////
// Constructors and other canonicals.
//

/*!
  Default constructor.  The object is created with an empty selection list.
  Actual predicates will usually define a mechanism for manipulating the 
  selection that has an appropriate name for how the selection is used.
*/
CRingSelectionPredicate::CRingSelectionPredicate() : 
  m_highWaterMark(DEFAULT_HIGH_WATER)
{
}

/*!  
  Construct a selection predicate with a set of unsampled items.
 \param nType      - Number of items in the selection.
 \param uint32_t*  - C array of types.
*/
CRingSelectionPredicate::CRingSelectionPredicate(unsigned int nTypes,
						 uint32_t* types) :
  m_highWaterMark(DEFAULT_HIGH_WATER)
{
  vector<ItemType> selection;
  for (int i =0; i < nTypes; i++) {
    ItemType anItem = {false, types[i]};
    selection.push_back(anItem);
  }
  addSelectionItems(selection);
}
/*!
  Construct a selection given a set of item types and their 
  corresponding sample/nosample parameter.
  \param nTypes    - Number of types to add to the selection.
  \param type      - POinter to an array of types.
  \param sample    - Pointer to an array of sampling specifications.

*/
CRingSelectionPredicate::CRingSelectionPredicate(unsigned int nTypes,
						 uint32_t* type,
						 bool*     sample) :
    m_highWaterMark(DEFAULT_HIGH_WATER)
{
  vector<ItemType> selection;
  for (int i =0; i < nTypes; i++) {
    ItemType anItem = {sample[i], type[i]};
    selection.push_back(anItem);
  }
  addSelectionItems(selection);
}
/*!
  Copy construction.
*/
CRingSelectionPredicate::CRingSelectionPredicate(const CRingSelectionPredicate& rhs) :
  m_highWaterMark(rhs.m_highWaterMark),
  m_selections(rhs.m_selections)
{
  
}
/*!
  Destructor
*/
CRingSelectionPredicate::~CRingSelectionPredicate()
{
}

/*!
   \param rhs objec that will be copied into *this.
   \return CRingSelection&
   \retval reference to *this after rhs has been copied i.
*/
CRingSelectionPredicate&
CRingSelectionPredicate::operator=(const CRingSelectionPredicate& rhs)
{
  if (this != &rhs) {
    m_highWaterMark = rhs.m_highWaterMark;
    m_selections = rhs.m_selections;
  }
  return *this;
}
/*!
  Equality is true if the selection lists are the same:

  \param rhs object that will be compared to *this.
  \return int
  \retval 0  - Objects are not equal.
  \retval 1  - Objects are equal.
*/
int 
CRingSelectionPredicate::operator==(const CRingSelectionPredicate& rhs) const
{
  return ((m_highWaterMark == rhs.m_highWaterMark) && (m_selections == rhs.m_selections));
}

/*!
  Inequality comparison.
  \param rhs the object that will be compared to *this.
  \return int
  \retval !operator==
*/
int
CRingSelectionPredicate::operator!=(const CRingSelectionPredicate& rhs) const
{
  return !(*this == rhs);
}

/*!
   This function gets the current high water mark for the predicate.
   When one is getting a sampled data type from the ring and the 
   free put space is less than this, the predicate will start skipping.
   @return size_t
   @retval the high watermark in bytes.
*/
size_t
CRingSelectionPredicate::getHighWaterMark() const
{
  return m_highWaterMark;
}

/*!
  Set a new high water mark.  See getHighWaterMark() for a description of this
  tuning parameter.
  @param newValue - New number of bytes in the high water mark value.
  @return size_t
  @retval number of bytes in previous high watermark.
*/
size_t
CRingSelectionPredicate::setHighWaterMark(size_t newValue) 
{
  size_t old = m_highWaterMark;
  m_highWaterMark = newValue;

  return old;
}
/*!
   Called by the predicate evaluator.  Returns
   true if we need to be called again or false if not.
   The assumption is that this is a predicate defined on the 
   type of a ring item. We'll be responsible for picking out the
   size of the item, and the type, selectThis will be responsible for
   telling us if the type should be skipped (true) or returned (false).
   And we'll be responsible for skipping the type if needed.

   Any blocking is done here so that we don't block for each
   item checked.

   \param ring - reference to the ring buffer we are acting on.


*/
bool
CRingSelectionPredicate::operator()(CRingBuffer& ring)
{
  // We need to have at least a header:

  if (ring.availableData() < sizeof(RingItemHeader)) {
    ring.pollblock();		// Give it a while for data to arrive.
    return true;
  }

  // Peek the header, decode the size and type (may need byteswapping).


  RingItemHeader header;
  ring.peek(&header, sizeof(header));

  uint32_t size = itemSize(reinterpret_cast<pRingItem>(&header));
  uint32_t type = itemType(reinterpret_cast<pRingItem>(&header));
  
  
  // Now that we have a header don't allow us to progress further if we don't
  // also have the full event:

  if (ring.availableData() < size) {
    ring.pollblock();
    return true;
  }

  // Select this really is skip this.  If we should skip the item;
  // - First if the entire item has not yet made it into the ring,
  //   return true, sleep  a bit ant return true, as we'll get called
  //   again to skip this item once it's all in.
  // - If we do have the entire item, then skip it and:
  //   * if the ring is empty wait a bit before returning to give more data
  //     a chance to arrive else
  //   * return again to get us called to look at the next item.
  // 
  if (selectThis(type)) {

      // full item in ring.

    ring.skip(size);	// Skip the ring item.

    if (!ring.availableData()) { // block for more data to come in.
      ring.pollblock();
    }
    return true;
  }
  else {
    // We have a type match.. What we do now depends on the state of the
    // sample flag.  If consuming the item would leave us with more putspace
    // that the higgwater mark
    //  we can return false, otherwise, true.
    // If the type is not in the selection list we can also return false.
    //

    CRingBuffer::Usage usage = ring.getUsage();
    size_t freeSpace     = usage.s_putSpace;
    size_t availableData = ring.availableData();

    SelectionMapIterator p = find(type);
    if (p == end()) {
      return false;
    }
    if (p->second.s_sampled) {
      if (freeSpace     >= m_highWaterMark) {
        return false;		// full item is in ring, and below high water.
      }  else if (availableData < header.s_size) {
        // Full item not yet in ring so wait and return true to try again.
      
        ring.pollblock();
        return true;
      } else if (freeSpace < m_highWaterMark) {
        ring.skip(header.s_size); // no need to block here.
        return true;
      }
    } else {

      // Else return false so that the ultimate caller can fetch the data:

      return false;
    }
  }
  return false;   
}

/*!
  Select an item that matches the prediate from the ring buffer.
  The caller will block until a matching item is found in the ring.
  Non matching items are transparently skipped.
  Once a match is found, the function returns.

  \param ring - Reference to the ring buffer.

  \note - At this time there's no capability for expressing a timeout.
*/
void
CRingSelectionPredicate::selectItem(CRingBuffer& ring)
{
  ring.While(*this);

}

///////////////////////////////////////////////////////////////////////////////
//
// Protected utility functions are intended for use by derived classes.
//

/*
 *  Adds a selection to the selection list.  This is protected so that derived
 *  classes can provide sensible names like  AddToSelection or RemoveFromSelection
 *  depending on what the selection actually means.
 *  Parameters:
 *    uint32_t type   - The type of the item to add
 *    bool     sample - True if sampling of that item type is desired. This defaults to false.
 *
*/
void
CRingSelectionPredicate::addSelectionItem(uint32_t type, bool sample)
{
  ItemType item = {sample, type};
  m_selections[type] = item;
}
/*
 *  Locates a selection item by type. If this fails, end() is returned.
 *  Parameters:
 *      type  -  Type of the item being sought in the selection map.
 *  Returns
 *    An iterator to the selection map which either 'points' to the entry
 *    or, if the entry is not found is the same as that returned from end().
 */
CRingSelectionPredicate::SelectionMapIterator
CRingSelectionPredicate::find(uint32_t type)
{
  return m_selections.find(type);
}
/*
 *  Returns an iterator to the end of the selection map.
 *  This value is used to determine if a search with find failed.
 *
 */
CRingSelectionPredicate::SelectionMapIterator
CRingSelectionPredicate::end()
{
  return m_selections.end();
}
/*
 * Does a byte swap on a longword.
 */
uint32_t 
CRingSelectionPredicate::longswap(uint32_t input)
{
  union {
    uint32_t longword;
    uint8_t  bytes[4];
  } swapper;
  swapper.longword = input;
  uint32_t result=0;

  for (int i=0; i < 4; i++) {
    result |=  swapper.bytes[i] << (3-i)*8;
  }
  return result;
}
/*
 * Add a bunch of items to the selection
 */
void 
CRingSelectionPredicate::addSelectionItems(vector<ItemType> selections)
{
  for (int  i =0; i < selections.size(); i++) {
    m_selections[selections[i].s_itemType] = selections[i];
  }
}
