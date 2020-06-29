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
#ifndef CTRADITIONALV977TRIGGER_H
#define CTRADITIONALV977TRIGGER_H

// Base class header.

#include <Trigger.h>
#include <daqdatatypes.h>

// Forward definitions.

class CCAENV977;


/*!
    This class provides a trigger for the traditional
    readout (readout classic).  See the application note
    on traditional triggers for information about how to
    use this.
*/

class CTraditionalV977Trigger : public Trigger
{
private:
  CCAENV977& m_Module;
  UShort_t   m_LastPattern;
public:
  CTraditionalV977Trigger(ULong_t base, UShort_t crate = 0);
  virtual ~CTraditionalV977Trigger();

  virtual void Initialize();
  virtual void Enable();
  virtual void Disable();
  virtual bool Check();
  virtual void Clear();

  UShort_t getTriggerPattern() const;


};

#endif
