/**
 * @file CPixieRunUtilities.cpp
 * @brief Implement class for managing list-mode and baseline runs.
 */

#include "CPixieRunUtilities.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

#if XIAAPI_VERSION >= 3
// XIA API version 3+
#include <cstddef>
#include <pixie16/pixie16.h>
#ifndef NEW_RUN   // This stuff will come from the pixie_config.h 
#define NEW_RUN 1 // or whatever its called once its in actual DDAS?
#endif
#else
// XIA API version 2
#ifndef PLX_LINUX
#define PLX_LINUX
#endif
#include <pixie16app_common.h>
#include <pixie16app_defs.h>
#include <pixie16app_export.h>
#include <xia_common.h>
#endif

#include "CDataGenerator.h"

/**
 * @brief Constructor.
 */
CPixieRunUtilities::CPixieRunUtilities() :
  m_histogram(MAX_HISTOGRAM_LENGTH, 0),
  m_baseline(MAX_HISTOGRAM_LENGTH, 0),
  m_baselineHistograms(16, std::vector<unsigned int>(MAX_HISTOGRAM_LENGTH, 0)),
  m_runActive(false),
  m_useGenerator(false),
  m_pGenerator(new CDataGenerator)
{}

/**
 * @brief Destructor.
 */
CPixieRunUtilities::~CPixieRunUtilities()
{
  delete m_pGenerator;
}

/**
 * @brief Begin a histogram (MCA) run for a single module. Explicitly sets 
 * module synchronization to OFF.
 *
 * @param module  Module number.
 *
 * @return int  0 on success, XIA error code on failure.
 *
 * @todo Disable multiple modules from running in non-sync mode.
 */
int
CPixieRunUtilities::BeginHistogramRun(int module)
{
  // Reset internal histogram data:
  
  std::fill(m_histogram.begin(), m_histogram.end(), 0);
  
  // Set the "infinite" run time of 99999 seconds:
  
  std::string paramName = "HOST_RT_PRESET";
  int retval = Pixie16WriteSglModPar(paramName.c_str(), Decimal2IEEEFloating(99999), module);
  
  if (retval < 0) {
    std::cerr << "Run time not properly set. CPixieRunUtilities::BeginHistogramRun() failed to write parameter: " << paramName << " to module " << module  << " with retval " << retval << std::endl;
    
    return retval;
  }

  // If the run time is properly set, begin a histogram run for this module
  // turn off synchronization (0):
  
  paramName = "SYNCH_WAIT";
  retval = Pixie16WriteSglModPar(paramName.c_str(), 0, module);
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::BeginHistogramRun() failed to disable " << paramName << " in module " << module << " with retval " << retval << std::endl;
    
    return retval;    
  }

  // Begin the run:
  
  retval = Pixie16StartHistogramRun(module, NEW_RUN);
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::BeginHistogramRun() failed to start run module " << module << " with retval " << retval << std::endl;
  } else {
    std::cout << "Beginning histogram run in Mod. " << module << std::endl;
    m_runActive = true;
  }
  
  return retval;
}

/**
 * @brief End a histogram (MCA) run for a single module. Assumes module 
 * synchronization is OFF __but__ only stops a run in a single module.
 *
 * @param module  Module number.
 *
 * @return int  0 (even if run has not ended properly).
 *
 * @todo Control for active non-sync runs in multiple modules (or end all).
 */
int
CPixieRunUtilities::EndHistogramRun(int module)
{
  int retval = Pixie16EndRun(module);
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::EndHistogramRun() failed to communicate end run operation to module " << module << " with retval " << retval << std::endl;
  }

  bool runEnded = false;
  int nRetries = 0;
  const int maxRetries = 10;
  while ((runEnded == false) && (nRetries < maxRetries)) {
    retval = Pixie16CheckRunStatus(module);
    
    if (retval < 0) {
      std::cerr << "CPixieRunUtilities::EndHistogramRun() failed to get current run status in module " << module << " with retval " << retval << std::endl;
    }
    
    runEnded = (retval == 0); // True if run ended.
    nRetries++;
    
    // Wait before checking again:
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  if (nRetries == maxRetries) {
    std::cout << "CPixieRunUtilities::EndHistogramRun() failed to end run in module " << module << std::endl;
  } else if (runEnded) {
    std::cout << "Ended histogram run in Mod. " << module << std::endl;
    m_runActive = false;
  }
  
  return 0;
}

/**
 * @brief Read energy histogram from single channel.
 *
 * Histogram data will be saved to a file if debugging mode is enabled.
 *
 * @param module   Module number.
 * @param channel  Channel number on module to read histogram from.
 *
 * @return int  0 on success, XIA error code on failure.
 */
int
CPixieRunUtilities::ReadHistogram(int module, int channel)
{
  // Allocate data structure for histogram and grab it or use the generator:
  
  int retval = -1;
  
  if (!m_useGenerator) {
    retval = Pixie16ReadHistogramFromModule(m_histogram.data(), MAX_HISTOGRAM_LENGTH, module, channel);
  } else {
    retval = m_pGenerator->GetHistogramData(m_histogram.data(), MAX_HISTOGRAM_LENGTH);
  }
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::ReadHistogram() failed to read histogram from module " << module << " channel " << channel << " with retval " << retval << std::endl;
  }

  return retval;
}

