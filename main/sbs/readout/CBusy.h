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

#ifndef CBUSY_H
#define CBUSY_H

/*!
   Abstract base class for a module that manages the busy for 
   the readout framework.  The key functions are:
   - GoBusy()      - Called when data taking is stopping.
   - GoClear()     - Called when the system is able to accept a new trigger.

*/
class CBusy 
{
public:
  virtual ~CBusy();		// Needed for destructor chaining.
public:
  virtual void GoBusy() = 0;
  virtual void GoClear() = 0;
};

#endif
