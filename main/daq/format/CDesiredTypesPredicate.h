#ifndef CDESIREDTYPESPREDICATE_H
#define CDESIREDTYPESPREDICATE_H

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

#include "CRingSelectionPredicate.h"

/*! 

   CDesiredTypesPredicate is a CRingSelectionPredicate that only selects items on the
   selection list.
*/
class CDesiredTypesPredicate : public CRingSelectionPredicate
{
  // Constructors and canonicals.

public:
  CDesiredTypesPredicate();
  CDesiredTypesPredicate(unsigned int nType, uint32_t* types);
  CDesiredTypesPredicate(unsigned int nType, uint32_t* types, bool* sample);
  CDesiredTypesPredicate(const CDesiredTypesPredicate& rhs);
  virtual ~CDesiredTypesPredicate();

  CDesiredTypesPredicate& operator=(const CDesiredTypesPredicate& rhs);
  int operator==(const CDesiredTypesPredicate& rhs) const;
  int operator!=(const CDesiredTypesPredicate& rhs) const;

  // Add a type to the selection set.

  void addDesiredType(uint32_t type, bool sample = false);

  // Virtual interface:

  virtual bool selectThis(uint32_t type);
};

#endif