/**
 * @brief Begin a baseline run.
 *
 * Baseline acquisition is not a "run" in the same sense that histogram runs
 * or list mode data taking is a "run" to the API (no begin/end functions, 
 * no run status change). However, in order for a user to accumulate enough 
 * baseline statistics to make judgements about e.g. manually setting 
 * baseline cuts, it needs to be treated as such in our manager.
 *
 * @return int  Always 0 (success).
 */
int
CPixieRunUtilities::BeginBaselineRun(int module)
{
  std::cout << "Beginning baseline run in Mod. " << module << std::endl;

  // Clear data vectors and set run active:
  
  for (auto& v : m_baselineHistograms) {
    std::fill(v.begin(), v.end(), 0);
  }
  
  std::fill(m_baseline.begin(), m_baseline.end(), 0);
  
  m_runActive = true;
  
  return 0;
}

/**
 * @brief "End" a baseline run.
 *
 * @param module  Module number.
 *
 * @return int  Always 0 (success).
 */
int
CPixieRunUtilities::EndBaselineRun(int module)
{
  m_runActive = false;
  std::cout << "Ended baseline run in Mod. " << module << std::endl;
    
  return 0;
}

/**
 * @brief Acquire baselines and read baseline data from a single channel.
 *
 * @param module  Module number.
 *
 * @return int 
 * @retval 0 on success.
 * @retval -1 if baseline memory cannot be allocated.
 * @retval -2 if updating the baseline histograms fails.
 */
int
CPixieRunUtilities::ReadBaseline(int module, int channel)
{  
  // Fill internal DSP memory prior to trace read:
  
  int retval = Pixie16AcquireBaselines(module);
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::ReadBaseline() failed to allocate memory for trace in module " << module << " with retval " << retval << std::endl;    
    return -1;
  }

  // Baseline data is an array of baseline values, not a histogram. To treat
  // this like a run, make cumulative histogram of read values:
  
  try {
    UpdateBaselineHistograms(module);
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;    
    return -2;
  }

  // The baseline we want (all other channels on the module are also updated):
  
  m_baseline = m_baselineHistograms[channel];
  
  return 0;
}

/**
 * @brief Read statistics for a single module after a run is ended.
 *
 * @param module  Module number.
 *
 * @return int  0 on success, XIA API error code on failure.
 *
 * @todo Confirm end of run and handle if not ended properly.
 */
int
CPixieRunUtilities::ReadModuleStats(int module)
{  
  // Where to read the statistics into, size depends on XIA API version:
  
#if XIAAPI_VERSION >= 3
  std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(),0);
#else
  std::vector<unsigned int> statistics(448,0); // see any v11.3.
#endif
  
  int retval = Pixie16ReadStatisticsFromModule(statistics.data(), module);
  
  if (retval < 0) {
    std::cerr << "CPixieRunUtilities::ReadModuleStats() error accessing scaler statistics " << "from module " << module << " with retval " << retval << std::endl;
    return retval;
  } else {
    double realTime = Pixie16ComputeRealTime(statistics.data(),module);
    for (int i = 0; i < 16; i++) {
      double inpRate = Pixie16ComputeInputCountRate(statistics.data(), module, i);
      double outRate = Pixie16ComputeOutputCountRate(statistics.data(), module, i);
      double liveTime = Pixie16ComputeLiveTime(statistics.data(), module, i);      
      std::cout << "Module " << module << " channel " << i << " input " << inpRate << " output " << outRate << " livetime " << liveTime << " runtime " << realTime << std::endl;
    }
  }
  
  return retval;
}

/**
 * @brief Update baseline histograms for all channels on a single module.
 *
 * @param module  Module number.
 *
 * @throw std::runtime_error  If the baseline read fails.
 */
void
CPixieRunUtilities::UpdateBaselineHistograms(int module)
{
  int retval = -1;
  
  for (int i = 0; i < 16; i++) {
    std::vector<double> baselines(MAX_NUM_BASELINES, 0);
    std::vector<double> timestamps(MAX_NUM_BASELINES, 0);
    
    // Allocate data structure for baselines and grab them or use the
    // data generator to get data for testing:
    
    if (!m_useGenerator) {
      retval = Pixie16ReadSglChanBaselines(baselines.data(), timestamps.data(), MAX_NUM_BASELINES, module, i);      
    } else {
      retval = m_pGenerator->GetBaselineData(baselines.data(), MAX_NUM_BASELINES);
    }
  
    if (retval < 0) {
      std::stringstream errmsg;
      errmsg << "CPixieRunUtilities::UpdateBaselineHistograms() failed to read baseline from module " << module << " channel " << i << " with retval " << retval;
      throw std::runtime_error(errmsg.str());
    }
    
    // If we have the baseline, update its histogram for valid values:
    
    for (const auto &ele : baselines) {
      int bin = static_cast<int>(ele);
      if (bin >= 0 && bin < MAX_HISTOGRAM_LENGTH) {
	m_baselineHistograms[i][bin]++;
      } 
    } 
    
  }  
}
