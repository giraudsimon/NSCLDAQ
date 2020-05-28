#ifndef CALLBUTPREDICATE_H
#define CALLBUTPREDICATE_H


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
   This predicate accepts all types except the non sampled items in the list.
   The sampled items in the list are accepted with sampling.
*/
class CAllButPredicate : public CRingSelectionPredicate
{
public:
  CAllButPredicate();
  CAllButPredicate(unsigned int nType, uint32_t* types);
  CAllButPredicate(unsigned int nType, uint32_t* types, bool* sample);
  CAllButPredicate(const CAllButPredicate& rhs);
  virtual ~CAllButPredicate();

  CAllButPredicate& operator=(const CAllButPredicate& rhs);
  int operator==(const CAllButPredicate& rhs) const;
  int operator!=(const CAllButPredicate& rhs) const;

  // add an exception type:
public:
  void addExceptionType(uint32_t type, bool sample = false);

  // virtual interface:

public:
  virtual bool selectThis(uint32_t type);
};

#endif
