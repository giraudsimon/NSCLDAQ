/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CElapsedTime.cpp
 *  @brief:  Implementation of CElapsedTime class.
 */
#include "CElapsedTime.h"
#include <sys/time.h>
#include "ErrnoException.h"

/**
 * constructor
 *    Just really calls start:
 */
CElapsedTime::CElapsedTime()
{
    start();                         // Sets the start time.
}

/**
 * start
 *   Sets the start time with the current time in double seconds:
 */
void
CElapsedTime::start()
{
    m_startTime  = getTime();
}
/**
 * measure
 *   @return double -the elapsed time in seconds from m_startTime
 */
double
CElapsedTime::measure() const
{
    return getTime() - m_startTime;
}

//////////////////////////  Private methods ////////////////////

/**
 * getTime
 *    @return double - Time since the epoch in seconds.
 *    @note - no timezone corrections are done. Normally not a problem.
 *    
 */
double
CElapsedTime::getTime()
{
    timeval tv;
    
    if (gettimeofday(&tv, nullptr)) {
        throw CErrnoException("Failing call to gettimeofday");
    }
    
    double result = tv.tv_sec;
    double micros = tv.tv_usec;
    result += micros/(1.0e6);
    
    return result;
}
