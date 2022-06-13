#ifndef CNULLTRIGGER_H
#define CNULLTRIGGER_H
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

#include <CEventTrigger.h>


/*!
  The simplest trigger is one that never fires.  That is a null trigger.
  This  is useful in some test environments.   This class implements the
  null trigger.
*/
class CNullTrigger : public CEventTrigger
{
 public:
  virtual bool operator()();
  
  
};

#endif
