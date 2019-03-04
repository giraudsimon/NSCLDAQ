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
#include "CNullTrigger.h"

/*!
   The null trigger's poll function  always returns false.
   This is exactly what the base class does:
*/
bool
CNullTrigger::operator()()
{
  return CEventTrigger::operator()();
}
