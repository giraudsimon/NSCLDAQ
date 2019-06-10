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
#include "CDesiredTypesPredicate.h"


///////////////////////////////////////////////////////////////////////////////////
//
// Constructors and canonicals; they mostly delegate to the base class.
//

/*!
   Default constructor.
*/
CDesiredTypesPredicate::CDesiredTypesPredicate()
{
}
/*!
   Construct with a list of accepted unsampled types
*/
CDesiredTypesPredicate::CDesiredTypesPredicate(unsigned int nType, uint32_t* types) : 
  CRingSelectionPredicate(nType, types)
{
}
/*!   Construct with a list of types and a list of sample characteristics
 */
CDesiredTypesPredicate::CDesiredTypesPredicate(unsigned int nType, uint32_t* types, bool* sample) : 
  CRingSelectionPredicate(nType, types, sample)
{}
/*!
  Copy constructor.
*/
CDesiredTypesPredicate:: CDesiredTypesPredicate(const CDesiredTypesPredicate& rhs) :
     CRingSelectionPredicate(rhs)
{
}
/*!  Destructor. */
CDesiredTypesPredicate::~CDesiredTypesPredicate()
{}

/*!
  Assignment.
  \param rhs - The object being assigned to *this.
  \return CDesiredTypesPredicaate&
  \retval *this
*/
CDesiredTypesPredicate&
CDesiredTypesPredicate::operator=(const CDesiredTypesPredicate& rhs)
{
  if (this != &rhs) {
    CRingSelectionPredicate::operator=(rhs);
  }
  return *this;
}
/*! 
  Equality compare
  \param rhs - The object being compared to *this
  \return int
  \retval 0 - unequal
  \retval 1 - equal.
*/
int 
CDesiredTypesPredicate::operator==(const CDesiredTypesPredicate& rhs) const
{
  return CRingSelectionPredicate::operator==(rhs);
}
/*!
  Inequality.
*/
int 
CDesiredTypesPredicate::operator!=(const CDesiredTypesPredicate& rhs) const
{
  return !(*this == rhs);
}


///////////////////////////////////////////////////////////////////////////////////
//
// Implementing the interface.

/*!
   add a type to the desired set of types.
   \param type   - The item type to add
   \param sample - True if we only want the type in sampled mode (defaults to false).
*/
void
CDesiredTypesPredicate::addDesiredType(uint32_t type, bool sample)
{
  addSelectionItem(type, sample);
}

/*!
  selectThis will return true if the type provided is not in the selection list.
  This indicates to the caller it should keep looking for items.
  \param type  - The type of the current item.
*/
bool
CDesiredTypesPredicate::selectThis(uint32_t type)
{
  return find(type) == end();
}
