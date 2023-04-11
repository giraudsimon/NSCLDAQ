#ifndef CMYEVENTSEGMENT_H
#define CMYEVENTSEGMENT_H

#include <Configuration.h>
#include <SystemBooter.h>

#include <CExperiment.h>
#include <CEventSegment.h>
#include <CRingItem.h>
#include <CRingBuffer.h>
#include <vector>
#include <deque>
#include <fstream>
#include <string>
#include <iostream>

#include "ZeroCopyHit.h"
#include "ModuleReader.h"

// event segment for the control
// and readout of pixie16 modules
// 1 crate only readout, standard firmware
// snl - 1/20/10
using namespace std;

#define MAXMOD 14
#define  MAX_NUM_PIXIE16_MODULES 24

class CMyTrigger;
class CExperiment;


class CMyEventSegment : public CEventSegment
{
private:
#pragma pack(push, 1)
  struct HitHeader {
    uint32_t    s_id;
    uint32_t    s_tstampLow;
    uint32_t    s_tstampHighCFD;
    uint32_t    s_traceInfo;
        
    // Selectors -- a bit too magic numbery but sufficient for
    // what we want to do in debugging.
        
    unsigned getChan() const {
      return s_id & 0xf;
    }
    unsigned getSlot() const {
      return (s_id & 0xf0) >> 4;
    }
    unsigned getCrate() const {
      return (s_id & 0xf00) >> 8;
    }
    unsigned headerLength() const {
      return (s_id & 0x1f000) >> 12;
    }
    unsigned eventLength() const {
      return (s_id & 0x7ffe0000) >> 17;
    }
  };
#pragma pack(pop)
private:
  std::ofstream ofile;


  unsigned short NumModules;
  unsigned int *ModEventLen;

  unsigned int ModuleRevBitMSPSWord[MAX_NUM_PIXIE16_MODULES]; //word to store rev, bit depth, and MSPS of module for insertion into the data stream.
  double ModClockCal[MAX_NUM_PIXIE16_MODULES]; //word to calibration between clock ticks and nanoseconds.



  CMyTrigger *mytrigger;
    
  DAQ::DDAS::Configuration m_config;
  bool m_systemInitialized;
  bool m_firmwareLoadedRecently;
  CExperiment*  m_pExperiment;
   
  // Statistics:
    
  size_t m_nCumulativeBytes;
  size_t m_nBytesPerRun;
    
public:
  CMyEventSegment(CMyTrigger *trig, CExperiment& exp);
  CMyEventSegment();                  // For unit testing only!!
  ~CMyEventSegment();

  virtual void initialize();
  //virtual DAQWordBufferPtr& Read(DAQWordBufferPtr& rBuffer);
  virtual size_t read(void* rBuffer, size_t maxwords);
  virtual void disable();
  virtual void clear();

  // manage explicit run start and resume operations
  virtual void onBegin();
  virtual void onResume();  

  virtual void onEnd(CExperiment* pExperiment);
  int GetNumberOfModules() {return (int)NumModules;}
  int GetCrateID() const;

  void synchronize();            //!< Clock synchronization.
  void boot(DAQ::DDAS::SystemBooter::BootType = DAQ::DDAS::SystemBooter::FullBoot);                   //!< load fimrware and start boards.

  std::pair<size_t, size_t>getStatistics() {
    return std::pair<size_t, size_t>(m_nCumulativeBytes, m_nBytesPerRun);
  }
    
};
#endif
