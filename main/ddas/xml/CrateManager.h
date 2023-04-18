/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CrateManager
 *  @brief: Manages configuring and booting a crate.
 */
#ifndef CRATEMANAGER_H
#define CRATEMANAGER_H
#include <vector>
#include <string>
namespace DAQ {
namespace DDAS {
      class Configuration;
      struct FirmwareConfiguration;
}}
namespace DDAS {
    /**
     * @class CrateManager
     *     This class is responsible for
     *     - Obtaining crate configuration.
     *     - Initializing the Pixie16 API
     *     - Booting the modules in the crate.
     *  @note We assume the compiler definition FIRMWARE_DEF_FILE is
     *       a CString path to the firmware directory file.
     */
    class CrateManager {
        DAQ::DDAS::Configuration*    m_pConfiguration;
        std::vector<unsigned short>  m_slots;
    public:
        CrateManager(const std::vector<unsigned short>& slots);
        virtual ~CrateManager();
        //
        // Illegal canonical methods.  We could write them but its' not
        // really worth it and there are no use cases.
        //
    private:
        CrateManager(const CrateManager&);
        CrateManager& operator=(const CrateManager& );
        int operator==(const CrateManager&) const;
        int operator!=(const CrateManager&) const;
    public:
        // Slot translation
        
        unsigned short moduleId(unsigned short slot);
        unsigned short  slot(unsigned short id);
        
        // Configuration:
        
        void loadDSPAddressMap(unsigned short id);
        void fullBoot(unsigned short id);
        void fullBoot(unsigned short id, const char* setFile);
        static std::string getVarFile(unsigned short speed);
    private:
        void initApi(unsigned nSlots, unsigned short* slots);
        void bootModule(
            unsigned short modId, unsigned short mask,
            const char* ComFirmware,
            const char* SPFPGAFirmware,
            const char* DSPFirmware,
            const char* DSPVarMap,
            const char* SetFile = nullptr
        );
        void getModuleInfo(
            unsigned short modId, unsigned short* rev,
            unsigned int* serial, unsigned short* bits,
            unsigned short* MHz
        );
        void getConfiguration(
            DAQ::DDAS::FirmwareConfiguration& files, 
            unsigned short id
        );
        
    };
    
    
}                                   // DDAS Namespace


#endif