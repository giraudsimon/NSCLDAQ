#ifndef CVARIABLEBUFFERS_H
#define CVARIABLEBUFFERS_H
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

#include <tcl.h>
#include <stdint.h>
#include "CDocumentedVars.h"

class CConditionVariable;
class CMutex;
class CRingBuffer;
class CTCLInterpreter;



/*!
  This class encapsulates the problem of creating run and state variable events.
  There are two issues.  The first is the simple one of creating those events.
  That's done by encapsulating the CDocumentedVars object so that there's access
  to the methods that return variable name/value pairs.

  The more interesting problem is that the thread that wants to produce a
  variable event may be different from the thread that runs the interpreter that
  maintains them.  If so some fancy footwork involving the event loop is required
  so that the Tcl apartment threading model is not violated:
  - Members are provided to 'trigger' these events.
  - If the trigger is done in a thread other than the one that created this object,
    an event is queued to the creating thread and we block on a condition variable.
    The guy that then makes the event will fire the condition variable when done
    so that the whole process looks synchronous to the trigger thread.
  - If the trigger is done in the same thread, we're just going to call the guy
    that makes the event.. which is inherently synchronous.
  - Finally, the condition thread's mutex is used to monitor access to the 
    trigger event...though the application is structured to prevent that anyway.

  This class makes use of a modified singleton pattern as well.  The constructor is 
  still public, however there is a static instance variable, and the
  constructor throws an exception if that's non-null.  There's a static function
  that will return the instance variable so the object can be located externally
  (e.g. by the objects involved in the trigger thread).

*/

class CVariableBuffers
{
  // Data structures:
private:
  typedef struct _TriggerEvent {
    Tcl_Event    s_RawEvent;
    CRingBuffer* s_pRing;
    uint64_t     s_TimeOffset;
    
  } TriggerEvent, *pTriggerEvent;

  typedef void Creator(CRingBuffer* pRing, uint64_t toffset);

  // Member data.
private:
  CDocumentedVars*     m_pVars;
  CMutex*              m_pTriggerGuard;
  CConditionVariable*  m_pTriggerSynchronize;

  Tcl_ThreadId           m_interpreterThread;

  static CVariableBuffers* m_pInstance;

  static uint32_t            m_nSourceId;

  // constructors and other canonicals as well as the instance fetcher:

public:
  CVariableBuffers(CTCLInterpreter& interp);
  ~CVariableBuffers();

  static  CVariableBuffers* getInstance();

  // Copy/assign compare don't make sense in a singleton.

private:
  CVariableBuffers(const CVariableBuffers& );
  CVariableBuffers& operator=(const CVariableBuffers&);
  int operator==(const CVariableBuffers&) const;
  int operator!=(const CVariableBuffers&) const;

  // Trigger members:
public:  
  void triggerRunVariableBuffer(CRingBuffer* pRing,
				uint64_t timeoffset);
  void triggerStateVariableBuffer(CRingBuffer* pRing,
				  uint64_t timeoffset);

  // manipulate and retrieve the source id value
  void setSourceId(uint32_t sourceId);
  uint32_t getSourceId() const;


 private:
  void triggerBuffer(CRingBuffer* pRing,
		     Tcl_EventProc* handler,
		     Creator* creator,
		     uint64_t timeoffset);
  
  // Event relays.. the targets of the event:
 private:
  static  void createRunVariableEvent(CRingBuffer* pRing, uint64_t timeBase);
  static  void createStateVariableEvent(CRingBuffer* pRing, uint64_t timeBase);
  
  static  void createDocEvent(CRingBuffer* pRing,
			      uint16_t     eventType,
			      CDocumentedVars::NameValuePairs& variables,
			      uint64_t    tbase);
  
  static int HandleRunVarTrigger(Tcl_Event* evPtr,
				 int        flags);
  static int HandleStateVarTrigger(Tcl_Event* evPtr,
				   int       flags);
};


#endif
