/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include <iostream>
#include <iomanip>
#include <CCompositeFilter.h>

/**! The default constructor
  Does nothing but initialize an empty vector of filters
*/ 
CCompositeFilter::CCompositeFilter()
  : m_filter()
{}

/**! The copy constructor
* Performs a deep copy of the filters. Clones of each of the filters
* in the target composite are made.
*
* \param rhs the filter to copy
*/
CCompositeFilter::CCompositeFilter(const CCompositeFilter& rhs)
  : CFilter(rhs), m_filter()
{
  const_iterator it = rhs.begin();
  const_iterator itend = rhs.end();

  while (it!=itend) {
    m_filter.push_back((*it)->clone());
    ++it;
  } 
}

/**! Assignment 

  Performs a deep copy of the target. The target's state is copied 
  entirely prior to assignment. Following this operation, this will not own pointers 
  to the same objects as were pointed to by the target's pointers. Instead, 
  the various pointers will refer to copies of the objects pointed to by the 
  target's pointers.

  \param rhs the object to copy 
  \return a reference to this after the copy operation
*/
CCompositeFilter& CCompositeFilter::operator=(const CCompositeFilter& rhs)
{
  if (this != &rhs) {
    const_iterator it = rhs.begin();
    const_iterator itend = rhs.end();

    // First fill a vector with all the newly
    // copied filters. If this fails, then we know the 
    // original filters were not modified.
    FilterContainer copy;
    while (it!=itend) {
      copy.push_back((*it)->clone());
      ++it;
    } 

    // We made it here so clear our container
    // and copy all of the new pointer values
    clearFilters();
    m_filter = copy;
  }

  return *this;
}

/**! Destructor
  1. Frees all dynamically allocated objects
  2. Clears the container holding the pointers to those objects.
*/
CCompositeFilter::~CCompositeFilter()
{
  clearFilters();
}

/**! Virtual copy constructor
  Returns a dynamically allocated object whose state is a copy of this.
  Ownership of the returned object belongs to the caller. The semantics 
  of the copy operation for this class is described in the
  CCompositeFilter::CCompositeFilter(const CCompositeFilter&) method.

  \return a pointer to the new dynamically allocated object
*/
CCompositeFilter* CCompositeFilter::clone() const 
{
  return new CCompositeFilter(*this);
}

/**! Append a filter to the container
*
   By registering a filter to this, a copy of the filter is actually
   registered. Filters are appended to the back of the container every
   time this method is called. As a result, the order of registration
   is preserved at execution time.

  \param filter a template of the filter to register
*/
void CCompositeFilter::registerFilter(const CFilter* filter)
{
  m_filter.push_back(filter->clone());
}

/**! Handle a generic ring item
    This is handler effectively just iterates through the set of 
    registered filters and calls their respective handleRingItem(const CRingItem*)
    methods. Memory is managed during the iterations such that each subsequent 
    filter receives as input the output of the previous. It is possible for a filter
    to return a pointer to the same object as was passed it and this is handled properly.
    
  \param item the ring item to process
  \return a pointer to the output of the last filter registered to the composite
*/  
CRingItem* CCompositeFilter::handleRingItem(CRingItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  // Loop through the filters while the newly returned object isn't null
  while (it!=itend && pItem0!=0) {

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleRingItem(pItem0);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle state change items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleStateChangeItem(const CRingStateChangleItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handleStateChangeItem(CRingStateChangeItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CRingStateChangeItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CRingStateChangeItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleStateChangeItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle scaler items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleScalerItem(const CScalerItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handleScalerItem(CRingScalerItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CRingScalerItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CRingScalerItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleScalerItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle text items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleTextItem(const CTextItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handleTextItem(CRingTextItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CRingTextItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CRingTextItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleTextItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handlePhysicsEventItem(const CPhysicsEventItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handlePhysicsEventItem(CPhysicsEventItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CPhysicsEventItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CPhysicsEventItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handlePhysicsEventItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handlePhysicsEventCountItem(const CRingPhysicsEventCountItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CRingPhysicsEventCountItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CRingPhysicsEventCountItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handlePhysicsEventCountItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Handle physics event items

   See handleRingItem(const CRingItem*) documentation above. The exact same thing
   is done except that handleFragmentItem(const CFragmentItem*) is 
   called.
*/
CRingItem* CCompositeFilter::handleFragmentItem(CRingFragmentItem* item)
{
  iterator it = begin(); 
  iterator itend = end(); 

  // Initialize some pointers to keep track of returned objects
  CRingItem* pItem0=item;
  CRingItem* pItem1=pItem0;
  CRingFragmentItem* state_item = 0;
  while (it!=itend && pItem0!=0) {

    state_item = static_cast<CRingFragmentItem*>(pItem0);

    // pass the first item to the filter and get the filtered item 
    pItem1 = (*it)->handleFragmentItem(state_item);

    // if the filter returned a different object than it was given
    // delete the one it was given 
    if (pItem1 != pItem0 && pItem0!=item) {
      delete pItem0;
    }
  
    // Update the pointer
    pItem0 = pItem1;

    // increment to the next filter
    ++it;
  }
  return pItem0;
}

/**! Clear and resize filter container
    Frees all memory pointed to by pointers in the container and
    then resizes the container to zero size.
*/
void CCompositeFilter::clearFilters()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    delete *it;
    ++it;
  }
  m_filter.clear();
}

/**! Initialization hook to run before any data is processed
*/
void CCompositeFilter::initialize()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    (*it)->initialize();
    ++it;
  }
}

/**! Finalization hook to run after all data is processed
*/
void CCompositeFilter::finalize()
{
  iterator it    = begin();
  iterator itend = end();

  while (it!=itend) {
    (*it)->finalize();
    ++it;
  }
}

