#ifndef CEVENTTRIGGER_H
#define CEVENTTRIGGER_H
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


/*!
   This is the base class of anything that can trigger anything else.
   Triggers are intended to be used inside of polling loops in the trigger thread.
   Triggers get periodically asked if they have fired and, if so the
   trigger loop dispatches to the appropriate action.

   \note This class is an abstract base class.  All concrete classes must at least
         override and implement oeprator() the function call operator that checks
	 the trigger condition.

*/
class CEventTrigger
{
public: 
  virtual void setup();
  virtual void teardown();
  virtual bool operator()() = 0;
};


#endif
