#ifndef CV977TRIGGER_H
#define CV977TRIGGER_H
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

#include <CEventTrigger.h>
#include <stdint.h>

class CCAENV977;

/*!
   The CAEN V977 is an coincidence/pattern register.  This class implements a trigger
   module using that device.  The first input of the device is used as a trigger.
   That bit is echoed at hardware speeds on the corresponding output, and is cleared
   once the trigger is acknowledged.  This is slightly different than the 
   CAEN V977 trigger used in the 8.1 version of NSCLDAQ..where that bit is the busy bit
   for a combined trigger/status module.
   The clear in this case ensures that the module is able to accept a new trigger.

   See the busy status module class associated with the V977 for information about how
   to use it as a dead-time register.

*/
class CV977Trigger : public CEventTrigger
{
private:
  CCAENV977*    m_pModule;	// Pointer to the device.
  bool          m_ownModule;    // True if destruction deletes.

  // Canonicals:
public:
  CV977Trigger(uint32_t base, unsigned crate =0);
  CV977Trigger(CCAENV977* pModule, bool deleteme=true);
  ~CV977Trigger();
private:
  CV977Trigger(const CV977Trigger& );
  CV977Trigger& operator=(const CV977Trigger&);
  int operator==(const CV977Trigger&) const;
  int operator!=(const CV977Trigger&) const;

  // Overrides for the trigger functions:

public:
  virtual void setup();
  virtual bool operator()();

};

#endif
