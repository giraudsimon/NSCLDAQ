#ifndef CTIMEDTRIGGER_H
#define CTIMEDTRIGGER_H

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
#include <time.h>

/*!
  This class defines a trigger that fires at fixed, periodic time intervals.
  The trigger is determined using the POSIX clock_gettime function operating on the
  real time (wall) clock.  In most systems this will allow for trigger periodicity less
  than one second...however it is recommended this not be used for that purpose.

*/
class CTimedTrigger : public CEventTrigger
{
private:
  struct timespec m_lastTrigger;
  struct timespec m_nextTrigger;
  struct timespec m_periodicity;

  // Canonicals:

public:
  CTimedTrigger(const timespec& interval);
  CTimedTrigger(const CTimedTrigger& interval);
  CTimedTrigger& operator=(const CTimedTrigger& interval);
  int operator==(const CTimedTrigger& rhs) const;
  int operator!=(const CTimedTrigger& rhs) const;

  // Additional object member functions:
public:
  void setPeriod(const struct timespec& newInterval);

  // Virtual function overrides
public:
  virtual void setup();
  virtual bool operator()();
};


// The following operators on times are useful:

struct timespec operator+(const struct timespec& t1,  const struct timespec& t2);
bool            operator>=(const struct timespec& t1, const struct timespec& t2);
bool            operator==(const struct timespec& t1, const struct timespec& t2);
#endif
