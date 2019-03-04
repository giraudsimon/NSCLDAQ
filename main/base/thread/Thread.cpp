/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/



#include <Thread.h>

#include <netdb.h>
#include <sys/socket.h>
#include <string>
#include <stdexcept>
#include <Exception.h>
#include <ErrnoException.h>
#include <limits.h>
#include <iostream>

using namespace std;


void*
Thread::threadStarter(void *aThread) {
  static int exitcode = 0;
  Thread *thrd = (Thread*)aThread;    
  if (thrd == NULL) return(NULL);

  thrd->my_id = (dshwrapthread_t)(dshwrapthread_self());
  thrd->living = true;

  try {
    thrd->run();
  } catch (std::runtime_error& re) {
    cerr << "*** Runtime error caught in thread: \"" << (re.what()) << "\"" << endl;
  } catch (CException& oe) {
    cerr << "*** Exception caught in thread: \"" << (oe.ReasonText()) << "\"" << endl;
  } catch (std::exception& se) {
    cerr << "*** Exception caught in thread: \"" << (se.what()) << "\"" << endl;
  } catch(std::string msg) {
    cerr << "*** Exception caught in thread: \"" << msg << "\"" << endl;
  } catch (const char* msg) {
    cerr << "*** Exception caught in thread: \"" << msg << "\"" << endl;
  } catch (...) {
    cerr << "*** Unknown exception caught in thread" << endl;
  }

  thrd->living = false;
  thrd->my_id = (dshwrapthread_t)(-1);

  dshwrapthread_exit((void *)&exitcode);
  return(NULL);
}

/*!
    Static method to return the id of the running thread.

    @return int
    @retval Id of currently running thread.
*/
int
Thread::runningThread() 
{
  return (int)(dshwrapthread_self());
}


/*===================================================================*/
/** @fn Thread::Thread()
* @brief Default constructor.
*                                        
* Default constructor.
*                                         
* @param None
* @return this
*/      
Thread::Thread() :
  my_id(-1),
  living(false)
{
  string nam("Thread(");
  nam     += "anonymous";
  nam     += ')';
  setName(nam);
}

/*===================================================================*/
/** @fn Thread::Thread(String& rName)
* @brief Constructor with a thread name.
*                                        
* Constructor with a thread name. 
*                                         
* @param rName A name for this thread.
* @return this
*/      
Thread::Thread(string rName)  :
  my_id(-1),
  living(false)
{
  setName(rName);
}

/*===================================================================*/
/** @fn Thread::~Thread()
* @brief Destructor.
*                                        
* Destructor.
*                                         
* @param None
* @return None
*/      
Thread::~Thread() { 
  my_id = (dshwrapthread_t)-1;
  living = false;
}

/*===================================================================*/
/** @fn unsigned long Thread::getId()
* @brief Get this thread's id.
*                                        
* Get this thread's id.
*                                         
* @param None
* @return This thread's id or -1 if the thread is uninitialized.
*/      
unsigned long Thread::getId() {
  return((unsigned long)my_id);
}

/*===================================================================*/
/** @fn int Thread::detach()
* @brief Detach (daemonize) this thread.
*                                        
* Detach (daemonize) this thread.  Daemon threads do not have to
* be joined with.
*                                         
* @param None
* @return The status of the detachment.
*/      
int Thread::detach() {
  if (!living) return -1;
  return dshwrapthread_detach(my_id);
}

/*===================================================================*/
/** @fn void Thread::join()
* @brief Join with this thread.
*                                        
* Join with this thread by waiting for it to exit.  Daemon threads
* cannot be joined with.  Errno is set on return.
*                                         
* @param None
* @return None
*/      
void Thread::join() {
   int rc;
   if (!living) return;
   rc = dshwrapthread_join(my_id,NULL);
   errno = rc;
}

/*===================================================================*/
/** @fn void Thread::setName(const String& rName)
* @brief Set this thread's name.
*                                        
* Set this thread's name.
*                                         
* @param rName A new name for this thread.
* @return None
*/      
void Thread::setName(const string rName) {
  threadname = rName;
}

/*===================================================================*/
/** @fn const String& Thread::getName() const
* @brief Get this thread's name.
*                                        
* Get this thread's name.
*                                         
* @param None
* @return This thread's name.
*/      
string Thread::getName() const {
  return(threadname);
}

/*===================================================================*/
/** @fn void Thread::start()
* @brief Start this thread.
*                                        
* Start this thread.
*                                         
* @param None
* @return None
*/      
void Thread::start() {
  int rc = 0;
  dshwrapthread_t newtid;
  if ((rc = dshwrapthread_create(&newtid,NULL,Thread::threadStarter,(void *)this)) < 0) {
    errno = rc;
    throw CErrnoException("Starting thread, unable to do so.");

  }
}
