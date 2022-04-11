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
    ~CMyEventSegment();

    virtual void initialize();
    //virtual DAQWordBufferPtr& Read(DAQWordBufferPtr& rBuffer);
    virtual size_t read(void* rBuffer, size_t maxwords);
    virtual void disable();
    virtual void clear();

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
