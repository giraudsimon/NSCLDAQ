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

/** @file:  CrateManager.cpp
 *  @brief: Implement the CrateManager class.
 */
#include "CrateManager.h"
#include <Configuration.h>
#include <HardwareRegistry.h>
#include <FirmwareVersionFileParser.h>
#include <config.h>
#include <config_pixie16api.h>

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

namespace DDAS {
    /**
     * constructor
     *   Our job is to create a configuration and
     *   read the hardware registry into the configuration
     *   by parsing the firmware definition file.
     *   We also initialize the Pixie16's with the slot map
     *   provided.
     *
     *   @param slots  - the slots we intend to use. Note that
     *                   each slot is assigned an id that is its index
     *                   into the slots array.
     *                   The moduleID and slot can translate between
     *                   the two.
     */
    CrateManager::CrateManager(const std::vector<unsigned short>& slots) :
        m_slots(slots), m_pConfiguration(nullptr)
    {
        // The thing most likely to fail is the system init
        // so do that first:
        
        initApi(m_slots.size(), m_slots.data());
        
        m_pConfiguration = new DAQ::DDAS::Configuration;
        
        // The try catch block below allows us to
        // free the configuration and exit the Pixie16 system.
        
        try {
            // Initialize the configuration by providing
            // it the firmware definition file.
            
            DAQ::DDAS::FirmwareVersionFileParser fwParser;
            std::ifstream fwfile(FIRMWARE_DEF_FILE);
            if (!fwfile) {
                std::stringstream msg;
                msg << "Unable to open firmware definition file: "
                    << FIRMWARE_DEF_FILE;
                throw std::invalid_argument(msg.str());
            }
            fwParser.parse(fwfile, m_pConfiguration->getDefaultFirmwareMap());
            fwfile.close();
        }
        catch (...) {
            delete m_pConfiguration;
            Pixie16ExitSystem(m_slots.size());
            throw;            // Let the callers deal with this.
        }
    }
    /**
     * destructor
     *    Exit the system (ignore errors) delete the configuration.
     */
    CrateManager::~CrateManager()
    {
        delete m_pConfiguration;

        Pixie16ExitSystem(m_slots.size());       // Closes all modules.
    }
    /**
     * moduleId
     *    Find the module id associated with a slot number.
     * @param s - the slot.
     * @return unsigned short - the module id.
     * @throw std::invalid_argument if there's no such slot.
     */
    unsigned short
    CrateManager::moduleId(unsigned short slot)
    {
        auto p = std::find(m_slots.begin(), m_slots.end(), slot);
        if (p == m_slots.end()) {
            std::stringstream msg;
            msg << "No such slot: " << slot
                << " while translating slot to a module id";
            throw std::invalid_argument(msg.str());
        }
        return std::distance(m_slots.begin(), p);
    }
    /**
     * slot
     *    Translate a module id into a slot.  This is just a
     *    protected lookup
     * @param id - module id.
     * @return unsigned short  - corresponding slot.
     */
    unsigned short
    CrateManager::slot(unsigned short id)
    {
        if (size_t(id) >= m_slots.size()) {
            std::stringstream msg;
            msg << "No such module id: " << id
                << " while translating a module id to a crate slot";
            throw std::invalid_argument(msg.str());
        }
        return m_slots[id];
    }
    /**
     * loadDSPAddressMap
     *    Does a null boot which has the effect of
     *    loading the DSP address map for the selected
     *    module.
     * @param id - mdule id.
     */
    void
    CrateManager::loadDSPAddressMap(unsigned short id)
    {
        auto s = slot(id);           // Early detect for valid id.
        DAQ::DDAS::FirmwareConfiguration files;
        getConfiguration(files, id);
        
        bootModule(
            id, 0,                     // Null boot.
             files.s_ComFPGAConfigFile.c_str(),
             files.s_SPFPGAConfigFile.c_str(),
             files.s_DSPCodeFile.c_str(),
             files.s_DSPVarFile.c_str()
        );
    }
    /**
     * fullBoot
     *   Performs a full boot of the selected board.
     *   This is done by setting getting the boot files
     *   and doing a mask of 0x6d
     *   Which does everything but load the DSP parameters
     *   The idea is that we'll load the DSP parameters later
     *   using one of our XML based loaders when we do this.
     *   Not we have an overload that supports old style SEt
     *  file parameter loading.
     *
     *  @param id  - board id.
     */
    void
    CrateManager::fullBoot(unsigned short id)
    {
        auto s= slot(id);           // throws if the id is bad.
        DAQ::DDAS::FirmwareConfiguration files;
        getConfiguration(files, id);

        bootModule(
            id, 0x6d,
            files.s_ComFPGAConfigFile.c_str(),
            files.s_SPFPGAConfigFile.c_str(),
            files.s_DSPCodeFile.c_str(),
            files.s_DSPVarFile.c_str()
        );
    }
    
