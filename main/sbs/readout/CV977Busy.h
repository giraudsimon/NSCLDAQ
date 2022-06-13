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

#ifndef CV977BUSY_H
#define CV977BUSY_H

#include "CBusy.h"
#include <stdint.h>

class CCAENV977;


/*!
    Concrete busy class that mangest he CAEN V977 as a busy module.
    The assumption is that the busy is the OR of all output bits...set by any input bit.
    (this dovetails nicely with CV977Trigger).  Our go clear will clear all bits in
    in the output register.   At construction time we'll set the module up the
    same way CV977Trigger is setup.
*/
class CV977Busy : public CBusy
{
private:
  CCAENV977&   m_busy;

public:
  // We can construct from base/crate a module object and do copy construction.

  CV977Busy(uint32_t base, unsigned crate=0);
  CV977Busy(CCAENV977& module);
  CV977Busy(const CV977Busy& rhs);


  // The following are not implemented:
private:
  CV977Busy& operator=(const CV977Busy& rhs); // can't dup stuff with references.
  int operator==(const CV977Busy& rhs) const; // comparison is senseless.
  int operator!=(const CV977Busy& rhs) const;

public:
  // THe busy interface:

  virtual void GoBusy();
  virtual void GoClear();

private:
  void initialize();

};


#endif
