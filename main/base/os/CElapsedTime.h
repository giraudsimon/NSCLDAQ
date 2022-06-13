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

/** @file:  CElapsedTime.h
 *  @brief: Provide a class for measuring elapsed times.
 */

#ifndef CELAPSEDTIME_H
#define CELAPSEDTIME_H

/**
 * @class CElapsedTime
 *     On construction, this class remembers the current time.  Let's call this
 *     the 'start time'  The 'start time' can be re-set using the start method.
 *     The meausure method will return a double  precision number of seconds
 *     since the start time.
 *
 *     @note - internally, gettimerofday(2) is used which is POSIX.1-2001
 *             compliant.
 */
class CElapsedTime
{
private:
    double    m_startTime;
public:
    CElapsedTime();
    
    // The other canonicals can default to the compiler defined ones.
    
    void start();
    double measure() const;
private:
    static double getTime();
};

#endif