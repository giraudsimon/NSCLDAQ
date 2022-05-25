#include <config.h>
#include <config_pixie16api.h>
#include "CMyEventSegment.h"
#include <CReadoutMain.h>
#include "Configuration.h"
#include <CExperiment.h> 
#include "SystemBooter.h"
#include "HardwareRegistry.h"
#include <string>
#include <stdio.h>
#include <iomanip>
#include <float.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstring>
#include <math.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include "CMyTrigger.h"
#include <string.h>


#include <stdexcept>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DAQ::DDAS;


CMyEventSegment::CMyEventSegment(CMyTrigger *trig, CExperiment& exp)
 : m_config(),
   m_systemInitialized(false),
   m_firmwareLoadedRecently(false),
   m_pExperiment(&exp),
   m_debugStream(std::cerr)
{

    ios_base::sync_with_stdio(true);

    // Trigger object
    mytrigger = trig;

    int retval(0);
    
    
    // Initialize the Pixie16 modules
    cout << "Trying to initialize pixie16 " << endl << flush;

    const char* fwFile =  FIRMWARE_FILE;   // Default.
    char* alternateFirmwareFile = getenv("FIRMWARE_FILE");
    if (alternateFirmwareFile) fwFile = alternateFirmwareFile;
 
    m_config = *(Configuration::generate(
        fwFile, "cfgPixie16.txt", "modevtlen.txt"
    ));

    m_config.print(std::cout);
    std::cout << std::endl;

    NumModules = m_config.getNumberOfModules();

    auto modEvtLengths = m_config.getModuleEventLengths();
    m_idToSlots = m_config.getHardwareMap();
    
    ModEventLen = new unsigned int[NumModules+1];
    std::copy(modEvtLengths.begin(), modEvtLengths.end(), ModEventLen);
    std::cout << "Module event lengths: ";
    for (auto len : modEvtLengths) {
        std::cout << len << " ";
    }
    std::cout << std::endl;

    cout.flush();
    
    


    // Conditionally load firmware and boot modules
    // The modules are only booted if the env variable DDAS_BOOT_WHEN_REQUESTED is
    // not defined.
    //
    if (getenv("DDAS_BOOT_WHEN_REQUESTED") == NULL) {
        boot();
    } else {
        boot(SystemBooter::SettingsOnly);		// Load parameters only.
    }
    
    ///////
    //  Create extra word for number of bits and ADC sampling rate. //
    namespace HR = HardwareRegistry;

    std::vector<int> hdwrMap = m_config.getHardwareMap();
    for(unsigned int k=0; k<NumModules; k++) {
        auto type = hdwrMap.at(k);
        HR::HardwareSpecification specs = HR::getSpecification(type);

        ModuleRevBitMSPSWord[k] = (specs.s_hdwrRevision<<24) + (specs.s_adcResolution<<16) + specs.s_adcFrequency;
        ModClockCal[k] = specs.s_clockCalibration;

        cout << "Module #" << k << " : module id word=0x" << hex << ModuleRevBitMSPSWord[k] << dec;
        cout << ", clock calibration=" << ModClockCal[k] << endl;
        
    }

    mytrigger->Initialize(NumModules);


#ifdef PRINTQUEINFO
    ofile.open("/scratch/data/dump.dat", std::ofstream::out);
    ofile.flags(std::ios::fixed);
    ofile.precision(0);
    
    // For the purpose of debugging... we want all of the output to go to 
    // the output file
    std::cout.rdbuf(ofile.rdbuf());
#endif 

    // specify the time_buffer... units of nanoseconds
    //buffer set to 10 seconds
    // @todo - allow this to be settable.
    
    uint64_t time_buffer = 10000000000;
    m_debug = CReadoutMain::getInstance()->getDebugLevel() == 2;
    
    
}
/**
 * Test constructor.
 */
CMyEventSegment::CMyEventSegment() :
    m_debug(true),
    m_debugStream(*(new std::stringstream))
{}

CMyEventSegment::~CMyEventSegment()
{
    // Nothing to clean up
    //cout << " frag out " << fragcount << endl;
}

