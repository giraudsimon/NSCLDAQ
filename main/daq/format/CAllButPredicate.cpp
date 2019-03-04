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
#include "CAllButPredicate.h"

//////////////////////////////////////////////////////////////////////////////////
//
// Constructors and canonicals.
// Pretty much all delegate to the base class.
//

/*!
  Default constructor:
*/
CAllButPredicate::CAllButPredicate() : 
  CRingSelectionPredicate()
{
  
}
/*!
  Construct with a list of unsampled types.
*/
CAllButPredicate::CAllButPredicate(unsigned int nType, uint32_t* types) : 
  CRingSelectionPredicate( nType,  types)
{
  
}
/*!
  Construct with a list of types and a list of sampling flags.
*/
CAllButPredicate::CAllButPredicate(unsigned int nType, uint32_t* types, bool* sample) : 
  CRingSelectionPredicate(nType, types, sample)
{
  
}

/*!   
  Copy construction.

*/

CAllButPredicate::CAllButPredicate(const CAllButPredicate& rhs) :
  CRingSelectionPredicate(rhs)
{
}

/*!
  Destructor
*/
CAllButPredicate::~CAllButPredicate() 
{}

/*!
   Assignment
*/
CAllButPredicate&
CAllButPredicate::operator=(const CAllButPredicate& rhs)
{
  if (this != &rhs) {
    CRingSelectionPredicate::operator=(rhs);
  }
  return *this;
}
/*!
  Equality compare:
*/
int
CAllButPredicate::operator==(const CAllButPredicate& rhs) const
{
  return CRingSelectionPredicate::operator==(rhs);
}

/*!
  Inequality compare
*/
int
CAllButPredicate::operator!=(const CAllButPredicate& rhs) const
{
  return !(*this == rhs);
}

/*!
  Add an exception to the rule of accepting everything.
*/
void
CAllButPredicate::addExceptionType(uint32_t type, bool sample)
{
  addSelectionItem(type, sample);
}
/*!
  Return true if we need to keep hunting or false if we're done.
*/
bool
CAllButPredicate::selectThis(uint32_t type)
{
  CRingSelectionPredicate::SelectionMapIterator p = find(type);
  if (p == end() || p->second.s_sampled) {
    return false;			// Match everything.
  }
  else {
    return true;
  }
}
