#ifndef CSYNCHRONIZEDTHREAD_H
#define CSYNCHRONIZEDTHREAD_H
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <Thread.h>
#include <CMutex.h>
#include <CCondition.h>

/**
 * This is an encapsulation of a thread that has some initialization that may be
 * inherently thread-unsafe.  The idea is that the start method is that we override
 * start() so that it takes out a mutex, starts the thread and then waits on a condition
 * variable tied to the thread.  The run() method is also overriddent to
 * first invoke the init() method and then signal the condition variable prior to 
 * invoking an operator() method that is the actual main body of the thread.
 *
 */
class CSynchronizedThread : public Thread 
{
  // The synchronization data structures:
private:
  CMutex             m_mutex;
  CConditionVariable m_conditionVar;
public:
  CSynchronizedThread();
  virtual ~CSynchronizedThread();

  // External interface:

public:
  void start();
  virtual void run();
  virtual void init();
  virtual void operator()() = 0;


};


#endif
