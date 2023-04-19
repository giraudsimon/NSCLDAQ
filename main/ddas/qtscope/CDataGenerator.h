/**
 * @file CDataGenerator.h
 * @brief Define a class for generating offline data for testing/debugging.
 */

#ifndef CDATAGENERATOR_H
#define CDATAGENERATOR_H

#include <random>

/**
 * @class CDataGenerator
 * @brief A class to generate test pulse, run, and baseline data for offline 
 * operation of QtScope.
 */

class CDataGenerator
{
public:
  CDataGenerator();
  ~CDataGenerator();

  int GetTraceData(unsigned short* data, int dataSize, double binWidth);
  int GetHistogramData(unsigned int* data, int dataSize);
  int GetBaselineData(double* data, int dataSize);
  
private:
  std::mt19937 m_engine; //!< Random number generator engine.
  
  unsigned short SinglePulse(double C, double A, double t0, double rise,
			     double decay, int sample, double binWidth);
};

#endif
