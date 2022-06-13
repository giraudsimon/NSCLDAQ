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


#ifndef CCAENV560_H
#define CCAENV560_H

#include <string>
#include <VmeModule.h>
#include <stdint.h>

// Forward definitions:



/*!
   Encapsulates the readout driver for a CCAENV560 scaler.
   This class was contributed by Kevin Carnes, Kansas State University.
   Only minor modifications have been made from the original.
   I believe that due to the use of the CVmeModule abstraction of the VME,
   this should be vme bus controller inependent.

   Summary of changes at the NSCL:
    - Reversed the order of the parameters to the constructor to allow
      construction without specifying a vme crate if crate 0 is desired,
      and to require a base address.
    - Removed the functions that control/examine the module interrupts as
      these are irrelevant in the way the module is used in the NSCLDAQ.
      Functions removed are:
\verbatim
  void EnableInt(int nSection) throw (STD(string));
  void DisableInt(int nSection) throw (STD(string));
  bool GetInt(int nSection) throw (STD(string)); 
  void ClearInt();
  void EnableVMEInt();
  void DisableVMEInt();
  void SetIntLevel(short nLevel);
  short GetIntLevel();
  void SetIntVec(short nVector);
  short GetIntVec();
\endverbatim

      - Attempt to catch incorrect module errors by checking for
        fixed code and manufacturer model no. register correctness.

*/

class CCAENV560
{

  // Class data.
private:
  int                m_nCrate;
  uint32_t           m_nBase;	     // Module base address.  
  CVmeModule*        m_pModule;
  int                m_nModuleType;
  int                m_nSerialno;
  int                m_nVersion;
  
  // Constructors and other canonical operations:

public:
  CCAENV560(uint32_t base, int crate = 0) ;

  ~CCAENV560();

  // Outlawed functions:
private:
  CCAENV560 (const CCAENV560& rhs );
  CCAENV560& operator= (const CCAENV560& rhs);
  int operator== (const CCAENV560& rhs) const;
  int operator!= (const CCAENV560& rhs) const;
public:


  // Selectors:

public:

  int getCrate() const {
    return m_nCrate;
  }
  int getModuleType() const {
    return m_nModuleType;
  }
  int getVersion() const {
    return m_nVersion;
  }

  int getSerial() const {
    return m_nSerialno;
  }

  // mutators:

  void setCrate(int crate) {
    m_nCrate = crate;
  }
  // Class operations.

public:
  bool GetSectStat(int nSection) ;
  void ScalInc();
  void SetVeto();
  void ResetVeto();
  void Clear();
  bool GetVetoStat();

  int ReadCounter(int nChannel) ;
  // Utility functions:
protected:
  void ComputeEventSize();
  void MapModule() ;
  void InitModule();
  void UnmapModule();
};
   


#endif
