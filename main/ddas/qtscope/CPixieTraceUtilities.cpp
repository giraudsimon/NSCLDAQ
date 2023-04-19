/**
 * @file  CPixieTraceUtilities.cpp
 * @breif Implement the trace manager class.
 */

#include "CPixieTraceUtilities.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#if XIAAPI_VERSION >= 3
// XIA API version 3+
#include <cstddef>
#include <pixie16/pixie16.h>
#ifndef NEW_RUN
#define NEW_RUN 1
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
 * @breif Constructor.
 */
CPixieTraceUtilities::CPixieTraceUtilities() :
  m_pGenerator(new CDataGenerator),
  m_useGenerator(false),
  m_trace(MAX_ADC_TRACE_LEN, 0)
{}

/**
 * @brief Destructor.
 */
CPixieTraceUtilities::~CPixieTraceUtilities()
{
  delete m_pGenerator;
}

/**
 * @brief Read a validated ADC trace from single channel.
 *    
 * @note The trace is not triggered by the fast filter settings! 
 *
 * @param module   Module number.
 * @param channel  Channel number on module for trace read.
 *
 * @return int
 * @retval  0  Success.
 * @retval -1  XIA API call fails.
 * @retval -2  Acquired trace is empty (median undefined).
 */
int
CPixieTraceUtilities::ReadTrace(int module, int channel)
{
  int maxAttempts = 100;  // Reacquire attempts.
  bool goodTrace = false; // Trace meets validation requirements.

  int attempt = 0;  
  while ((goodTrace == false) && (attempt < maxAttempts)) {
    try {
      AcquireADCTrace(module, channel);
    
      // Check for good trace (signal likely present) and validate.
      // Median is more robust measure of baseline than mean for signals with
      // long decay time e.g. HPGe detectors, Si.
      
      double median = GetTraceMedian(m_trace);
      std::vector<double> traceMAD; // To hold the median avg. dev. values.
      for (const auto &ele : m_trace) {
	traceMAD.push_back(std::abs(ele-median));
      }
      double mad = GetTraceMedian(traceMAD); // Median avg. deviation.
      double sigma = 1.4826 * mad; // Estimate of std. dev.

      // iterators
      auto max = std::max_element(m_trace.begin(), m_trace.end());
      auto min = std::min_element(m_trace.begin(), m_trace.end());
    
      // 10 standard deviations ought to do it for a good signal. Check
      // negative as well in case the signal polarity is wrong.
      
      if ((*max > median + 10.0*sigma) ||
	  (*min < median - 10.0*sigma)) {
	if ((*max - median) > 20) { // Some (small) minimum amplitude.
	  goodTrace = true;
	}
      }
      
      // Try again:
      
      attempt++;
    }
    catch (std::runtime_error& e) {
      std::cerr << e.what() << std::endl;      
      return -1;
    }
    catch (std::invalid_argument& e) {
      std::cerr << e.what() << std::endl;      
      return -2;
    }
  }
  
  return 0;
}

/**
 * @brief Read a validated ADC trace from single channel.
 *  
 * @note The trace is not triggered by the fast filter settings! 
 *
 * @param module   Module number.
 * @param channel  Channel number on module for trace read.
 *
 * @return int
 * @retval  0  Success.
 * @retval -1  XIA API call fails.
 */
int
CPixieTraceUtilities::ReadFastTrace(int module, int channel)
{
  try {
    AcquireADCTrace(module, channel);
  }
  catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;    
    return -1; 
  }
  
  return 0;
}

/**
 * @brief Call to Pixie-16 API to acquire an ADC trace from a single channel.
 *
 * @param module   Module number.
 * @param channel  Channel number on module for trace read.
 *
 * @throws std::runtime_error  If ADC traces cannot be acquired (internal DSP 
 *                             memory fails to fill).
 * @throws std::runtime_error  If trace read fails.
 */
void
CPixieTraceUtilities::AcquireADCTrace(int module, int channel)
{
  std::fill(m_trace.begin(), m_trace.end(), 0); // Reset trace.
  
  // Fill internal DSP memory prior to trace read:
  
  int retval = Pixie16AcquireADCTrace(module);
  
  if (retval < 0) {
    std::stringstream errmsg;
    errmsg << "CPixieTraceUtilities::AcquireADCTrace() failed to allocate memory for trace in module " << module << " with retval " << retval;
    throw std::runtime_error(errmsg.str());
  }
 
  // Traces are in memory and can be read out, or read generator data:
  
  try {
    if (!m_useGenerator) {
      retval = Pixie16ReadSglChanADCTrace(m_trace.data(), MAX_ADC_TRACE_LEN, module, channel);
    
      if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieTraceUtilities::AcquireADCTrace() failed to read trace from module " << module << " channel " << channel << " with retval " << retval;	
	throw std::runtime_error(errmsg.str());
      }
      
    } else {
      
      // Get the trace binning and if successful generate a pulse:
      
      const char* pXDT = "XDT";
      double xdt = 0;
      retval = Pixie16ReadSglChanPar(pXDT, &xdt, module, channel);

      if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieTraceUtilities::AcquireADCTrace() failed to read parameter "
	       << pXDT << " from module " << module << " channel " << channel
	       << " with retval " << retval;
	throw std::runtime_error(errmsg.str());
      }
    
      retval = m_pGenerator->GetTraceData(m_trace.data(),
					  MAX_ADC_TRACE_LEN,
					  xdt);

      if (retval < 0) {
	std::stringstream errmsg;
	errmsg << "CPixieTraceUtilities::AcquireADCTrace() failed to read trace from module " << module << " channel " << channel << " with retval " << retval;
	throw std::runtime_error(errmsg.str());
      }
      
    }
  }
  catch (std::runtime_error& e) {
    throw e;
  }
}

/**
 * @brief Calculate the median value from a trace.
 *
 * @param trace  Input trace vector of type T.
 *
 * @return double  Median value of the trace.
 *
 * @throws std::invalid_argument  If trace is empty (median is undefined).
 */
template<typename T> double
CPixieTraceUtilities::GetTraceMedian(std::vector<T> trace)
{  
  if (trace.empty()) {
    std::stringstream errmsg;
    errmsg << "CPixieTraceUtilities::GetTraceMedian() failed to calculate the median value: the trace is empty and the median is undefined";
    throw std::invalid_argument(errmsg.str());
  }
  
  const auto midItr = trace.begin() + trace.size()/2;
  std::nth_element(trace.begin(), midItr, trace.end());
  
  if ((trace.size() % 2) == 0) { // Even number of samples (default 8192).
    const auto leftItr = std::max_element(trace.begin(), midItr);
    return 0.5*(*leftItr + *midItr);
  } else { // Odd number of samples, just in case someone changes it.
    return (double)(*midItr);
  }
}
