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

#ifndef CCAENV262BUSY_H


#include "CBusy.h"
#include <stdint.h>


class CCaenIO;

/*!
    Provide busy status ofor a CAEN V269 module
    Uses the following signals:
    - SHP0   - Computer going busy. (software busy)
    - SHP1   - Computer going ready
    - SHP3   - Module clears.  Issued just prior to going ready.
*/
class CCAENV262Busy : public CBusy
{
private:
  CCaenIO&   m_busy;
public:
  CCAENV262Busy(uint32_t base, unsigned crate = 0);
  CCAENV262Busy(CCaenIO&  module);
  CCAENV262Busy(const CCAENV262Busy& rhs);


  // Comparisons don't make sense

private:
  CCAENV262Busy& operator=(const CCAENV262Busy& rhs);
  int operator==(const CCAENV262Busy& rhs) const;
  int operator!=(const CCAENV262Busy& rhs) const;

  // The busy module interface:
public:
  virtual void GoBusy();
  virtual void GoClear();
  void GoReady();		/* Left in for compatiblity with external kludge code. */

};
#endif
