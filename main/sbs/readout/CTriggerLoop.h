#ifndef CTRIGGERLOOP_H
#define CTRIGGERLOOP_H

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

// Headers:

#include <Thread.h>
#include <string>

#include <string>


// Forward class definitions

class CExperiment;
class CEventTrigger;

/*!

  The trigger loop is responsible for checking for things to do when an
  experiment is taking data.  Specificially it will ge the
  experiment's event and scaler triggers and iterate checking them
  and calling the appropriate methods in the experiment to react to trigger
  conditions.

*/
class CTriggerLoop : public Thread
{
private:
  CExperiment*       m_pExperiment;


  volatile bool      m_running;
  volatile bool      m_stopping;  // Shared between threads.
  volatile bool      m_pausing;   // Shared between threads.
  volatile bool      m_failed;    // Shared between thread... trigger loop failed.
  bool               m_vmeLock;   // True if we need to serialize VME access.
public:
  CTriggerLoop(CExperiment& experiment);
  virtual ~CTriggerLoop();

  // most of the canonicals are illegal so we don't have to worry about cloning threads:

private:
  CTriggerLoop(const CTriggerLoop& rhs);
  CTriggerLoop& operator=(const CTriggerLoop& rhs);
  int operator==(const CTriggerLoop& rhs) const;
  int operator!=(const CTriggerLoop& rhs) const;

  // Member functions externally available.
public:
  virtual void start();
  void         stop(bool pausing);          // stop/join.
  virtual void run();
  void         doVMELock() {m_vmeLock = true;}
  void         noVMELock() {m_vmeLock = false;}

protected:
  void         mainLoop();
private:
  void reportFail(std::string msg);
};


#endif
