/*********************************************************
 Scaler class for DDAS
 Access statistics directly from the Pixie 16 modules
 H.L. Crawford 6/13/2010
*********************************************************/

#include <config.h>
#include <config_pixie16api.h>
#include "CMyScaler.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string.h>


extern unsigned int ChanEventsB_Address[];
extern unsigned int ChanEventsA_Address[];
extern unsigned int FastPeaksA_Address[];
extern unsigned int FastPeaksB_Address[];


/* Constructor */
CMyScaler::CMyScaler(unsigned short moduleNr, unsigned short crateid) 
{
  moduleNumber = moduleNr;
  for (int i=0; i<16; i++) {
    PreviousCounts[i] = 0;
    PreviousCountsLive[i] = 0;
  }

  crateID = crateid;
  clearCounters(m_Statistics.s_cumulative);
  clearCounters(m_Statistics.s_perRun);

  cout << "Scalers know crate ID = " << crateID << endl;
}

CMyScaler::~CMyScaler()
{ 
  // Destructing here
}

void CMyScaler::initialize() 
{

  for (int i=0; i<16; i++) {
    PreviousCounts[i] = 0;
    PreviousCountsLive[i] = 0;
  }
  clearCounters(m_Statistics.s_perRun);    // New run.
}

void CMyScaler::disable(){
  // modules do not need scalers disabled at end of run
}

vector<uint32_t> CMyScaler::read()
{
  try{

    int retval;
#if XIAAPI_VERSION >= 3
    std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(),0);
#else
    std::vector<unsigned int> statistics(448,0); // see any v11.3
#endif
    retval = Pixie16ReadStatisticsFromModule(statistics.data(), moduleNumber);
    if (retval < 0) {
      cout << "Error accessing scaler statistics from module " 
	   << moduleNumber << endl;
    }
    
    // // API 3+ this is much easier...
    // double Counts[16] = {0};
    // double CountsLive[16] = {0};
    // unsigned long OutputScalerData[33] = {0};  
    // OutputScalerData[0] = (unsigned long)crateID;
    // for (int i=0; i<16; i++) {
    //   // Raw input counts (number of fast triggers seen by FPGA)
    //   Counts[i] = Pixie16ComputeRawInputCount(statistics.data(), moduleNumber, i);
    //   // Raw output counts (validated events handled by DSP, "live" counts)
    //   CountsLive[i] = Pixie16ComputeRawOutputCount(statistics.data(), moduleNumber, i);    
    //   /* Finally compute the events since the last scaler read! */
    //   OutputScalerData[(2*i + 1)] = (unsigned long) (Counts[i] - PreviousCounts[i]);
    //   OutputScalerData[(2*i + 2)] = (unsigned long) (CountsLive[i] - PreviousCountsLive[i]);      
    //   PreviousCounts[i] = Counts[i];
    //   PreviousCountsLive[i] = CountsLive[i];
    // }

    /* Now we need to calculate the # of events from the last
       read of the scalers -- NSCL scaler buffers just expect
       the # events since the last read.  However, Pixie16 statistics
       cannot be cleared, so we need to do some math */
    double ICR[16] = {0};
    double OCR[16] = {0};
    double LiveTime[16] = {0};
    double RealTime = 0;
    double Counts[16] = {0};
    double CountsLive[16] = {0};
    double ChanEvents[16] = {0}; 
    double FastPeaks[16] = {0};
    unsigned long OutputScalerData[33] = {0};

    for (int i=0; i<16; i++) {
      /* Compute input count rate in counts/second for each channel */  
      ICR[i] = Pixie16ComputeInputCountRate (statistics.data(), moduleNumber, i);
      /* Compute output count rate in counts/second for each channel */
      OCR[i] = Pixie16ComputeOutputCountRate (statistics.data(), moduleNumber, i);
      /* Compute LiveTime for each channel */    
      LiveTime[i] = Pixie16ComputeLiveTime (statistics.data(), moduleNumber, i);

      ChanEvents[i] = Pixie16ComputeOutputCountRate(statistics.data(), moduleNumber, i);
      FastPeaks[i]  = Pixie16ComputeInputCountRate(statistics.data(), moduleNumber, i);

      /* Now compute total events for each channel, and total "live"
	 events for each channel. */
      Counts[i] = FastPeaks[i] * LiveTime[i];
      CountsLive[i] = ChanEvents[i] * RealTime;
    
      /* Finally compute the events since the last scaler read! */
      OutputScalerData[(2*i + 1)] = (unsigned long) (Counts[i] - 
						     PreviousCounts[i]);
      OutputScalerData[(2*i + 2)] = (unsigned long) (CountsLive[i] - 
						     PreviousCountsLive[i]);

      PreviousCounts[i] = Counts[i];
      PreviousCountsLive[i] = CountsLive[i];
    }
    
    /* Copy scaler information into the output vector */
    scalers.clear(); //SNL added for new readout
    scalers.insert(scalers.end(), OutputScalerData, OutputScalerData+33);
  
    //  Figure out the statistics by summing over the OutputScalerData (it's
    // incremental so we can just add it all in).
  
    int chb(1);
    for (int i =0; i < 16; i++) {
      m_Statistics.s_cumulative.s_nTriggers += OutputScalerData[chb];
      m_Statistics.s_perRun.s_nTriggers     += OutputScalerData[chb];
    
      m_Statistics.s_cumulative.s_nAcceptedTriggers += OutputScalerData[chb+1];
      m_Statistics.s_perRun.s_nAcceptedTriggers     += OutputScalerData[chb+1];
    
      chb += 2;
    }
  
    return scalers;

  }
  catch(...){
    cout << "exception in scaler " << endl;
    return scalers;
  }
  
}

void CMyScaler::clear() 
{

  // Clear? Don't think we need this with Pixie16 -- can't clear!

}

/**
 * clearCounters
 *    Clears statistics information.
 *
 *    @param c - reference to a counters struct.
 */
void CMyScaler::clearCounters(Counters& c)
{
  memset(&c, 0, sizeof(Counters));
}