    /**
     *  fullBoot
     *    Overload for booting and loading DSP variables
     *    from old fashioned setfile:
     *
     *  @param id  - module ID.
     *  @param setFile - The DSP variable settings file.
     */
    void
    CrateManager::fullBoot(unsigned short id, const char* setFile)
    {
        auto s= slot(id);           // throws if the id is bad.
        DAQ::DDAS::FirmwareConfiguration files;
        getConfiguration(files, id);

        bootModule(
            id, 0x7d,
            files.s_ComFPGAConfigFile.c_str(),
            files.s_SPFPGAConfigFile.c_str(),
            files.s_DSPCodeFile.c_str(),
            files.s_DSPVarFile.c_str(),
            setFile
        );
    }
    /**
     * (static) getVarFile
     *    Gets our best guess at the varfile for a specific
     *    digitizer speed.
     *
     *  @param speed MHz of the digitizer.
     */
    std::string
    CrateManager::getVarFile(unsigned short speed)
    {
        // Need to make a configuration sigh
        
        // Initialize the configuration by providing
            // it the firmware definition file.
            
        DAQ::DDAS::FirmwareVersionFileParser fwParser;
        DAQ::DDAS::Configuration             config;
        std::ifstream fwfile(FIRMWARE_DEF_FILE);
        if (!fwfile) {
            std::stringstream msg;
            msg << "Unable to open firmware definition file: "
                << FIRMWARE_DEF_FILE;
            throw std::invalid_argument(msg.str());
        }
        fwParser.parse(fwfile, config.getDefaultFirmwareMap());
        fwfile.close();
            
        // Use F revision except for 100Mhz in which case we use
        // D
        // Use 14 bits for all but 100MHz which is 12.
        int rev = 15;     // (F).
        int bits =14;
        if (speed == 100) {
            rev = 13;     // (D).
            bits= 12;
        }
        unsigned hwtype = 
            DAQ::DDAS::HardwareRegistry::computeHardwareType(
                rev, speed, bits
            );
        DAQ::DDAS::FirmwareConfiguration files =
            config.getFirmwareConfiguration(hwtype);
        return files.s_DSPVarFile;
    }
    ///////////////////////////////////////////////////////////
    // Private Utilities.

