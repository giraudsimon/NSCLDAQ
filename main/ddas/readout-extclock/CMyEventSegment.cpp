#include <config.h>
#include <config_pixie16api.h>
#include "CMyEventSegment.h"
#include "Configuration.h"
#include "HardwareRegistry.h"
#include "SystemBooter.h"
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
#include <stdexcept>
#include "CMyTrigger.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

using namespace DAQ::DDAS;

#define CHANNELIDMASK             0xF  // bits 0-3 inclusive
#define SLOTIDMASK               0xF0  // bits 4-7 inclusive
#define CRATEIDMASK             0xF00  // bits 8-11 inclusive
#define HEADERLENGTHMASK      0x1F000  // bits 12-16 inclusive
#define CHANNELLENGTHMASK  0x3FFE0000  // bits 17-29 inclusive
#define OVERFLOWMASK       0x40000000  // bit 30
#define FINISHCODEMASK     0x80000000  // bit 31
#define LOWER16BITMASK         0xFFFF  // lower 16 bits

#define  FILENAME_STR_MAXLEN     256


/* Comparator function needed for the vector sort.  Function will be 
   given the start location of two different channel events.  The time 
   information is stored in the second word and the lower 16 bits of 
   the third word of the channel event. Function returns true if ch1
   event has lower timestamp than ch2. */

bool cmp(const unsigned int  *ch1, const unsigned int *ch2){

    /* I think the math here was wrong anyway - should mult by 2^32 */
    //  if( (*(ch1 + 1) + ((*(ch1 + 2)&LOWER16BITMASK) * 65536)) < 
    //      (*(ch2 + 1) + (((*(ch2 + 2)&LOWER16BITMASK)) * 65536))) {
    //    return true;
    //  } else {
    //    return false;
    //  }

    /* Do time sorting by checking slow time first, then if the slow
       times are the same, compare the fast times */

    if ( (*(ch1 + 2)&LOWER16BITMASK) < (*(ch2 + 2)&LOWER16BITMASK) ) {
        return true;
    } else if ( ((*(ch1 + 2)&LOWER16BITMASK) == (*(ch2 + 2)&LOWER16BITMASK)) &&
            (*(ch1 + 1) < *(ch2 + 1)) ) {
        return true;
    } else {
        return false;
    }

}

bool cmp1(const channel *c1, const channel *c2){
    if(c1->time < c2->time) return true;
    else return false;

}


channel::channel()
{
    time = 0;
    //data.resize(4,0);
}

channel::~channel()
{

}

int channel::SetTime(){

    if(data.size()>=4){
        time = (data[2]&LOWER16BITMASK)*4294967296. + data[1];
        return 0;
    }
    else
        return 1;
}

int channel::SetChannelLength(){
//    cin.get();
    if(data.size()>=4){
        channellength = (data[0] & CHANNELLENGTHMASK)>>17; 
        return 0;
    }
    else
        return 1;
}

int channel::SetChannel(){
    if(data.size()>=4){
        chanid = (data[0] & CHANNELIDMASK);
        return 0;
    }
    else{
        return 1;
    }
}

int channel::Validate(int evtlen){

    //return values
    // 0 - data is good
    // 1 - channel length within data does not match external input file

    if(channellength != evtlen){
        cout << "data is corrupt or setting in ModEvtLen.txt is incorrect " << endl;
        cout << "Channel length is " << channellength << " - was expecting " << evtlen << " from modevtlen.txt file " << endl;
        return 1;
    }
    else{
        return 0;
    }

}


