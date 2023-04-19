/**
 * @file  CPixieTraceUtilities.h
 * @breif Defines a class for trace management and an interface accessable by 
 * Python ctypes
 */

#ifndef CPIXIETRACEUTILITIES_H
#define CPIXIETRACEUTILITIES_H

#include <vector>

class CDataGenerator;

/**
 * @class CPixieTraceUtilities
 * @brief A trace manager to read trace data from Pixie modules and marshall 
 * the trace data to Python.

 * This class provides a ctypes-friendly interfaace which can be called from 
 * Python. Anything callable must be extern "C" to avoid name mangling by the
 * C++ compiler.
 */

class CPixieTraceUtilities
{
public:
  CPixieTraceUtilities();
  ~CPixieTraceUtilities();

  int ReadTrace(int module, int channel);
  int ReadFastTrace(int module, int channel);

  /**
   * @brief Return the trace data.
   * @return unsigned short*  Pointer to the underlying trace storage.
   */
  unsigned short* GetTraceData() {return m_trace.data();};
  /**
   * @brief Set flag for offline running using the data generator.
   * @param mode  Generator flag is set to input value.
   */
  void SetUseGenerator(bool mode) {m_useGenerator = mode;};
  
private:
  CDataGenerator* m_pGenerator; //!< The offline data generator.
  bool m_useGenerator; //!< True if using generated data, else online data.
  std::vector<unsigned short> m_trace; //!< Single channel trace data.

  void AcquireADCTrace(int module, int channel);
  template<typename T> double GetTraceMedian(std::vector<T> trace);
};
  
extern "C" {
  /** @brief Wrapper for the class constructor. */
  CPixieTraceUtilities* CPixieTraceUtilities_new() {return new CPixieTraceUtilities();};

  /** @brief Wrapper for reading a validated trace. */
  int CPixieTraceUtilities_ReadTrace(CPixieTraceUtilities* utils, int mod, int chan) {return utils->ReadTrace(mod, chan);};
  /** @brief Wrapper for reading an unvalidated trace. */
  int CPixieTraceUtilities_ReadFastTrace(CPixieTraceUtilities* utils, int mod, int chan) {return utils->ReadFastTrace(mod, chan);};
  /** @brief Wrapper to get trace data. */
  unsigned short* CPixieTraceUtilities_GetTraceData(CPixieTraceUtilities* utils) {return utils->GetTraceData();}
  /** @brief Wrapper to set generator use. */
  void CPixieTraceUtilities_SetUseGenerator(CPixieTraceUtilities* utils, bool mode) {return utils->SetUseGenerator(mode);};

  /** @brief Wrapper for the class destructor. */
  void CPixieTraceUtilities_delete(CPixieTraceUtilities* utils)
  {
    if(utils) {
      delete utils;
      utils = nullptr;
    }
  };
}

#endif
