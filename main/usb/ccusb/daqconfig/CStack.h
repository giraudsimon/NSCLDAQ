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
#ifndef __CSTACK_H
#define __CSTACK_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
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

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif


// forward definitions

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/*!
  The CStack class represents a CC-USB triggered readout list.   
  The CC-USB supports two stacks:
  - event which can be triggerd by NIM 1 or(and?) any of a set of lams.
  - scaler which can be triggered by a time interval or every few events
    (I won't support the every few events option yet).

  The CStack has the following configuration options:

\verbatim
Option            Type/Default     Meaning
-type             enum event    Type of stack, "event" or "scaler" are the
                                supported keyword values for this enum.
-modules          list          List of names of modules that should be read
                                for this stack.

options only valid only for event stacks:

-lams             int  0        Masks of lams that can trigger the event,
                                NIM1 can always trigger.
-lamtimeout       int  0        microseconds to wait for lams in lamwait operations.
-delay            int  0        Delay in usec between trigger and stack strt.
options only valid for scaler stacks:

-period           int  2        Seconds between scaler readouts.
-incremental      bool true     determines if scaler ring items are marked incremental.


\endverbatim
  
*/
class CStack : public CReadoutHardware
{
private:
  // Data types:

  typedef std::list<CReadoutHardware*>  StackElements;
public:
  typedef enum _TriggerType {
    Event,
    Scaler,
  } TriggerType;

private:
  CReadoutModule*    m_pConfiguration;
  static bool        m_incrementalScalers;


public:
  // Canonicals:

  CStack();
  CStack(const CStack& rhs);
  virtual ~CStack();
  CStack& operator=(const CStack& rhs);

  // forbidden canonicals
private:
  int operator==(const CStack& rhs) const;
  int operator!=(const CStack& rhs) const;
public:


  // The CReadoutHardware Interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual void onEndRun(CCCUSB& controller);
  virtual CReadoutHardware* clone() const;

  // Public interfaces specific to CStack

  void loadStack(CCCUSB& controller);
  void enableStack(CCCUSB& controller);
  TriggerType     getTriggerType();
  static bool     isIncremental() {return m_incrementalScalers; }


  // Utility member functions:

private:

  unsigned int    getIntegerParameter(std::string name);
  StackElements   getStackElements();
  int getListNumber();

  // Custom validators

  static bool moduleChecker(std::string name, std::string proposedValue, void* arg);

};


#endif