CMyEventSegment::CMyEventSegment(CMyTrigger *trig) :
  m_tstampScaleFactor(1),
  m_config(),
  m_systemInitialized(false),
  m_firmwareLoadedRecently(false)
{

    ios_base::sync_with_stdio(true);

    // Trigger object
    mytrigger = trig;

    retval = 0;
    //mod_numwordsread = 0;

    // Initialize the Pixie16 modules
    cout << "Trying to initialize pixie16 " << endl << flush;

    const char* fwFile =  FIRMWARE_FILE;   // Default.
    char* alternateFirmwareFile = getenv("EXTCLK_FIRMWARE_FILE");
    if (alternateFirmwareFile) fwFile = alternateFirmwareFile;
 
    m_config = *(Configuration::generate(
        fwFile, "cfgPixie16.txt", "modevtlen.txt")
    );

    m_config.print(std::cout);
    std::cout << std::endl;

    NumModules = m_config.getNumberOfModules();

    auto modEvtLengths = m_config.getModuleEventLengths();
    ModEventLen = new double[NumModules+1];
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
    bool booted = false;
    if (getenv("DDAS_BOOT_WHEN_REQUESTED") == NULL) {
        boot();
        booted = true;
    } else {
        boot(SystemBooter::SettingsOnly);
    }

    if (isHeterogeneousSystem(m_config.getHardwareMap())) {
        std::string errmsg("Detected heterogeneous hardware in crate. ");
        errmsg += "Readout_extcl only supports systems where all digitizers ";
        errmsg += "are the same!";
        throw std::runtime_error(errmsg);
    }

    ///////
    //  Create extra word for number of bits and ADC sampling rate. //
    namespace HR = HardwareRegistry;

    std::vector<int> hdwrMap = m_config.getHardwareMap();
    for(unsigned short k=0; k<NumModules; k++) {
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
    
    std::cout << "*** REDIRECTING output on std::cout to /scratch/data/dump.dat" << std::endl;

    // For the purpose of debugging... we want all of the output to go to 
    // the output file
    std::cout.rdbuf(ofile.rdbuf());
#endif 

    // specify the time_buffer... at the moment this is in clock ticks
    // and will need to be changed to deal with actual time values.
    // To do that, we will need a way to specify which digitizer is used
    // and what its sampling rate is.
    // For the 100MHz, this is 10 s
    // For the 250MHz, this is 4 s
    time_buffer = 1000000000; 

}

bool CMyEventSegment::isHeterogeneousSystem(const std::vector<int> &hdwrMap)
{
    unsigned int moduleZeroHardwareType = hdwrMap.at(0);
    auto pBegin = hdwrMap.begin();
    auto pEnd   = hdwrMap.end();

    auto pResult = std::find_if_not(pBegin, pEnd,
                                    [moduleZeroHardwareType](const unsigned int& value) {
                                        return moduleZeroHardwareType == value;
                                    });
    return (pResult != pEnd);
}


CMyEventSegment::~CMyEventSegment()
{
    // Nothing to clean up
    //cout << " frag out " << fragcount << endl;
}

void CMyEventSegment::initialize(){

    fragcount = 0;

    m_processing = false;

    // Initialize unless there is an INFINITY_CLOCK environment variable
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

    // Get the scale factor for timestamp
    std::string scaleFactor;
    envVal = std::getenv("DDAS_TSTAMP_SCALE_FACTOR");
    if (envVal) scaleFactor = envVal;
    if (!scaleFactor.empty()) {
      std::cout << "Scale factor set to " << scaleFactor << std::endl;
      m_tstampScaleFactor = std::atof(scaleFactor.c_str());
    }

    /***** Start list mode run *****/
    retval = Pixie16StartListModeRun (NumModules, LIST_MODE_RUN, NEW_RUN);
    if (retval < 0) {
        cout << "*ERROR* Pixie16StartListModeRun failed " << retval 
            << endl << flush;
    } else {
        cout << "List Mode started OK " << retval << endl << flush;
    }

    nFIFOWordsinModuleCurrentRead.resize(NumModules,0);
    nFIFOWordsinModuleTotal.resize(NumModules,0);
    CurrentTimeinModuleRead.resize(NumModules,0);
    FirstTimeinModuleRead.resize(NumModules,0);

    for(int z=0;z<NumModules;z++){
        ModuleDeque[z].clear();
    }

    usleep(100000); // Delay for the DSP boot 

    //initialize global time
    globaltime = 0;

}


/* Pixie16 has triggered.  There are greater than  EXTFIFO_READ_THRESH 
   words in the output FIFO of a particular Pixie16 module.  Read out 
   all pixie16 modules. */

//  virtual size_t read(unsigned short* rBuffer, size_t maxwords);
size_t CMyEventSegment::read(void* rBuffer, size_t maxwords)
{

    int channellength = 0;
    try{


        processdata = true;




        /* Check to determine if this is a fresh read or a continuation 
           of previous buffer processing. */
        if(!m_processing) {

            // need new data from ddas modules for processing

            // **
            //nFIFOWordsinModulePrevious.clear();
            //nFIFOWordsinModulePrevious.resize(NumModules);

            // **
            //for(int i=0;i<NumModules;i++){
            //  nFIFOWordsinModulePrevious[i] = nFIFOWordsinModuleCurrentRead[i];
            //}

            nFIFOWordsinModuleCurrentRead.clear();

            /* This is a new read, set a processing buffer flag. */
            m_processing = true;

            totwords = 0;

#ifdef PRINTQUEINFO
            ofile << "Query modules until complete data exists" << std::endl;
#endif

            // flag for reattempting data 
            bool retrigger = true;
            int attempts = 0;
            while(retrigger){
                if(attempts>0) usleep(100);
                retrigger = false;
                //totwords = 0;
                //nFIFOWordsinModuleCurrentRead.clear();
                /* Loop over modules to determine how many words to read from each module. */

                for(int i=0; i<NumModules;i++) {

                    unsigned short ModNum = (unsigned short)i;

                    unsigned int temp = 0;

                    /* Check the number of words in the FIFO, it may have changed 
                       since the trigger was issued */
                    retval = Pixie16CheckExternalFIFOStatus(&nFIFOWords,ModNum); //SNL modified
                    if(retval < 0) {
                        cout << "Failed to check FIFO status in module " 
                            << ModNum << endl << flush;
                    }

                    //cout << "words in mod " << i << " " << nFIFOWords << " " << " attempt " << attempts << endl;

                    //in the real world, enforce the requirement that all channels within one module
                    //readout the same event structure or the following code segmenets will fail
                    temp = nFIFOWords;

                    // logic below no longer required.  Firmware fixed
                    //nFIFOWords = (unsigned long)(floor(nFIFOWords/4.) * 4);


                    //      nFIFOWords = (unsigned long)(floor(nFIFOWords/ModEventLen[ModNum]) * ModEventLen[ModNum]); 
                    //need to retrigger until we have greater than the ceiling number of words in the module
                    nFIFOWords = (unsigned long)(ceil(nFIFOWords/ModEventLen[ModNum]) * ModEventLen[ModNum]);
                    

                    if(attempts == 0){
                        nFIFOWordsinModuleCurrentRead.push_back(nFIFOWords);
                        /* Sum the total number of words to read out across the crate. */
                        totwords = totwords + nFIFOWords;
                    }	

#ifdef PRINTQUEINFO
                    ofile << "attempt=" << std::setw(4) << attempts
                        << " mod=" << ModNum 
                        << " ... nFIFOWords required=" << nFIFOWordsinModuleCurrentRead[ModNum] << std::endl;
#endif

                    if(temp < nFIFOWordsinModuleCurrentRead[ModNum]){
                        //cout << "readout truncation in Mod " << ModNum 
                        //     << " words in mod - " << temp 
                        //     << " words to read " << nFIFOWordsinModuleCurrentRead[ModNum] << endl;
                        retrigger = true;
                    }
                    else {
                        //cout << " readout from Mod " << ModNum << " - " << nFIFOWords << endl;	
                    }

                    /* Retain the number of words to read out in this module from the first pass. */


                }
                attempts++;
                if(attempts > 300){
                    totwords = 0;
                    for(int z=0; z<nFIFOWordsinModuleCurrentRead.size();z++){
                        //truncated and pick up next event later
                        //cout << "event later " << z << " : " << nFIFOWordsinModuleCurrentRead[z] << " " <<ModEventLen[z] << endl;
                        int trunc = (floor(nFIFOWordsinModuleCurrentRead[z]/ModEventLen[z])-1);
                        if(trunc < 0) trunc = 0;
                        nFIFOWordsinModuleCurrentRead[z] = (unsigned long)(trunc * ModEventLen[z]);
                        totwords = totwords + nFIFOWordsinModuleCurrentRead[z];
                        //cout << "pick up next event later " << z << " : " << nFIFOWordsinModuleCurrentRead[z] << endl;
                    }
#ifdef PRINTQUEINFO
                    ofile << "Incomplete read" << std::endl;
#endif
                    break;
                };

            }

            // If there was not data read, don't bother bogging down communication with trying to
            // to read nothing from the modules... Simply exit.
            if (totwords == 0) {
                // wait a little time
                usleep(100);
                m_processing=false;
                mytrigger->Reset();    
                reject();
                return 0;
            }

            for(int i=0; i<NumModules; i++){
                ModuleData[i].resize(nFIFOWordsinModuleCurrentRead[i]);
            }

            //    unsigned int DataWordsCopied;
            //DataWordsCopied = 0;

#ifdef PRINTQUEINFO
            ofile << "Polls to module = " << attempts << " total words read=" << totwords << std::endl;
            ofile << "Before read data in Que:" << std::endl; 
            for(int z=0; z<NumModules; ++z){
                ofile << std::setw(5) << ModuleDeque[z].size() << " "; 
                ofile << flush;
            }
            ofile << std::endl;
#endif

            // Knowing how much data to read, read out all modules
            for(int i = 0;i<NumModules;i++) {
                //cout << "try to read data " << i << " " << nFIFOWords << endl;

                int ModNum = i;

                // Number of words in this module
                nFIFOWords = nFIFOWordsinModuleCurrentRead[i];
                CurrentTimeinModuleRead[i] = 0;
                FirstTimeinModuleRead[i] = 0;

                /* Read nFIFOWords of data from the pixie16 module into 
                   variable ExtFIFO_Data only if there are words to read */
                if(nFIFOWords /*> 0*/ > 2) {
                    //unsigned int *ExtFIFO_Data;

                    //cout << "reading mod num " << ModNum << " nwords " << nFIFOWords << endl;

                    //ExtFIFO_Data = (unsigned int *)malloc(sizeof(unsigned int)*nFIFOWords);

                    //cout << "being read "<< endl;
                    // read data from module
                    retval = Pixie16ReadDataFromExternalFIFO(/*&CrateData[DataWordsCopied],&temp[DataWordsCopied],*/&ModuleData[i][0] , nFIFOWords, ModNum);
                    if(retval !=0 ) {
                        cout << "Failed to read data from module " << ModNum << endl << flush;
                    }
		    else {
			//cout << "read this much " << i << " "
			//     << retval << endl;
		    }

                    //cout << "end read " << ModuleData[i].size() << endl;
                    
                    // tstamp is 48 bits. IEEE754 double precision floating point numbers have 52 bits of precision
                    // 11 bits for the exponent, and 1 bit for the sign. Therefore, a double is perfectly capable
                    // of perfectly representing the timestamp.
    
                    // Note that the scope of these is local to the iteration in the for loop. Therefore, they
                    // are reset for every module.
                    double tmp_mintime=DBL_MAX;
                    double tmp_maxtime=0;
                    double tmp_prevtime=0;
                    double tmp_time=0;
                    bool data_needs_sort=false;
                    std::deque<channel*> tmp_deque;
                    // take module data, parse it into individual channel objects, and place channel objects into module deque
		    //cout << "que size " << i << " " << ModuleData[i].size() << endl;

		    //for(int zz=0;zz<ModuleData[i].size();zz=zz+ModEventLen[i]){
			//cout << "  zz " << zz << " data " ;
			//for(int q=0;q<ModEventLen[i];q++){
			//	cout << ModuleData[i][zz+q] << " ";
			//}
			//cout << endl;
		    //}
		    //cout << endl;

                    for(int z=0; z<ModuleData[i].size(); z=z+ModEventLen[i]){
                        //cout << "  z " << z << " data " ;
			//for(int q=0;q<ModEventLen[i];q++){
			//  cout << "data " << ModuleData[i][z+q];
			//}
			//cout << endl;
                        channel *tempdchan = new channel();
                        tempdchan->data.insert(tempdchan->data.end(),ModuleData[i].begin()+z,ModuleData[i].begin()+z+(uint32_t(ModEventLen[i])));
                        int rv = tempdchan->SetTime();
                        tmp_time = tempdchan->GetTime();
                        if ( tmp_time < 25 ) {
                          std::cout << "mod=" << i << " z=" << z << " found time less than 25 " << std::endl;
                        } 

                        // while we are loading the data, keep track of some simple
                        // information about its time to use later.
                        if (tmp_time > tmp_maxtime) tmp_maxtime = tmp_time;
                        if (tmp_time < tmp_mintime) tmp_mintime = tmp_time;
                        if (z==0) {
                             tmp_prevtime = tmp_time;
                        } else {
                             // if the consecutive events do not have monotonically increasing 
                             // timestamps, then flag this batch of data for sorting
                             if (tmp_time < tmp_prevtime) data_needs_sort = true;
                             tmp_prevtime = tmp_time;   
                        }

                        //cout << "    time " << tempdchan->GetTime();
                        if(rv == 1) cout << "Data stream appears corrupt - less than 4 words in channel data " << endl;
                        rv = tempdchan->SetChannelLength();
                        //cout << "    channel length " << tempdchan->GetChannelLength();
                        if(rv == 1) cout << "Data stream appears corrupt - less than 4 words in channel data " << endl;
#ifdef PRINTQUEINFO
                        rv = tempdchan->SetChannel();
                        if(rv == 1) cout << "Data stream appears corrupt - less than 4 words in channel data " << endl;
#endif
                        rv = tempdchan->Validate(ModEventLen[i]);
                        if(rv != 0) cout << "Data or setting appear to be corrupt - Module " << i << " code " << rv << endl;
#ifdef PRINTQUEINFO
                        ofile << "mod=" << std::setw(2) << i 
                            << " chan=" << std::setw(3) << tempdchan->GetChannel() 
                            << " tstamp=" << tempdchan->GetTime() << std::endl;
#endif

//                        ModuleDeque[i].push_back(tempdchan);
                        tmp_deque.push_back(tempdchan);
                        nFIFOWordsinModuleTotal[i] += ModEventLen[i];

                    }

                    // Clear out data raw data 
                    ModuleData[i].clear();

                    if (tmp_deque.size()>0) {

#ifdef PRINTQUEINFO
                        ofile << "Data read from module " << i 
                            << "\n-------------------------" << std::endl;
                        ofile << "\t tmin=" << tmp_mintime << std::endl;
                        ofile << "\t tmax=" << tmp_maxtime << std::endl;
                        ofile << "\t data_needs_sort=" << data_needs_sort << std::endl;
#endif
                        // Only sort the recently read data if it was found to be out 
                        // of order because this is an expensive operation.
                        if (data_needs_sort) {
#ifdef PRINTQUEINFO
                            ofile << "Local_sort " << std::flush;
#endif
                            sort(tmp_deque.begin(),tmp_deque.end(),cmp1); 
                        } 

                        // We are guaranteed now that the data just read from the module is time ordered.
                        // As long as the highest time stamp in that data is later than the last thing stored in the
                        // associated queue, we need only to push the data into the queue. 
                        if (ModuleDeque[i].size()>0) {
#ifdef PRINTQUEINFO
                            ofile << "moddeque_notempty ... ";
#endif
                            if (tmp_mintime >= ModuleDeque[i].back()->GetTime()) {
#ifdef PRINTQUEINFO
                                ofile << "simple_insertion" << std::endl;
#endif
                                PushDataOntoQueue(ModuleDeque[i],tmp_deque);
                            } else {

#ifdef PRINTQUEINFO
                                ofile << "extra_sort_insertion"<< std::endl;
#endif
                                // We have found that the recently read data does not come later
                                // than the newest data in the queue. 

                                // The queue is already time ordered, so the entire thing doesn't need to be resorted.
                                // Instead, only the portions of it that will be affected by the new data should be 
                                // resorted.

                                PushDataOntoQueue(ModuleDeque[i],tmp_deque);

                                // Find the region that is affected.
                                // Set our iterator to the very end of the deque
                                std::deque<channel*>::iterator it = ModuleDeque[i].end();
                                --it;

                                // Skip over the data that was just pushed on the queue
                                std::advance(it,-1*tmp_deque.size());
                                std::deque<channel*>::iterator first = it;

                                // proceed to iterate backwards through the que to find the data that is out of
                                // order. The queue should have been time ordered already and thus there should 
                                // be no problem stopping our search immediately
                                while ((*it)->GetTime()>tmp_mintime && it!=ModuleDeque[i].end()) --it;

#ifdef PRINTQUEINFO
                                ofile << " distance into que of offending tstamp @ " << std::distance(it,first) << std::endl;
#endif
                                // Sort only the affected portion
                                sort(it,ModuleDeque[i].end(),cmp1);
                            }
                        } else {

                            // No data was in the queue. We just need to add the newest data.
#ifdef PRINTQUEINFO
                            ofile << "moddeque_empty ... simple_insertion" << std::endl;
#endif
                            PushDataOntoQueue(ModuleDeque[i],tmp_deque);
                        }

                    }
                } // End if nFIFOWords > 0

                if (ModuleDeque[i].size()>0) {
                    FirstTimeinModuleRead[i] = ModuleDeque[i].front()->GetTime();
                    CurrentTimeinModuleRead[i] = ModuleDeque[i].back()->GetTime();
                } 

                // Clear out the temporary storage
                ModuleData[i].clear();

                //temporary check data from one deque to ensure it is in time order
                double curtime = 0;
                double oldtime = 0;
                //for(int z=0;z<ModuleDeque[i].size();z++){ //remove SNL
                //    curtime = ModuleDeque[i][z]->time;  
                //    //cout << "times " << curtime << " " << oldtime << endl;
                //    if(curtime >= oldtime){
                //        oldtime = curtime;
                //    }
                //    else{
                //        cout << "Time out of order: mod=" << i << " oldtime=" << oldtime << " curtime=" << curtime <<endl;
                //        exit(1);
                //    }
                //}
                //       if(ModuleDeque[i].size()>0) cout << "done with read " << i << " " << ModuleDeque[i].size() 
                // 				       << " first time " << FirstTimeinModuleRead[i] 
                // 				       << " current time " << CurrentTimeinModuleRead[i] << endl;


            } // Finished reading data out of modules


#ifdef PRINTQUEINFO
            ofile << "After read data in Que:" << std::endl; 
            for(int z=0; z<NumModules; ++z){
                ofile << std::setw(5) << ModuleDeque[z].size() << " ";
                ofile << flush;
            }
            ofile << std::endl;
#endif


            //     for(int z=0;z<ModuleDequeTemp.size();z++){
            //       for(int k=0;k<ModuleDequeTemp[z]->data.size();k++){
            // 	//cout << ModuleDequeTemp[z]->data[z] << " ";
            //       }
            //       //cout << endl;
            //       delete (ModuleDequeTemp[z]);
            //     }
            //     ModuleDequeTemp.clear();

            //temp.clear();


            int j = 0;

            // determine to what time the data should be extracted from the deques.
            // After any given read, the data will extracted up to the time 
            // halfway between the earliest FirstTimeinModuleRead and 
            // latest CurrentTimeinModuleRead
            StopTime = 0;
            double mintime = DBL_MAX;
            double maxtime = 0;
            for(int i=0; i<NumModules; i++){

                // only check for modules with data
                //                if(nFIFOWordsinModuleCurrentRead[i] > 0){
                if(ModuleDeque[i].size() > 0){
                    if(FirstTimeinModuleRead[i]   < mintime) mintime = FirstTimeinModuleRead[i];
                    if(CurrentTimeinModuleRead[i] > maxtime) maxtime = CurrentTimeinModuleRead[i];
                }

            }

#ifdef PRINTQUEINFO
            //            cout << "times " << static_cast<unsigned long long>(mintime) << " " << static_cast<unsigned long long>(maxtime) << endl;
            ofile << "mintime=" << mintime << "\tmaxtime=" << maxtime << endl;
#endif

            //            double time_from_min = (maxtime - mintime)/2.;
            //            StopTime = mintime + time_from_min; 

            StopTime = maxtime - time_buffer; 

            // If we have not filled the buffer yet, the above StopTime calculation will 
            // set StopTime to a negative value. In this case, keep filling the buffer.
            if (StopTime<0) {
                m_processing = false;
                reject();
                mytrigger->Reset();
                return 0; 
            }

            if(StopTime < 0){
                cout << "Problem with determining Stop Time " << StopTime << endl;
                for(int z=0;z<NumModules;z++){
                    cout << " first times " << FirstTimeinModuleRead[z] << " current times " << CurrentTimeinModuleRead[z] << endl;
                }
            }
            //            if(StopTime < globaltime){
            //                cout << "Determine a really low StopTime ..."
            //                    <<"\nstop" << StopTime
            //                    << " glbl=" << globaltime
            //                    << " min=" << mintime 
            //                    << " max=" << maxtime << endl;	 
            ////                cout << "Determine a really low StopTime " << StopTime << " " << globaltime << " " << mintime << " " <<maxtime << endl;	 
            //                for(int z=0;z<NumModules;z++){
            //                    cout << nFIFOWordsinModuleCurrentRead[z] << " first times " << FirstTimeinModuleRead[z] << " current times " << CurrentTimeinModuleRead[z] << " deque[" << z << "].size=" << ModuleDeque[z].size() << endl;
            //                }
            //            }

#ifdef PRINTQUEINFO
            //            cout << "StopTime " << StopTime << endl;
            ofile << "StopTime = " << StopTime << endl;
#endif

            } // End pixie16 module readout (i.e. m_processing block is finished)


            ///////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////

            channellength = SelectivelyOutputData(rBuffer,maxwords);
        }
        catch(...){
            cout << "exception in event segment " << endl;
        }
        return channellength;
}

void CMyEventSegment::PushDataOntoQueue(std::deque<channel*>& deque, std::deque<channel*>& buffer)
{
    deque.insert(deque.end(),buffer.begin(), buffer.end());
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

    // Set the stop time to a gigantic value that is guaranteed to 
    // be greater than the most recent timestamp
    StopTime = DBL_MAX;

    // Skip read
    m_processing=true;


#ifdef PRINTQUEINFO
    std::cout << "Run ended" << std::endl;
    std::cout << "Data remaining to send to EVB" << std::endl;
    for (int mod=0; mod<NumModules; ++mod) {
        std::cout << std::setw(6) << ModuleDeque[mod].size();
    }
    std::cout << std::endl;    

    std::cout << "\nSending remaining data ... " << std::flush;
#endif

    // loop deques have no more data 
    int i=0;
    while ( DataToRead() ) {

        // Send the next oldest piece of data to the 
        // ring and pop it off the deque
        pExperiment->ReadEvent();

//        usleep(50);

#ifdef PRINTQUEINFO
        for (int mod=0; mod<NumModules; ++mod) {
            std::cout << std::setw(6) << ModuleDeque[mod].size();

        }
        std::cout << std::endl;    
#endif
        ++i;
    }
#ifdef PRINTQUEINFO
    std::cout << "complete" << std::endl; 

    std::cout << "Size of each queue"<< std::endl;
    for (int mod=0; mod<NumModules; ++mod) {
        std::cout << std::setw(6) << ModuleDeque[mod].size();
    }
    std::cout << std::endl;    
#endif

}
/*
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
    retval = Pixie16WriteSglModPar(const_cast<char*>("SYNCH_WAIT"), 1, modnum);
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


bool CMyEventSegment::DataToRead()
{
    // Find out if any deques have more data
    bool ret_val=false;
    for (unsigned int mod=0; mod<NumModules; ++mod) {
        if (ModuleDeque[mod].size()!=0) {
            ret_val = true;
            break;
        }
    }
    return ret_val;
}

size_t CMyEventSegment::SelectivelyOutputData(void* rBuffer, size_t maxwords)
{

    double currenttime = -1;
    int ModuleDequeLowTime = -1;
    bool nodata = true;

    if(globaltime < StopTime) {

        for(int i=0;i<NumModules;i++){

            // still have data to process within this deque from the previous readout
            if( ModuleDeque[i].size()>0 ){
                nodata = false;
#ifdef PRINTQUEINFO
                ofile << "mod=" << i << " lowtime0=" << ModuleDeque[i][0]->time; //[2]&LOWER16BITMASK)*4294967296. + ModuleDeque[i][1];
                if (ModuleDeque[i].size()>1) 
                    ofile << " lowtime1=" << ModuleDeque[i][1]->time;
                if (ModuleDeque[i].size()>2) 
                    ofile << " lasttime=" << ModuleDeque[i].back()->time;
                ofile << " size=" << ModuleDeque[i].size(); 
                ofile << std::endl; 
#endif
                // initialize currenttime to the time in the first deque found with data
                if(currenttime == -1){
                    // initialize time to first deque with valid data
                    currenttime = (ModuleDeque[i][0]->time); //[2]&LOWER16BITMASK)*4294967296. + ModuleDeque[i][1];
                    ModuleDequeLowTime = i;
                }
                else{
                    // check to determime if the present module has the lowest time
                    double temptime;
                    temptime = (ModuleDeque[i][0]->time); //[2]&LOWER16BITMASK)*4294967296. + ModuleDeque[i][1];
                    if(temptime < currenttime){
                        currenttime = temptime;
                        ModuleDequeLowTime = i;
                    }

                }

            }
        }

#ifdef PRINTQUEINFO
        ofile << "selected mod=" << ModuleDequeLowTime 
            << " time=" << ModuleDeque[ModuleDequeLowTime][0]->time << endl;  
#endif
    }

    // At this point it has found a time that may or may not be less than StopTime
    // All we know is that the last time it sent data to the EVB, the data had a tstamp
    // less than StopTime.

    // If we found a time that is less than StopTime send it to the EVB
    int channellength = 0;
    if (currenttime < StopTime) {
        uint32_t *r = reinterpret_cast<uint32_t*>(rBuffer);

        // if a deque has been selected, send its data out
        if(ModuleDequeLowTime != -1){
            if(!(ModuleDeque[ModuleDequeLowTime].size()>0)) {
                cout << "selected module " << ModuleDequeLowTime << " has no data " << endl;
                exit(1);
            }

            //make sure time is monotonically increaseing
            if(! (ModuleDeque[ModuleDequeLowTime][0]->time >= globaltime)){
                std::cout << "Time is not increasing..." 
                    //                cout << "Time is not increasing..." 
                    << "\n\tstoptime=" << StopTime 
                    << "\n\tglbltime=" << globaltime 
                    << "\n\tquelotime=" << ModuleDeque[ModuleDequeLowTime][0]->time 
                    << "\n\tmodindex=" <<  ModuleDequeLowTime << endl;
                exit(1);
            }

            std::vector<uint32_t>& data = ModuleDeque[ModuleDequeLowTime][0]->data;

            //update global time
            globaltime = ModuleDeque[ModuleDequeLowTime][0]->time;


            /* Consider each channel as a microevent and send for processing. 
               SpecTcl will reconstruct a complete event based on timestamps */
            channellength = ModuleDeque[ModuleDequeLowTime][0]->channellength;// (ModuleDeque[ModuleDequeLowTime][0] & CHANNELLENGTHMASK)>>17; 
            if(channellength ==0) {

                cout << "bad channel length at copyin " << channellength<< endl;

                exit (1);
            }


            uint64_t externalTstamp = extractExternalTimestamp(data);
            setTimestamp(externalTstamp*m_tstampScaleFactor);
            
            // copy data into output buffer
            *r++ = ModuleRevBitMSPSWord[ModuleDequeLowTime];
            r = std::copy(data.begin(), data.end(), r);

            // adjust the work counts
            nFIFOWordsinModuleCurrentRead[ModuleDequeLowTime] -= data.size();

            // we are now done with the chennel we just outputted. Get rid of it.
            delete ModuleDeque[ModuleDequeLowTime].front();//[0];
            ModuleDeque[ModuleDequeLowTime].pop_front();

            fragcount++;

        } // end if (ModuleDequeueLowTime != -1) block


        // Process a complete readout at once
        // Remove information after eventlist is completely processed
        if(/*ChannelList_it == ChannelList.end()*/nodata){
            mytrigger->Reset();
            m_processing = false;
        }

    }  // end if (currenttime < StopTime) block
    else {
        // if we made it here, currenttime > StopTime so we reject this
        // and reset to ensure that we read next
        m_processing = false;
        channellength=0;
        reject();
        mytrigger->Reset();
    }


    channellength += 1; // account for the module id 

    // number of 16-bit words ?
    //cout << "chan " << channellength << endl;
    if(channellength *2 > maxwords) { 
        cout << "channel event too big for readout " << endl;
    }

    if(channellength == 1){
        reject();
    }

    return channellength*2;

    // SNL new reaout
    //return rBuffer;

}

