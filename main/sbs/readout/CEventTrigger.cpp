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
#include "CEventTrigger.h"

/*!
   This member function can be overridden to do one-time
   setup of the trigger to make the hardware (if there is any) 
   sensitive to the trigger.  Once this has been called, the 
   expectation is that operator() will be repeatedly called, and then
   some (long) time later, teardown will be called.

   The default operation, supplied here, does nothing, and is supplied to make
   the effort of building a trigger that has no need for setup less onerous.
*/
void
CEventTrigger::setup()
{
}

/*!
   This member function can be overridden to disable the trigger from 
   responding to new external (or soft) events.  The expectation is that
   after checking for triggers for some time, this will be called
   at the end of data taking.
 
   The default operation, supplied here is null, and is supplied to make
   the effort of building triggers that need no teardown less onerous.
*/
void
CEventTrigger::teardown()
{
  
}
/*!
  This pure member function must be implemented by concrete
  trigger subclasses.  It is expected to implement this function
  to report true when a trigger condition exists.

  \return bool
  \retval true  - A trigger condition exists.
  \retval false - No trigger condition exists.

  \note If post trigger actions are needed to make the hardware sensitive
        to the next trigger, they  must be done in this function prior to returning
	true.
*/
bool
CEventTrigger::operator()()	// = 0
{
  return false;			// Null trigger.
}
