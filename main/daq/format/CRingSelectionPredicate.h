#ifndef CRINGSELECTIONPREDICATE_H
#define CRINGSELECTIONPREDICATE_H


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

#include <CRingBuffer.h>
#include <stdint.h>
#include <map>
#include <vector>


class CRingBuffer;

/*!
   This file implemenets the base class for a selection predicate.
   Selection predicates skip unwanted data items in ring buffers, blocking as needed until
   a desired item is encountered.

   This base class has quite a bit of the mechanisms of predicates that select 
   items on the basis of their item type.


*/
class CRingSelectionPredicate : public CRingBuffer::CRingBufferPredicate
{
  // internal types:

protected:
  typedef struct _ItemType {
    bool      s_sampled;
    uint32_t  s_itemType;
    _ItemType& operator=(const _ItemType& rhs) {
      s_sampled = rhs.s_sampled;
      s_itemType= rhs.s_itemType;
      return *this;
    }
    int operator==(const _ItemType& rhs) const {
      return (s_sampled == rhs.s_sampled)   &&
	(s_itemType == rhs.s_itemType);
    }
    int operator!=(const _ItemType& rhs) const {
      return !(*this == rhs);
    }
  } ItemType, *pItemType;

  typedef std::map<uint32_t, ItemType> SelectionMap;
  typedef SelectionMap::iterator       SelectionMapIterator;

  //  per object data:

private:
  SelectionMap  m_selections;
  size_t        m_highWaterMark; // When to start skipping for sampled data.

  // Constructors and canonicals.

public:
  CRingSelectionPredicate();
  CRingSelectionPredicate(unsigned int nType, uint32_t* types);
  CRingSelectionPredicate(unsigned int nType, uint32_t* type, bool* sample);
  CRingSelectionPredicate(const CRingSelectionPredicate& rhs);
  virtual ~CRingSelectionPredicate();

  CRingSelectionPredicate& operator=(const CRingSelectionPredicate& rhs);
  int operator==(const CRingSelectionPredicate& rhs) const;
  int operator!=(const CRingSelectionPredicate& rhs) const;


  // Tuning parameters:

  size_t getHighWaterMark() const;
  size_t setHighWaterMark(size_t newValue);

  // Predicate virtual interface.


  virtual bool operator()(CRingBuffer& ring);
  virtual bool selectThis(uint32_t type) = 0;
  void selectItem(CRingBuffer& ring);
  size_t getNumberOfSelections() const { return m_selections.size(); }

  // Utilities for derived classes:
protected:
  void addSelectionItem(uint32_t type, bool sample = false);
  SelectionMapIterator find(uint32_t type);
  SelectionMapIterator end();
  uint32_t longswap(uint32_t input);
  void addSelectionItems(std::vector<ItemType> selections);
};

#endif
