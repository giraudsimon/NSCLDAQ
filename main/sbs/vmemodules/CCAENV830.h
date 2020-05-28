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


#ifndef CCAENV830_H
#define CCAENV830_H


#include <string>
#include <VmeModule.h>
#include <stdint.h>

#ifndef CCAENV820
#define CCAENV820 CCAENV830	// V820/V830 -- same class.
#endif

// Forward definitions:



/*!
   Encapsulates the readout driver for a CCAENV820 scaler.
   In this case the scaler is intended to be readout as part of an


*/

class CCAENV830
{
  // Class types:
public:
  typedef enum _TriggerMode {
    Disabled,
    Random,
    Periodic
  } TriggerMode;

  // Class data.
private:
  int                m_nCrate;
  int                m_nSlot;
  int                m_nEventLength; // Computed from settings.
  int                m_nBase;	     // Module base address.  
  CVmeModule*        m_pModule;
  int                m_nModuleType;
  int                m_nSerialno;
  int                m_nFirmware;
  bool               m_fGeo;
  
  // Constructors and other canonical operations:

public:
  CCAENV830(int slot,
	    int crate = 0,
	    bool geo = true,
	    uint32_t base = 0) ;

  ~CCAENV830();

  // Outlawed functions:
private:
  CCAENV830 (const CCAENV830& rhs );
  CCAENV830& operator= (const CCAENV830& rhs);
  int operator== (const CCAENV830& rhs) const;
  int operator!= (const CCAENV830& rhs) const;
public:


  // Selectors:

public:

  int getCrate() const {
    return m_nCrate;
  }
  int getSlot() const {
    return m_nSlot;
  }
  int getModuleType() const {
    return m_nModuleType;
  }
  int getFirmware() const {
    return m_nFirmware;
  }

  int getSerial() const {
    return m_nSerialno;
  }

  // mutators:

  void setCrate(int crate) {
    m_nCrate = crate;
  }
  void setSlot(int slot) {
    m_nSlot = slot;
  }
  void setModuleType(int type) {
    m_nModuleType = type;
  }
  void setFirmware(int fwRev) {
    m_nFirmware = fwRev;
  }


  // Class operations.

public:
  void Enable(int nChannel) ;
  void Disable(int nChannel) ;
  void SetEnableMask(uint32_t nMask);
  void SetDwellTime(int n400ns);
  void SetTriggerMode(TriggerMode mode) ;
  void SetWide();
  void SetNarrow();
  void EnableHeader();
  void DisableHeader();
  void FPClearsMEB(bool state);
  void EnableAutoReset();
  void DisableAutoReset();
  bool isDataReady();
  bool isAlmostFull();
  bool isFull();
  bool isGlobalDready();
  bool isGlobalBusy();
  void Reset();
  void Clear();
  void Trigger();
  void SetTriggerCounter(int n);
  void SetAlmostFullLevel(int n);
  int GetMEBEventCount();

  int  ReadEvent(void* pBuffer);

  int  ReadCounter(int nChannel);

  // Utility functions:
protected:
  void ComputeEventSize();
  void MapModule() ;
  void InitModule();
  void UnmapModule();
};
   


#endif
