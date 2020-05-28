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
// event segment for the control
// and readout of pixie16 modules
// 1 crate only readout, standard firmware
// snl - 1/20/10
using namespace std;

#define MAXMOD 14
#define  MAX_NUM_PIXIE16_MODULES 24

class CMyTrigger;

namespace DAQ {
namespace DDAS {
class Configuration;
}
}

class channel
{

    public:
        double time;
        int chanid;
        int channellength;
        vector<uint32_t> data;

        channel();
        ~channel();
        double GetTime() const {return time;};
        int SetTime();
        double GetChannelLength() const {return channellength;};
        int SetChannelLength();
        int SetChannel();
        int GetChannel() const {return chanid;};
        int Validate(int);
};

class CMyEventSegment : public CEventSegment
{

private:
    //any private variables
    //CRingBuffer *ring;
    std::ofstream ofile;

    int fragcount;
    double globaltime;
    double time_buffer;

    unsigned short NumModules;
    double *ModEventLen;
    double StopTime;

    unsigned int ModuleRevBitMSPSWord[MAX_NUM_PIXIE16_MODULES]; //word to store rev, bit depth, and MSPS of module for insertion into the data stream.
    unsigned int ModClockCal[MAX_NUM_PIXIE16_MODULES]; //word to calibration between clock ticks and nanoseconds.

    int retval;
    unsigned int nFIFOWords;

    unsigned long totwords;

    deque<channel *> ModuleDeque[MAXMOD];
    vector<uint32_t> ModuleData[MAXMOD];

    vector<uint32_t> nFIFOWordsinModuleCurrentRead;
    vector<uint64_t> nFIFOWordsinModuleTotal;

    vector<double> CurrentTimeinModuleRead;
    vector<double> FirstTimeinModuleRead;

    bool m_processing;
    bool processdata;

    CMyTrigger *mytrigger;

    double m_tstampScaleFactor;

    DAQ::DDAS::Configuration   m_config;
    bool m_systemInitialized;
    bool m_firmwareLoadedRecently;

public:
    CMyEventSegment(CMyTrigger *trig);
    ~CMyEventSegment();

    virtual void initialize();
    //virtual DAQWordBufferPtr& Read(DAQWordBufferPtr& rBuffer);
    virtual size_t read(void* rBuffer, size_t maxwords);
    virtual void disable();
    virtual void clear();

    virtual void onEnd(CExperiment* pExperiment);
    int GetNumberOfModules() {return (int)NumModules;}
    int GetCrateID() const;

    bool IsUniqueEvent(const channel* event);

    void boot(DAQ::DDAS::SystemBooter::BootType type = DAQ::DDAS::SystemBooter::FullBoot);
    void synchronize();

    void setTimestampScaleFactor(double value);
    double getTimestampScaleFactor() const;

private:
    size_t SelectivelyOutputData(void* rBuffer, size_t maxwords);
    bool   DataToRead();
    void PushDataOntoQueue(std::deque<channel*>& deque, std::deque<channel*>& buffer);
    uint64_t extractExternalTimestamp(const std::vector<uint32_t>& data);
    bool isHeterogeneousSystem(const std::vector<int>& hdwrMap);

};
#endif