void CMyEventSegment::initialize(){


    // Initialize unless there is an INFINITY_CLOCK environment variable
    // with the value "YES"
    // NOTE: This is not threadsafe as C++ does not require getenvto be
    //       thread-safe.
    //       TODO: paging through the global **environ is probably thread-safe
    //             however I'm pretty sure at this point in time there's
    //             no other thread doing a getenv().
    //
    std::string infinityClock;
    const char* envVal =  std::getenv("INFINITY_CLOCK");

    if (!envVal || m_firmwareLoadedRecently) {
        synchronize();
        m_firmwareLoadedRecently = false;
    }
    // @todo - flush the fifos?

    /***** Start list mode run *****/
    int retval = Pixie16StartListModeRun (NumModules, LIST_MODE_RUN, NEW_RUN);
    if (retval < 0) {
        cout << "*ERROR* Pixie16StartListModeRun failed " << retval 
            << endl << flush;
    } else {
        cout << "List Mode started OK " << retval << endl << flush;
    }

    usleep(100000); // Delay for the DSP boot 



}


/* Pixie16 has triggered.  There are greater than  EXTFIFO_READ_THRESH 
   words in the output FIFO of a particular Pixie16 module.  Read out 
   all pixie16 modules. */

//  virtual size_t read(unsigned short* rBuffer, size_t maxwords);
// Note - maxwords is actually maxbytes.

size_t CMyEventSegment::read(void* rBuffer, size_t maxwords)
{
  bool debug = m_debug;
  
  //memset(rBuffer, 0, maxwords);            // See what's been read.
    
    // This loop finds the first module that has at least one event in it
    // since the trigger fired.  We read the minimum of all complete events
    // and the number of complete events that fit in that buffer
    // (note that ) each buffer will also contain the module type word.
    // note as well that modules count words in uint32_t's but maxwords
    // is in uint16_t's.
    
    size_t maxLongs = maxwords/sizeof(uint32_t); // longs in buffer.
    maxLongs = maxLongs - 128;                   // To be really sure.
    //std::cerr << "max reads: " << maxwords << " -> " << maxLongs << std::endl;
    unsigned int* words = mytrigger->getWordsInModules();
    for (int i =0; i < NumModules; i++) {
        if (words[i] >= ModEventLen[i]) {
            // Figure out if we fill the buffer or just take the complete
            // events from the module:
            
            uint32_t* p = static_cast<uint32_t*>(rBuffer);
            *p++        = ModuleRevBitMSPSWord[i];
            maxLongs--;   // count that word.
            int readSize = words[i];
            if (readSize > maxLongs) readSize = maxLongs;
            // Truncated to the the nearest event size:
            
            readSize -= (readSize % ModEventLen[i]);  // only read full events.

            // Read the data right into the ring item:
            
            //std::cerr << "Going to read " << readSize
            //    << "(" << words[i] <<") " << " from " << i << std::endl;
            int stat = Pixie16ReadDataFromExternalFIFO(
                reinterpret_cast<unsigned int*>(p), (unsigned long)readSize, (unsigned short)i
            );
            if (stat != 0) {
                std::cerr << "Read failed from module " << i << std::endl;
                m_pExperiment->haveMore();
                reject();
                return 0;
            }
            m_pExperiment->haveMore();      // until we fall through the loop
            words[i] -= readSize;           // count down words still to read.
            
            return (readSize + 1) *sizeof(uint32_t)/sizeof(uint16_t);
        }
    }
    // If we got here nobody had enough data left since the last trigger:
    //std::cerr << "Not enough data to read (" << words[0] << ") since last trigger" << std::endl;
    mytrigger->Reset();
    reject();
    return 0;

    
    
}


void CMyEventSegment::clear()
{
    // Nothing to clear right now
}

void CMyEventSegment::disable()
{
    // Nothing to disable right now
}

void CMyEventSegment::onEnd(CExperiment* pExperiment) 
{
  return;                       // sorting is offloaded.
  

}

/**
 * synchronize
 *     Perform clock synchronization.  This is has been removed from
 *     initialize() so that it an be called from the constructor
 *     (infinity clock mode), conditionally from initialize() (classic mode)
 *     via a command (resynchronized infinity clock mode).
 *
 *  @throw std::runtime_error - if we fail to talk properly to the module(s).
 */
void
CMyEventSegment::synchronize()
{
    /***** Sychronize modules *****/
    int modnum = 0;
    int retval = Pixie16WriteSglModPar(const_cast<char*>("SYNCH_WAIT"), 1, modnum);
    if(retval < 0) {
        throw std::runtime_error( "Synch Wait problem ");
    } else {
        cout << "Synch Wait OK " << retval << endl;
    }

    retval = Pixie16WriteSglModPar(const_cast<char*>("IN_SYNCH"),0,modnum);
    if(retval < 0) {
        throw std::runtime_error( "In Synch problem ");
    } else {
        cout << "In Synch OK " << retval << endl;
    }
    
}
/**
 * boot
 *    Load firmware and start the modules.
 *  @param bootmask     Boot mask passed to boot.
 */
