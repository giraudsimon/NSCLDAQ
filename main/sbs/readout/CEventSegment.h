#ifndef CEVENTSEGMENT_H
#define CEVENTSEGMENT_H
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

#include "CObject.h"
#include <stddef.h>
#include <stdint.h>

/*!

   This is an abstract base class for event segments.  An event segment reads out a
   logical chunk of an experiment. What defines a logical chunk of an experiment is
   up to the application designer.  The CCompoundEventSegment container allows you to
   compose event segments hierarchically into larger chunks.  This makes it easy
   to combine code from different detector subsystems while maintaining them 
   separately.
*/
class CEventSegment : public CObject
{
  // public types:

public:
  typedef enum _AcceptState {Keep, Reject, RejectImmediately} AcceptState;
private:
  AcceptState m_accept;
public:
  virtual void   initialize();
  virtual void   clear();
  virtual void   disable();
  virtual size_t read(void* pBuffer, size_t maxwords) = 0;
  virtual void   onBegin();
  virtual void   onEnd();
  virtual void   onPause();
  virtual void   onResume();

  virtual bool isComposite() const;
  
  
  // Base class methods
public:
  void reject();
  void rejectImmediately();
  void keep();
  AcceptState getAcceptState() const;
  virtual void setTimestamp(uint64_t stamp);
  void setSourceId(uint32_t id);
};


#endif

