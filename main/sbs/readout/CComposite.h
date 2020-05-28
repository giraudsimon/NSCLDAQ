#ifndef CCOMPOSITE_H
#define CCOMPOSITE_H
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

#include "CObject.h"
#include <vector>
#include <unistd.h>



/*!
  This class extends the object base class to support composite objects.
  A composite object is simply an object that contains other objects.
  The other objects are contained in an ordered container.
  The container is optimized for sequential reference, rather than
  for insertion/removal, as in the NSCLDAQ system, sequential reference is
  the time critical operation.

  \note This class is not abstract but it is also not very functional.
        It only implements the mechanics of the composite, it is up to 
	subclasses to implement any useful functionality for the objects
	and composites.
*/
class CComposite : public CObject
{
public:
  typedef std::vector<CObject*> objectList;
  typedef objectList::iterator  objectIterator;

private:
  objectList        m_items;

public:

  void addItem(CObject* pObject);
  void deleteItem(CObject* pObject);

  // Shallow iteration:

  objectIterator begin();
  objectIterator end();
  size_t         size() const;

  size_t leafCount();	//!< # of non composites in this subtree.
  size_t fullCount();     //!< # count of all objects in this subtree.
  
  virtual bool isComposite() const;
};

#endif
