/**
 * @file CPixieDSPUtilities.h
 * @brief Define a class to read and write settings to XIA Pixie modules.
 */

#ifndef CPIXIEDSPUTILITIES_H
#define CPIXIEDSPUTILITIES_H

/**
 * @class CPixieDSPUtilities
 * @brief This class reads and writes both channel-level and module-level 
 * settings to XIA Pixie modules. The class also contains a function to adjust
 * the DC offsets on a single module, as the DC offset is itself a channel
 * parameter.
 */

class CPixieDSPUtilities
{
public:
  CPixieDSPUtilities();
  ~CPixieDSPUtilities();

  int AdjustOffsets(int module);
  int WriteChanPar(int module, int channel, char* paramName, double value);
  int ReadChanPar(int module, int channel, char* paramName, double& value);
  int WriteModPar(int module, char* paramName, unsigned int value);
  int ReadModPar(int module, char* paramName, unsigned int& value);
};

extern "C" {
  /** @brief Wrapper for the class constructor. */
  CPixieDSPUtilities* CPixieDSPUtilities_new() {return new CPixieDSPUtilities();};

  /** @brief Wrapper to adjust DC offsets. */
  int CPixieDSPUtilities_AdjustOffsets(CPixieDSPUtilities* utils, int mod) {return utils->AdjustOffsets(mod);};
  /**  @brief Wrapper to write a channel parameter. */
  int CPixieDSPUtilities_WriteChanPar(CPixieDSPUtilities* utils, int mod, int chan, char* pName, double val) {return utils->WriteChanPar(mod, chan, pName, val);};
  /**  @brief Wrapper to read a channel parameter. */
  int CPixieDSPUtilities_ReadChanPar(CPixieDSPUtilities* utils, int mod, int chan, char* pName, double& val) {return utils->ReadChanPar(mod, chan, pName, val);};
  /**  @brief Wrapper to write a module parameter. */
  int CPixieDSPUtilities_WriteModPar(CPixieDSPUtilities* utils, int mod, char* pName, unsigned int val) {return utils->WriteModPar(mod, pName, val);};
  /**  @brief Wrapper to read a module parameter. */
  int CPixieDSPUtilities_ReadModPar(CPixieDSPUtilities* utils, int mod, char* pName, unsigned int& val) {return utils->ReadModPar(mod, pName, val);};
  
  /**  @brief Wrapper for the class destructor. */
  void CPixieDSPUtilities_delete(CPixieDSPUtilities* utils)
  {
    if(utils) {
      delete utils;
      utils = nullptr;
    }
  };
}

#endif
