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

#ifndef CTRADTIONALV977BUSY_H
#define CTRADITIONALV977BUSY_H

// include base class header:

#include <Busy.h>
#include <daqdatatypes.h>

// forward classes


class CCAENV977;

/*!
  This class is a busy module using the CCAENV977 i/o register.
  See application notes describing how to use this.
*/
class CTraditionalV977Busy : public CBusy
{
private:
  CCAENV977& m_Module;
public:
  CTraditionalV977Busy(ULong_t base, UShort_t crate = 0);
  virtual ~CTraditionalV977Busy();


  virtual void Set();		//!< Go busy.
  virtual void Clear();		//!< go not busy.
  virtual void ModuleClear();	//!< Pulse the module clears.
  virtual void ScalerSet() {}   //!< No scaler busy signal used.
  virtual void ScalerClear() {} //!< No scaler busy signal used.

  // Utility functions

private:
  void PulseOutputs(UShort_t mask); 
};
#endif
