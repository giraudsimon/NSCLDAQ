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

#ifndef __CRUNSTATE_H
#define __CRUNSTATE_H

#ifndef __CGAURDEDOBJECT_H
#include <CGaurdedObject.h>
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

class CMonitorVariables;

/*!
   This class contains run state information.  Methods allow the
   run state data to be modified and accessed in a thread safe way.
   Typically three threads will need to access these data:
   - Main thread in resonse to commands that change the state.
   - Readout thread to make state changes.
   - Router thread to fill in control buffers and NSCL Buffer header information.

   \note This is an object in a singletonpattern.
*/
class CRunState : public CGaurdedObject
{
  // Data structures and consts usable by clients:
public:
  typedef enum _RunState {	// Simplified model.
    Idle,
    Active,
    Paused,
    Starting,			/* Run is starting but not yet active */
    Stopping			/* Run is stopping but not yet inactive. */
  } RunState;

  // Class level data:
private:
  static CRunState*   m_pTheInstance;

  // Instance data:

private:
  std::string    m_title;
  uint16_t       m_runNumber;
  RunState       m_state;
  uint32_t       m_scalerPeriod;
  CMonitorVariables* m_pVarMonitor;

private:
  CRunState();
public:
  ~CRunState();			// This is final so not virtual

  // These make no sense for singleton objects;

  CRunState(const CRunState& rhs);
  CRunState& operator=(const CRunState& rhs);
  int operator==(const CRunState& rhs) const;
  int operator!=(const CRunState& rhs) const;

  // Singleton access:
public:
  static CRunState* getInstance();

  // Methods on the object that are gaurded by the semaphore:

public:
  RunState getState();
  void     setState(RunState newState);

  std::string getTitle();
  void        setTitle(std::string newTitle);

  uint16_t    getRunNumber();
  void        setRunNumber(uint16_t newRunNumber);
  

  uint32_t    getScalerPeriod();
  void        setScalerPeriod(uint32_t period);
  CMonitorVariables* getVarMonitor() {
    return m_pVarMonitor;
  }
  void setVarMonitor(CMonitorVariables* pMon) {
    m_pVarMonitor = pMon;
  }
};

#endif
