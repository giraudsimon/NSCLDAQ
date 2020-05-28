#ifndef CCAENV262TRIGGER_H
#define CCAENV262TRIGGER_H

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

#include "CEventTrigger.h"
#include <stdint.h>

class CCaenIO;

/*!
  The CAEN V262 I/O module can be used to provide external
  triggers to the data taking sytsem.  This is normally used
  to create event triggers however if you want to use it to issue
  scaler triggers, knock yourself out, it's not illegal and should
  even work.

  The module should be used as follows:
  - NIM IN         0, is used to indicate a trigger,
  - NIM Pulsed out 3, is used to acknowledge the receipt of the trigger.

  since the NIM inputs are not latched, trigger in (NIM 0) must be 
  externally latched until the NIM Pulsed out 3  is seen, or else the trigger
  will be missed.  This is normally done with a gate and delay generator in
  latch mode.  The Master.Live trigger starts the gate generator, whose
  output goes to NIM In 0, the NIM pulsed out 3 is then cabled to the
  gate generator's stop input.


*/

class CCAENV262Trigger : public CEventTrigger
{
private: 
  CCaenIO&    m_TriggerModule;

  // Canonicals:
public:
  CCAENV262Trigger(uint32_t base, unsigned crate = 0);
  CCAENV262Trigger(CCaenIO& module);

  CCAENV262Trigger(const CCAENV262Trigger& rhs);


  // comparisons don't make sense really.
private:
  CCAENV262Trigger& operator=(const CCAENV262Trigger& rhs);
  int operator==(const CCAENV262Trigger& rhs) const;
  int operator!=(const CCAENV262Trigger& rhs) const;

  // operations on the trigger:

public:
  virtual bool operator()();
};

#endif