    /**
     *  initApi
     *    Does a Pixie16InitSystem analyzing errors and
     *    turning them into an exception throw.
     * @param nSLots - number of slots with modules.
     * @param slots  - Slots that are occupied with modules.
     */
    void
    CrateManager::initApi(unsigned nSlots, unsigned short* slots)
    {
        int stat = Pixie16InitSystem(nSlots, slots, 0);
        if (stat < 0) {
            std::stringstream msg;
            switch (-stat) {
            case 1:
                msg << " Invalid total number of Pixie16 modules: "
                    << nSlots;
                break;
            case 2:
                msg << "Null pointer to slot map (can't happen)";
                break;
            case 3:
                msg << "System initialization failed, "
                    << "check Pixie16msg.txt log file";
                break;
            default:
                msg << "Unexpected error code from Pixie16InitSystem: " 
                    << stat;
                break;
            }
            throw std::invalid_argument(msg.str());
        }
    }
    /**
     *  bootModule
     *     Boot a module given the needed files and the
     *     boot mask.  Exhaustive error analysis is done
     *     and errors are turned into exception throws.
     *
     * @param modId    - module id.
     * @param mask     - Mask of boot options.
     * @param ComFirmware - Path to communications fpga firmware.
     * @param SFPGAFirmware - path to the signal processing FPGA firmware.
     * @param DSPFirmware   - Path to the DSP firmware file.
     * @param DSPVarMap     - Path to the DSP Variables map.
     * @param SetFile  - Defaults to nullptr - this is an optional
     *                   parameter to support old style loading
     *                   of DSP Parameters using a set file.
     */
    void CrateManager::bootModule(
        unsigned short modId, unsigned short mask,
        const char* ComFirmware,
        const char* SFPGAFirmware,
        const char* DSPFirmware,
        const char* DSPVarMap,
        const char* SetFile
    ) 
    {
        int stat = Pixie16BootModule(
            ComFirmware, SFPGAFirmware, nullptr, DSPFirmware,
            SetFile, DSPVarMap,
            modId, mask
        );
        if (stat < 0) {
            std::stringstream msg;
            switch(-stat) {
            case 1:
                msg << "Invalid module id at boot: " << modId;
                break;
            case 2:
                msg << "Communications FPGA file is bad: " 
                    << ComFirmware;
                break;
            case 3:
                msg << "Communication FPGA boot failed. "
                    << "See Pixie16msg.txt logfile";
                break;
            case 4:
                msg << "Memory allocation for contents of "
                    << "comm. fpga file failed "
                    << ComFirmware;
                break;
            case 5:
                msg << "Unable to open Comm. fpga file: "
                    << ComFirmware;
                break;
            case 10:
                msg << "Signal processing FPGA file is bad: "
                    << SFPGAFirmware;
                break;
            case 11:
                msg << "Signal processing FPGA boot failed. " 
                    << " See Pixie16msg.txt logfile ";
                break;
            case 12:
                msg << "Memory allocation for contents of "
                    << "SFGPA file failed";
                break;
            case 13:
                msg << "Unable to open signal FPGA firmware: "
                    << SFPGAFirmware;
                break;
            case 14:
                msg << "DSP Boot failed";
                break;
            case 15:
                msg << "Failed to allocate memory for DSP executable "
                    << " from " << DSPFirmware;
                break;
            case 16:
                msg << "Failed to open DSP executable: " << DSPFirmware;
                break;
            case 17:
                msg << "DSP Firmware file size invalid: " << DSPFirmware;
                break;
            case 18:
                msg << "Failed to open the DSP Var file" << DSPVarMap;
                break;
            case 19:
                msg << "Can't initialie the DSP var map " << DSPVarMap;
                break;
            case 20:
                msg << "Can't copy DSP var map indices: " << DSPVarMap
                    << " Check Pixie16msg.txt logfile";
                break;
            case 21:
                msg << "Failed to program FPPI check Pixie16msg.txt logfile";
                break;
            case 22:
                msg << "Failed to set module DACs check Pixie16msg.txt logfile";
                break;
            case 23:
                msg << "Failed to reset ADC run in a module "
                    << "check Pixie16msg.txt logfile";
                break;
            case 24:
                msg << "RESET_ADC run timed out in a module "
                    << "checkm Pixie16msg.txt log file";
                break;
            default:
                msg << "Pixie16BootModule threw an unexpected error "
                    << stat << " when booting module " << modId;
                break;

            

            }
            throw std::invalid_argument(msg.str());
        }
    }
    /**
     *  getModuleInfo
     *    Wraps the Pixie16ReadModuleInfo call in error checking
     *    that maps errors into exceptions with reasonable
     *    text.
     * @param modId   - Id of module to read.
     * @param[out] rev - Pointer to where to put the module revision.
     * @param[out] serial - Pointer to where to put the module serial#
     * @param[out] bits- Pointer to where to put the # of bits.
     * @param[out] MHz - Pointer to where to put the FADC speed.
     */
    void
    CrateManager::getModuleInfo(
        unsigned short modId, unsigned short* rev,
        unsigned int* serial, unsigned short* bits,
        unsigned short* MHz
    )
    {
        int stat = Pixie16ReadModuleInfo(
            modId, rev, serial, bits, MHz
        );
        if (stat < 0) {
            std::stringstream msg;
            switch (-stat)
            {
            case 1:
                msg << "getModuleInfo - invalid module id: " 
                    << modId;
                break;
            case 2:
                msg << "Pixie16ReadModuleInfo - could ot read from "
                    << " module: " << modId << "'s i2c serial EEPROM "
                    << " Check Pixie16msg.txt log file.";
                break;
            default:
                msg << "Pixe16ReadModuleInfo - unxepcted status value: "
                    << stat << " for module id: " << modId;
                break;
            }
            throw std::invalid_argument(msg.str());
        }
    }
    /**
     * getConfiguration
     *    Returns the configuration files associated
     *    with a module.  These will be stored in
     *   a DAQ::DDAS::FirmwareConfiguration struct.
     * 
     * @param[out] files - results get put here.
     * @param      id    - Module Id to ask.
     */
    void
    CrateManager::getConfiguration(
        DAQ::DDAS::FirmwareConfiguration& files,
        unsigned short id
    )
    {
        unsigned short rev;
        unsigned int   serial;
        unsigned short bits;
        unsigned short MHz;

        getModuleInfo(id, &rev, &serial, &bits, &MHz);
        
        int hwType =
            DAQ::DDAS::HardwareRegistry::computeHardwareType(
                rev, MHz, bits
            );
        files = m_pConfiguration->getFirmwareConfiguration(hwType);

    }
    
}                             // DDAS Namespace.
