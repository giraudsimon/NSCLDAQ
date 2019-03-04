#ifndef DAQHWYAPI_THREAD_H
#define DAQHWYAPI_THREAD_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/



#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <iostream>


#ifndef DAQHWYAPI_RUNNABLE_H
#include <Runnable.h>
#endif

#ifndef DAQHWYAPI_SYNCHRONIZABLE_H
#include <Synchronizable.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



/*=====================================================================*/
/**
* @class Thread
* @brief Abstract class for implementing threaded processes.
*
* Abstract class for implementing threaded processes.  Application
* specific threads should inherit from this class and implement the
* virtual run() method.  Threads can be started calling the start()
* method.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Thread : public daqhwyapi::Runnable {

  // Object data:

  private:
    std::string     threadname;
    dshwrapthread_t my_id;
    bool            living;

  // Public interface

  public: 
    Thread();                     // Constructor
    Thread(std::string);   // Constructor with a name
    virtual ~Thread();            // Destructor
    int detach();                 // Detach thread
    unsigned long getId();        // Return thread id
    void start();                 // Start this thread
    void setName(std::string);  // Set this thread's name
    void join();                  // Join with this thread
    std::string getName() const; // Get this thread's name
    virtual void run() = 0;       // Run this thread

    static int runningThread();	  // Get id of running thread.

  // Utilities.

private:
    static void     *threadStarter(void *);
};


#endif
