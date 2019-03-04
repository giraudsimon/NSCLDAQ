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
#include "CTimedTrigger.h"


static const long nsPerSec(1000*1000*1000); // Nanoseconds per second.
static const  struct timespec zero={0,0};

//////////////////////////////////////////////////////////////////////////////////

/*!
   This unbound function allows users to naturally add two timespecs together,
   getting a new one back.
   \param t1 first summand.
   \param t2 second summand
   \result struct timespec
   \retval The normalized sum of t1 and t2.  Normalized means that
           the tv_nsec field of the result is strictly less than
	   1 second worth of nanoseconds.
   
*/
struct timespec
operator+(const struct timespec& t1, const struct timespec& t2)
{
  struct timespec result = t1;
  result.tv_nsec           += t2.tv_nsec;
  result.tv_sec          += t2.tv_sec;

  // Don't assume t1/t2 are normalized, normalize the result though:

  while(result.tv_nsec >= nsPerSec) { 
    result.tv_sec++;
    result.tv_nsec -= nsPerSec;
  }
  return result;
}
/*!
  This unbound function allows users to compare one time with another in 
  a notationally natural way.  The inbound parameters need not be normalized.

  \param t1 - The left side of the >= operator.
  \param t2 - The right side of the >= operator.

  \result bool
  \retval true t1 is the same or later time than t2.
  \retval false, t1 is earlier than t2.
*/
bool
operator>=(const timespec& t1,const timespec& t2)
{
  // the following trick normalizes t1/t2 prior to
  // the compare:

  struct timespec left  = t1 + zero;
  struct timespec right = t2 + zero;

  // Now we can trivially compare:

  if (left == right) return true;
  if (left.tv_sec > right.tv_sec) return true;
  if (left.tv_sec == right.tv_sec) {
    return left.tv_nsec > right.tv_nsec;
  }

  return false;
}
/*!
  This unbound function compares for equality.
  \param t1 - The left side of the == operator.
  \param t2 - The right side of the == operator.

  \result bool
  \retval true t1 is the same as t2.
  \retval false, t1 is not the same time as  t2.
*/
bool
operator==(const timespec& t1, const timespec& t2)
{
  struct timespec left  = t1+zero;
  struct timespec right = t2+zero;

  return ((left.tv_sec == right.tv_sec) &&
	  (left.tv_nsec  == right.tv_nsec));

}

/*!
  Construct a triger
  \param period - the trigger periodicity. This can be modified with setPeriod.
*/
CTimedTrigger::CTimedTrigger(const struct timespec& period) :
  m_periodicity(period)
{
}


/*!
  Copy construction:
  \param rhs - the template from which this object is initialized
*/
CTimedTrigger::CTimedTrigger(const CTimedTrigger& rhs)
{
  m_lastTrigger = rhs.m_lastTrigger;
  m_nextTrigger = rhs.m_nextTrigger;
  m_periodicity = rhs.m_periodicity;
}
/*!
   Assignment is like copy construction.
   \param rhs - the trigger assigned to us.
   \return CTimedTrigger&
   \retval Reference to this object.
*/
CTimedTrigger&
CTimedTrigger::operator=(const CTimedTrigger& rhs)
{
  m_lastTrigger = rhs.m_lastTrigger;
  m_nextTrigger = rhs.m_nextTrigger;
  m_periodicity = rhs.m_periodicity;

  return *this;
}
/*!
   Two timed triggers are equal if their last trigger and
   periodicity are the same.

*/
int
CTimedTrigger::operator==(const struct CTimedTrigger& rhs) const
{
  return ((m_lastTrigger == rhs.m_lastTrigger)   &
	  (m_periodicity == rhs.m_periodicity));
}
/*!
   inequality is the logical complement of equality:
*/
int
CTimedTrigger::operator!=(const struct CTimedTrigger& rhs) const
{
  return !(*this == rhs);
}


/*!
  If the period is adjusted, the last trigger is also set to now.
  that is the next trigger will be at least the new period from now.
  \param newInterval - new requested periodicity
*/
void
CTimedTrigger::setPeriod(const struct timespec& newInterval)
{

  m_periodicity = newInterval;
  setup();
}

/*!
    The setup operation simply sets m_lastTrigger to now and
    the next trigger to now + interval
*/
void
CTimedTrigger::setup()
{
  clock_gettime(CLOCK_REALTIME, &m_lastTrigger);
  m_nextTrigger  = m_lastTrigger + m_periodicity;
}

/*!
   If the current time is later or the same as then next trigger time,
   a trigger occurs.
*/
bool
CTimedTrigger::operator()()
{
  bool triggered = false;

  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);

  triggered = (now >= m_nextTrigger);

  if (triggered) {
    m_lastTrigger = now;
    m_nextTrigger = now + m_periodicity;
  }
  

  return triggered;
}
