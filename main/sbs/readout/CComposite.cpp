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
#include "CComposite.h"
#include <algorithm>


using namespace std;

/*!
  Adds an item to the end of the list of objects contained.
  This can be anything derived from CObject.   The
  callers are responsible for any storage management associated with
  dynamically allocated object.
 
  \param pObject  - Pointer to the object to add
*/

void
CComposite::addItem(CObject* pObject)
{
  m_items.push_back(pObject);
}

/*!
  Removes an item from this composite.  This has performance O(n)
  in the number of objects already in this list.  The removal
  is shallow in the sense that this method does not recurse
  into composites in search of the object to remove.

  \param pObject - Pointer to the object to remove.

  \note  It is a null operation to attempt to remove an object that is not
         in the collection

  \note  Naturally removing a composite object removes the entire sub-tree
         of objects managed by that composite and its children.

  \note  The caller is responsible for any storage management of dynamically
         allocated objects.
*/
void
CComposite::deleteItem(CObject* pObject)
{
  objectIterator p = find(begin(), end(), pObject);

  if (p != end()) {
    m_items.erase(p);
  }
}
/*!
   Returns a shallow begin iteration iterator on the object.  
   Shallow means that the iterator will not recurse into composites in the collection.
*/
CComposite::objectIterator
CComposite::begin()
{
  return m_items.begin();
}
/*!
  Returns an end iteration iterator to go with the begin member above:
*/
CComposite::objectIterator
CComposite::end()
{
  return m_items.end();
}

/*!
  Returns the number of top level items in the subtree managed by
  this composite. By top level we mean that the count does not recurse into
  composites.
*/
size_t
CComposite::size() const
{
  return m_items.size();
}

/*!
   Returns the number of leaf nodes this composite controls.  This is a deep count,
   that is all members that are composites are requested to add their leaf node
   count to the total.

*/
size_t
CComposite::leafCount()
{
  size_t count(0);
  objectIterator p = begin();
  while (p != end()) {
    if ((*p)->isComposite()) {
      CComposite* c = dynamic_cast<CComposite*>(*p);
      count        += c->leafCount();
    }
    else {
      count++;
    }
    p++;
  }
  return count;
}
/*!
  Returns the total number of nodes in the composite.  This counts composites
  as well as their children.  This number will be at least the value returned
  by leafCount(). The count is deep.
*/
size_t
CComposite::fullCount()
{
  size_t count(0);
  objectIterator p = begin();
  while (p != end()) {
    if ((*p)->isComposite()) {
      CComposite* c = dynamic_cast<CComposite*>(*p);
      count        += c->fullCount();
    }
    count++;			// Count composites...

    p++;
  }
  return count;
}

/*!
   This is a composite:
*/
bool
CComposite::isComposite() const
{
  return true;
}