uint64_t CMyEventSegment::extractExternalTimestamp(const std::vector<uint32_t>& data)
{
  uint64_t tstamp = 0;

  // if there is no data, don't try to access it
  if (data.size() == 0) return tstamp;

  // extract the header length
  int headerLength = ((data[0] & 0x1f000) >> 12); 
  if (data.size() < headerLength) { 
    std::string msg("Data missing in event! Header length is greater than data length");
    throw std::runtime_error(msg);
  }

  // XIA makes life difficult for us because it doesn't label any of the data words.
  // We therefore have to compute the location of the external timestamp. It always falls
  // at the very end of the header (right before the trace), but that location depends
  // on the presence of the QDC or energy sum / baseline settings. We know that the timestamp
  // adds an extra 2 32-bit words to each permutation and the header lengths 
  // produced by each permutation is 4, 8, 12, and 16. We need to thus handle only when
  // the length is 6, 10, 14, and 18.
  if (headerLength == 6 || headerLength == 10 
      ||  headerLength == 14 || headerLength == 18) {

    tstamp = (0xffff & data[headerLength-1]);
    tstamp = ((tstamp<<32) | data[headerLength-2]);
  }
  
  return tstamp;
}

void CMyEventSegment::setTimestampScaleFactor(double factor)
{ m_tstampScaleFactor = factor; }

double CMyEventSegment::getTimestampScaleFactor() const
{ return m_tstampScaleFactor; }


int CMyEventSegment::GetCrateID() const
{
    return m_config.getCrateId();
}