void
CMyEventSegment::boot(SystemBooter::BootType type)
{
    if (m_systemInitialized) {
        int status = Pixie16ExitSystem(m_config.getNumberOfModules());
        if (status < 0) {
            std::stringstream errmsg;
            errmsg << "CMyEventSegment::boot() Failed to exit system with status=";
            errmsg << status << ".";
            throw std::runtime_error(errmsg.str());
        }
        m_systemInitialized = false;
    }

    try {
        SystemBooter booter;
        booter.boot(m_config, type);

        m_systemInitialized = true;

        // keep track of whether we loaded firmware... if we did, then we need
        // to sync next time around.
        m_firmwareLoadedRecently = (type == SystemBooter::FullBoot);
    } catch (std::exception& exc) {
        m_systemInitialized = false;
        std::cout << exc.what() << std::endl;
    }

}



int CMyEventSegment::GetCrateID() const
{
    return m_config.getCrateId();
}


////////////////////////////////////////////////////////////////////////
// Debug logging.

// For a very idiodic level parse of the raw data:
// C++'s standard for where bit fields live is not sufficiently
// precise to allow me to use them but...







/**
 * checkBuffer
 *    Ensure that a buffer of data from a Pixie is reasonable:
 *
 * @param pFifoContents - data from the FIFO.
 * @param nLongs        - Number of longs in the data.
 * @param id            - module id (slot in m_idToSlots[id]).
 */
void
CMyEventSegment::checkBuffer(const uint32_t* pFifoContents, int nLongs, int id)
{
    uint32_t slot = m_idToSlots[id];
    int lastSlot(-1);
    int lastChan(-1);
    int lastCrate(-1);
    while (nLongs > 0) {
        const HitHeader* pHeader = reinterpret_cast<const HitHeader*>(pFifoContents);
        int thisSlot = pHeader->getSlot();
        int thisChan = pHeader->getChan();
        int thisCrate = pHeader->getCrate();
        
        // The header must always be exactly 4 :
        
        if (pHeader->headerLength() != 4) {
           dumpHeader(pHeader, "Header length is not 4");
        }
        // The slot number must be correct:
        
        if (thisSlot != slot) {
           std::stringstream msg;
           msg << "Mismatch between expected and actual slots sb: "
            << slot << " was " <<  thisSlot << std::endl;
           msg << " Prior slot: " << lastSlot << " prior chan: " << lastChan;
           std::string s = msg.str();
           dumpHeader(pHeader, s.c_str());
        }
        
        pFifoContents += pHeader->eventLength();
        nLongs -= pHeader->eventLength();
        lastSlot = thisSlot;
        lastChan = thisChan;
        lastCrate = thisCrate;
    }
    if (nLongs < 0) {
        m_debugStream << "last event in buffer went off the end:";
        m_debugStream << "Slot: " << lastSlot << "Chan: " << lastChan
            << "Crate: " << lastCrate << " off end by " << -nLongs <<std::endl;
    }
}
/**
 * dumpHeader
 *    Dump a XIA hit header.
 *    @param pHeader - pointer to the header of a hit.
 *    @param msg     - Error Message.
 */
void
CMyEventSegment::dumpHeader(const void* pHeader, const char* msg)
{
    m_debugStream << msg << std::endl;
    const HitHeader* p = reinterpret_cast<const HitHeader*>(pHeader);
    
    m_debugStream << "Channel: " << p->getChan() << std::endl;
    m_debugStream << "Slot:    " << p->getSlot() << std::endl;
    m_debugStream << "Crate:   " << p->getCrate() << std::endl;
    m_debugStream << "Header length: " << p->headerLength() << std::endl;
    m_debugStream << "Event Length:  " << p->eventLength() << std::endl;
    m_debugStream << "Timestamp low: " << std::hex << p->s_tstampLow
        << std::dec << std::endl;
    m_debugStream << "Timestamp high" << std::hex << (p->s_tstampHighCFD & 0xffff)
        << " CFD: " << ((p->s_tstampHighCFD & 0xffff0000) >> 16)
        << std::dec << std::endl;
    m_debugStream << "Energy: " << (p->s_traceInfo & 0xffff) << std::endl;
    m_debugStream << "Trace length: " << ((p->s_traceInfo & 0x7fff0000) >> 16)
        << std::endl;
        
    m_debugStream << "-------------------------------------------------------\n";
    
}
