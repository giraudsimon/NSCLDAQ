/**
 * @file CPixieSystemUtilities.h
 * @brief Define a class for managing the state of Pixie DAQ systems.
 */

#ifndef CPIXIESYSTEMUTILITIES_H
#define CPIXIESYSTEMUTILITIES_H

#include <vector>

#include <Configuration.h>

/**
 * @class CPixieSystemUtilities
 * @brief Pixie DAQ system manager class.
 *
 * This class manages the Pixie DAQ system. It controls loading and saving 
 * settings files, booting and exiting, and stores information about the state
 * of the system which can be accessed across the ctypes interface.
 */

class CPixieSystemUtilities
{
public:
  CPixieSystemUtilities();
  ~CPixieSystemUtilities();

  int Boot();
  int SaveSetFile(char* fileName);
  int LoadSetFile(char* fileName);
  int ExitSystem();

  /**
   * @brief Set the boot mode.
   * @param mode  Value to set the boot mode to.
   */
  void SetBootMode(int mode) {m_bootMode = mode;};
  /**
   * @brief Set the boot mode.
   * @param mode  Value to set the boot mode to.
   */
  int GetBootMode() {return m_bootMode;};
  /**
   * @brief Get the crate boot status.
   * @return bool  True if system is booted, false otherwise.
   */
  bool GetBootStatus() {return m_booted;};
    /**
   * @brief Get the number of installed modules.
   * @return int  The number of modules in the crate.
   */
  int GetNumModules() {return m_numModules;};
  unsigned short GetModuleMSPS(int module);
    
private:
  DAQ::DDAS::Configuration m_config; //!< Hardware configuration information.
  int m_bootMode; //<! Offline (1) or online (0) boot mode.
  bool m_booted; //<! True when the system is booted, false otherwise.
  bool m_ovrSetFile;  //!< True if a settings file has been re-loaded since boot.
  unsigned short m_numModules; //!< Number of modules in the crate.
  std::vector<int> m_modEvtLength;           //!< event length in 32 bit words.
  std::vector<unsigned short> m_modADCMSPS;  //!< sampling rate of a module.
  std::vector<unsigned short> m_modADCBits;  //!< adc bits of a module.
  std::vector<unsigned short> m_modRev;      //!< module revision in hex format.
  std::vector<unsigned short> m_modClockCal; //!< ns per clock tick.
};

extern "C" {
  CPixieSystemUtilities* CPixieSystemUtilities_new() {return new CPixieSystemUtilities();};

  int CPixieSystemUtilities_Boot(CPixieSystemUtilities* utils) {return utils->Boot();};
  int CPixieSystemUtilities_SaveSetFile(CPixieSystemUtilities* utils, char* fName) {return utils->SaveSetFile(fName);};
  int CPixieSystemUtilities_LoadSetFile(CPixieSystemUtilities* utils, char* fName) {return utils->LoadSetFile(fName);};
  int CPixieSystemUtilities_ExitSystem(CPixieSystemUtilities* utils) {return utils->ExitSystem();};

  void CPixieSystemUtilities_SetBootMode(CPixieSystemUtilities* utils, int mode) {return utils->SetBootMode(mode);};
  int CPixieSystemUtilities_GetBootMode(CPixieSystemUtilities* utils) {return utils->GetBootMode();};
  bool CPixieSystemUtilities_GetBootStatus(CPixieSystemUtilities* utils) {return utils->GetBootStatus();};
  unsigned short CPixieSystemUtilities_GetNumModules(CPixieSystemUtilities* utils) {return utils->GetNumModules();};
  unsigned short CPixieSystemUtilities_GetModuleMSPS(CPixieSystemUtilities* utils, int mod) {return utils->GetModuleMSPS(mod);};
  
  void CPixieSystemUtilities_delete(CPixieSystemUtilities* utils)
  {
    if(utils) {
      delete utils;
      utils = nullptr;
    }
  };
}

#endif
