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

const uint32_t CCSRA_EXTTSENAmask(1 << 21);    // Mask for ext ts enable.
const uint32_t ModRevEXTCLKBit(1 << 21);   // Ext clock indicator.

CMyEventSegment::CMyEventSegment(CMyTrigger *trig, CExperiment& exp)
 : m_config(),
   m_systemInitialized(false),
   m_firmwareLoadedRecently(false),
   m_pExperiment(&exp),
   m_nCumulativeBytes(0),
   m_nBytesPerRun(0)
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
    int numInternalClock(0);
    int numExternalClock(0);
    for(unsigned int k=0; k<NumModules; k++) {
        auto type = hdwrMap.at(k);
        HR::HardwareSpecification specs = HR::getSpecification(type);

        
        ModuleRevBitMSPSWord[k] = (specs.s_hdwrRevision<<24) + (specs.s_adcResolution<<16) + specs.s_adcFrequency;
        ModClockCal[k] = specs.s_clockCalibration;
        
        // Fold in the external clock  - in our implemenation,
        // all channels save the external clock or none do..
        // We'll determine if all do by looking at the CCSRA_EXTSENA
        // bit of channel control register A of channel 0.
        // We assume that resolution is limited to 16 bits max
        // making the resolution field of the ModuleRevBitMSPSWord 5 bits
        // wide;  So we'll put a 1 in bit 21 if the external clock is used:
        
        
        double fCsra = fCsra;
        int stat = Pixie16ReadSglChanPar(
           "CHANNEL_CSRA", &fCsra,  k, 0);
        if (stat < 0) {
         std::cerr << "****ERROR Failed to read chanel CSRA Module: " << k
                   << " status: " << stat << std::endl;
         std::exit(EXIT_FAILURE);  // Can't go on.
        }
        // External clock mode:
        
        uint32_t csra = static_cast<uint32_t>(fCsra);
        if (csra & CCSRA_EXTTSENAmask) {
         ModuleRevBitMSPSWord[k] |= ModRevEXTCLKBit;
         numExternalClock++;
         
         // In external clock mode, the default clock scale factory
         // is 1 but the DDAS_TSTAMP_SCALE_FACTOR environment
         //variable can override this.
         // Note that our implementation doesn't well support a mix
         // of internal and external timestamps in a crate:
         
         ModClockCal[k] = 1;
         const char* override = std::getenv("DDAS_TSTAMP_SCALE_FACTOR");
         if (override) {
          ModClockCal[k] = atof(override);
          if (ModClockCal[k] <= 0) {
           std::cerr << "Invalid value for DDAS_TSTAMP_SCALE_FACTOR: '"
            << ModClockCal[k] << "'\n";
           std::exit(EXIT_FAILURE);
          }
         }
         
        } else {
         numInternalClock++;
        }
        // We don't really support both internal and external
        // clocks in the same readout at present
        
        if ((numInternalClock > 0) && (numExternalClock > 0)) {
         std::cerr << "Some modules are set for internal while others for external clock\n";
         std::cerr << "This is not a supported configuration\n";
         std::exit(EXIT_FAILURE);
        }
        

        cout << "Module #" << k << " : module id word=0x" << hex << ModuleRevBitMSPSWord[k] << dec;
        cout << ", clock calibration=" << ModClockCal[k] << endl;
        if (ModuleRevBitMSPSWord[k] & ModRevEXTCLKBit) {
         cout << "External clock timestamping will be used\n";
         cout << " With a clock multiplier of " << ModClockCal[k] << std::endl;
        }
        

        // Create the module reader for this module.
        
       
    }

    mytrigger->Initialize(NumModules);


   
    
}

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
      cout << "List Mode started OK " << retval << " mode " << std::hex << std::showbase << LIST_MODE_RUN << std::dec << " " << NEW_RUN << endl << flush;
    }
    m_nBytesPerRun = 0;                // New run presumably.
    usleep(100000); // Delay for the DSP boot 



}


/* Pixie16 has triggered.  There are greater than  EXTFIFO_READ_THRESH 
   words in the output FIFO of a particular Pixie16 module.  Read out 
   all pixie16 modules. */

//  virtual size_t read(unsigned short* rBuffer, size_t maxwords);
// Note - maxwords is actually maxbytes.

size_t CMyEventSegment::read(void* rBuffer, size_t maxwords)
{
  // memset(rBuffer, 0, maxwords);            // See what's been read.
  
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
            double* pd = reinterpret_cast<double*>(p);
            *pd++       = ModClockCal[i];   // Clock multiplier.
            maxLongs -= (sizeof(double)/sizeof(uint32_t)); // Count it.
            p          = reinterpret_cast<uint32_t*>(pd);
            
            int readSize = words[i];
            if (readSize > maxLongs) readSize = maxLongs;
            // Truncated to the the nearest event size:
            
            readSize -= (readSize % ModEventLen[i]);  // only read full events.

            // Read the data right into the ring item:
            
            //std::cerr << "Going to read " << readSize
            //    << " (I think " << words[i] <<" remain) " << " from module " << i << std::endl;
	    auto prewords = words[i];
	    unsigned int preread;
	    Pixie16CheckExternalFIFOStatus(&preread, i);
	    //std::cerr << "--> Pre-read: FIFO module " << i << " contains " << remaining << " words" << std::endl;

	    int stat = Pixie16ReadDataFromExternalFIFO(
                reinterpret_cast<unsigned int*>(p), (unsigned long)readSize, (unsigned short)i
            );
	    if (stat != 0) {
                std::cerr << "Read failed from module " << i << std::endl;
                m_pExperiment->haveMore();
                reject();
                return 0;
            }

	    unsigned int postread;
	    Pixie16CheckExternalFIFOStatus(&postread, i);
	    //std::cerr << "--> Post-read: FIFO module " << i << " contains " << remaining << " words" << std::endl;
	    
	    m_pExperiment->haveMore();      // until we fall through the loop
            words[i] -= readSize;           // count down words still to read.
	    	    
            // maintain statistics and 
            // Return 16 bit words in the ring item body.
            
            size_t nBytes = sizeof(double) + (readSize+1)*sizeof(uint32_t);
            m_nCumulativeBytes += nBytes;
            m_nBytesPerRun     += nBytes;
            
            return nBytes/sizeof(uint16_t);
        }
    }
    // If we got here nobody had enough data left since the last trigger:
    //std::cerr << "Not enough data to read since last trigger:" << std::endl;
    //for(int i=0; i<NumModules; i++) {
    //  std::cout << words[i] << std::endl;
    //}
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
