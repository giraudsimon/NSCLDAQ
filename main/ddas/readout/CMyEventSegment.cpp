#include <config.h>
#include "CMyEventSegment.h"
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
   m_pExperiment(&exp)
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
    for(unsigned int k=0; k<NumModules; k++) {
        auto type = hdwrMap.at(k);
        HR::HardwareSpecification specs = HR::getSpecification(type);

        ModuleRevBitMSPSWord[k] = (specs.s_hdwrRevision<<24) + (specs.s_adcResolution<<16) + specs.s_adcFrequency;
        ModClockCal[k] = specs.s_clockCalibration;

        cout << "Module #" << k << " : module id word=0x" << hex << ModuleRevBitMSPSWord[k] << dec;
        cout << ", clock calibration=" << ModClockCal[k] << endl;
        
        // Create the module reader for this module.
        
        m_readers.push_back(
            new DDASReadout::ModuleReader(
	      k, modEvtLengths[k],  ModuleRevBitMSPSWord[k], ModClockCal[k]
            )
        );
        cout << " Reader created\n";
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
    
    m_sorter = new DDASReadout::CHitManager(10.0);         // bit kludgy for now.
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
            int readSize = words[i];
            if (readSize > maxLongs) readSize = maxLongs;
            // Truncated to the the nearest event size:
            
            readSize -= (readSize % ModEventLen[i]);  // only read full events.

            // Read the data right into the ring item:
            
      //      std::cerr << "Going to read " << readSize
      //          << "(" << words[i] <<") " << " from " << i << std::endl;
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
    
    mytrigger->Reset();
    reject();
    return 0;

    
    /*  If the sorter has data to output - we get a hit from it
     *  output it and, if there are more hits, tell the Experiment we
     *  want to output more:
     */
    if (m_sorter->haveHit()) {
        size_t result = emitHit(rBuffer);

        if (m_sorter->haveHit()) {
            m_pExperiment->haveMore();    // We want to output more hits.
        }
        return result; 
    }
    // If we get here, we have a data in the modules.
    
    
    
    // Read the modules and hand the data to the hit manager.
    // if it says we have a hit that can be output output it and,
    // if three are outputtable hits, indicate that to the experiment:

    mytrigger->Reset();
    std::vector<std::deque<DDASReadout::ModuleReader::HitInfo>> moduleHits;
    
    for(int  i = 0; i < m_readers.size(); i++) {
        if (words[i]) {             // Don't bother with modules with no data:
            
            // By pushing back an empty hit list and then getting its reference,
            // for the reader, we avoid copying the hit list
            // If we'd just declared the deque read into it and then pushed it back,
            // the push_back would push a copy of all the hits.
            
            std::deque<DDASReadout::ModuleReader::HitInfo> tmp;
            moduleHits.push_back(tmp);
            auto& hits(moduleHits.back());
#ifdef SANITY_CHECKING
	    unsigned int nActual;
	    int status = Pixie16CheckExternalFIFOStatus(&nActual, i);
	    if (status < 0) {
	      std::cerr << "Sanity check fifo status read failed for module " << i << std::endl;
	      std::cerr << "Status: " << status << std::endl;
	    }
	    if (nActual < words[i]) {
	      std::cerr << "Fewer actual words than trigger words actual: "
			<< nActual << " trigger: " << words[i]
			<< " module: " << i << std::endl;
	    }
#endif	    
            m_readers[i]->read(hits, words[i]);
        }
    }
    m_sorter->addHits(moduleHits);
    
    // Can we output any hits?
    
    if (m_sorter->haveHit()) {
        size_t nWords = emitHit(rBuffer);
        
        if (m_sorter->haveHit()) {
            m_pExperiment->haveMore();    // We want to output more hits.
        }
        return nWords;
    }
    
    // If we landed here, we read stuff out but there were no
    // words ready to output yet;
    // Reject the trigger:
    
    reject();
    return 0;                      // nothing read.
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
  std::cerr << "Ending run - flushing data in the hit manager\n";
  
    //Flush hits from the hit manager:
    
    m_sorter->flushing(true);
    if (m_sorter->haveHit()) {  // In case there aren't any:
        
        
        m_pExperiment->ReadEvent();      // read will request retrigger till done.
    }
    std:: cerr << "Done\n";
    m_sorter->flushing(false);
    for (int i =0; i < m_readers.size(); i++) {
        m_readers[i]->reset();                 // Clear last known channel timestamps.
    }
    std::cerr << "Finished with end run action\n";

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

/**
 * emitHit
 *    Emit  a single hit from the sorter:
 *
 *  @return size_t number of 16 bit words emitted:
 */
size_t
CMyEventSegment::emitHit(void* pBuffer)
{
        auto hitInfo = m_sorter->getHit();
        auto& hit(hitInfo.second);
        
        uint32_t* pWords = static_cast<uint32_t*>(pBuffer);
        *pWords++ = hitInfo.first->m_moduleTypeWord;
        
        // Put the channel data in the buffer and set the timestamp:
        size_t nBytes = hit->s_channelLength * sizeof(uint32_t);
        memcpy(pWords, hit->s_data, nBytes);
        setTimestamp(uint64_t(hit->s_time));
        
        DDASReadout::ModuleReader::freeHit(hitInfo);   // Return hit storage.
        
        return nBytes/sizeof(uint16_t) + sizeof(uint32_t)/sizeof(uint16_t); 
}
