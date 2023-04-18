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

/** @file:  ModuleWriter.cpp
 *  @brief: Implement the module writer class.
 */
 
#include "ModuleWriter.h"
#include <config.h>
#include <config_pixie16api.h>

#include <stdexcept>
#include <sstream>
#include <fstream>

#include <HardwareRegistry.h>
#include <FirmwareVersionFileParser.h>
#include <Configuration.h>

namespace DDAS {
/**
 * constructor
 *   Saves the information about which module we're loading and some
 *   of the global parameters that are not in the module configuration
 *   because they are really experimental configuration items.
 *
 * @param crate - the crate id of the modules (which crate it's in).
 * @param slot  - slotId of the module (normally the slot its in).
 * @param module - moduleId of the module used to specify the module to the
 *                 API.
 */
ModuleWriter::ModuleWriter(unsigned crate, unsigned slot, unsigned module)
    : m_moduleId(module), m_crateId(crate), m_slotId(slot)
{
     
}
/**
 * destructor
 */
ModuleWriter::~ModuleWriter()
{
}
/**
 * write
 *    Write module settings to the crate.
 *    This assumes that the Pixie16 API has already been initialized
 *    and booted sufficiently to have read in the apropriate VAR
 *    files.
 *
 *    @param dspSettings the vector of module settings that describes
 *           what to load.
 */
void
ModuleWriter::write(const ModuleSettings& dspSettings)
{
    loadModule(m_moduleId, dspSettings);

}

///////////////////////////////////////////////////////////////////////
//  Private utility methods.
//

/**
 * loadModule
 *     Loads a module from its internal settings structure.
 *  @param id - module id.
 *  @param settings references the sttings to load.
 */
void
ModuleWriter::loadModule(unsigned id, const ModuleSettings& settings)
{
    loadModuleSettings(id, settings);
    for (int i = 0; i < 16; i++) {
        loadChannelSettings(id, i, settings);
    }
}
/**
 * loadModuleSettings
 *     Calls Pixie16WriteSglModPar repeatedly until all the module
 *     level settings for a module have been written.  Instead of calling
 *     @Pixie16WriteSglModPar directly,  we call
 *     writeModuleSetting which does that and error reporting.
 *
 * @param modid   - Id of the module.
 * @param settings - the ModuleSettings for the module we're loading.
 */
void
ModuleWriter::loadModuleSettings(
        unsigned modid, const ModuleSettings& settings
)
{
    writeModuleSetting(modid, "MODULE_CSRA",  settings.s_csra);
    writeModuleSetting(modid, "MODULE_CSRB",  settings.s_csrb);
    writeModuleSetting(modid, "MODULE_FORMAT", settings.s_format);
    writeModuleSetting(modid, "SYNCH_WAIT",    settings.s_synchWait? 1 : 0);
    writeModuleSetting(
        modid, "SLOW_FILTER_RANGE", settings.s_SlowFilterRange
    );
    writeModuleSetting(
        modid, "FAST_FILTER_RANGE", settings.s_FastFilterRange
    );
    writeModuleSetting(
        modid, "FastTrigBackplaneEna", settings.s_FastTrgBackPlaneEnables
    );
    writeModuleSetting(modid, "CrateID", m_crateId);
    writeModuleSetting(modid, "SlotID", m_slotId);
    writeModuleSetting(modid, "ModID", m_moduleId);
    writeModuleSetting(modid, "TrigConfig0", settings.s_trigConfig0);
    writeModuleSetting(modid, "TrigConfig1", settings.s_trigConfig1);
    writeModuleSetting(modid, "TrigConfig2", settings.s_trigConfig2);
    writeModuleSetting(modid, "TrigConfig3", settings.s_trigConfig3);
    writeModuleSetting(modid, "HOST_RT_PRESET", settings.s_HostRtPreset);
}
/**
 * loadChannelSettings
 *     Loads the settings for a single channel from the settings data.
 * @param modid   - Id of the module to load.
 * @param chan    - Channel to load.
 * @param settings - the settings data to load from.
 * @note we don't directly call Pixie16WriteSglChanPar but instead
 *       our method writeChannelSetting which does error analysis
 *       and reporting.
 */
void
ModuleWriter::loadChannelSettings(
    unsigned modid, unsigned chan, const ModuleSettings& settings
)
{
    writeChannelSetting(
        modid, chan, "TRIGGER_RISETIME", settings.s_triggerRiseTime[chan]
    );
    writeChannelSetting(
        modid, chan, "TRIGGER_FLATTOP", settings.s_triggerFlattop[chan]
    );
    writeChannelSetting(
        modid, chan, "TRIGGER_THRESHOLD", settings.s_triggerThreshold[chan]
    );
    writeChannelSetting(
        modid, chan, "ENERGY_RISETIME", settings.s_energyRiseTime[chan]
    );
    writeChannelSetting(
        modid, chan, "ENERGY_FLATTOP", settings.s_energyFlattop[chan]
    );
    writeChannelSetting(modid, chan, "TAU", settings.s_tau[chan]);
    writeChannelSetting(
        modid, chan, "TRACE_LENGTH", settings.s_traceLength[chan]
    );
    writeChannelSetting(
        modid, chan, "TRACE_DELAY", settings.s_traceDelay[chan]
    );
    writeChannelSetting(
        modid, chan, "VOFFSET" , settings.s_vOffset[chan]
    );
    writeChannelSetting(modid, chan, "XDT", settings.s_Xdt[chan]);
    writeChannelSetting(
        modid, chan, "BASELINE_PERCENT", settings.s_BaselinePct[chan]
    );
    writeChannelSetting(modid, chan, "EMIN", settings.s_Emin[chan]);
    writeChannelSetting(
        modid, chan, "BINFACTOR", settings.s_binFactor[chan]
    );
    writeChannelSetting(
        modid, chan, "BASELINE_AVERAGE", settings.s_baselineAverage[chan]
    );
    writeChannelSetting(
        modid, chan, "CHANNEL_CSRA", settings.s_chanCsra[chan]
    );
    writeChannelSetting(
        modid, chan, "CHANNEL_CSRB", settings.s_chanCsrb[chan]
    );
    writeChannelSetting(modid, chan, "BLCUT", settings.s_blCut[chan]);
    writeChannelSetting(
        modid, chan, "FASTTRIGBACKLEN", settings.s_fastTrigBackLen[chan]
    );
    writeChannelSetting (
        modid, chan, "CFDDelay", settings.s_CFDDelay[chan]
    );
    writeChannelSetting(
        modid, chan, "CFDScale", settings.s_CFDScale[chan]
    );
    writeChannelSetting(
        modid, chan, "CFDThresh", settings.s_CFDThreshold[chan]
    );
    writeChannelSetting(modid, chan, "QDCLen0", settings.s_QDCLen0[chan]);
    writeChannelSetting(modid, chan, "QDCLen1", settings.s_QDCLen1[chan]);
    writeChannelSetting(modid, chan, "QDCLen2", settings.s_QDCLen2[chan]);
    writeChannelSetting(modid, chan, "QDCLen3", settings.s_QDCLen3[chan]);
    writeChannelSetting(modid, chan, "QDCLen4", settings.s_QDCLen4[chan]);
    writeChannelSetting(modid, chan, "QDCLen5", settings.s_QDCLen5[chan]);
    writeChannelSetting(modid, chan, "QDCLen6", settings.s_QDCLen6[chan]);
    writeChannelSetting(modid, chan, "QDCLen7", settings.s_QDCLen7[chan]);
    writeChannelSetting(
        modid, chan, "ExtTrigStretch", settings.s_extTrigStretch[chan]
    );
    writeChannelSetting(
        modid, chan, "VetoStretch", settings.s_vetoStretch[chan]
    );
    writeChannelSetting(
        modid, chan, "MultiplicityMaskL",
        settings.s_multiplicityMaskL[chan]
    );
    writeChannelSetting(
        modid, chan, "MultiplicityMaskH",
        settings.s_multiplicityMaskH[chan]
    );
    writeChannelSetting(
        modid, chan, "ExternDelayLen", settings.s_externDelayLen[chan]
    );
    writeChannelSetting(
        modid, chan, "FtrigoutDelay", settings.s_FTrigoutDelay[chan]
    );
    writeChannelSetting(
        modid, chan, "ChanTrigStretch", settings.s_chanTriggerStretch[chan]
    );
}
/**
 * writeModuleSetting
 *    This is a wrapper for Pixie16WriteSglModPar which also analyzes
 *    the status returned and throw std::invalid_argument if there
 *    are errors.
 *
 * @param id -  id of the module to set.
 * @param name - parameter name (see Pixie16WriteSglModPar).
 * @param value - Value to write for that parameter.
 */
void
ModuleWriter::writeModuleSetting(
    unsigned id, const char* name, unsigned int value
)
{
    int stat = Pixie16WriteSglModPar(name, value, id);
    if (stat < 0) {
        std::stringstream msg;
        msg << " Pixie16WriteSglModPar error: ";
        switch (-stat) {
            case 1:
                msg << id << " is an invalid module number";
                break;
            case 2:
                msg << "'" << name << "' is an invalid parameter name";
                break;
            case 3:
                msg << " unable to program  FIPPI after loading the parameter\n";
                msg << "You probably need to boot module " << id;
                break;
            case 4:
                msg << "Failed to locate the BLcut value after loading parameter\n";
                msg << "You probably need to boot module "  << id;
                break;
            default:
                msg << "Undocumented error: " << stat << " module: " << id
                    << ", paramter '" << name << "'";
                break;
        }
        throw std::invalid_argument(msg.str());
    }
}
/**
 * writeChannelSetting
 *    This is a wrapper for Pixie16WriteSglChanPar that analyzes and
 *    reports the status returned from that call. Non normal status
 *    values are signalled via a std::invalid_argument exception.
 *
 * @param modid - id of the module to write.
 * @param chan  - Channel number to write to.
 * @param name  - Parameter name to write (see Pixie16WriteSglPar).
 * @param value - value to write.
 */
void
ModuleWriter::writeChannelSetting(
    unsigned modid, unsigned chan, const char* name, double value
)
{
    int stat = Pixie16WriteSglChanPar(name, value, modid, chan);
    
    if (stat < 0) {
        std::stringstream msg;
        msg << "Pixie16WriteSglChanPar failed module: " << modid
        << " channel: " << chan << std::endl;
        
        switch (-stat) {
            case 1:
                msg << "Invalid module number";
                break;
            case 2:
                msg << "Invalid channel number";
                break;
            case 3:
                msg << "Invalid parameter name: '" << name << "'";
                break;
            case 4:
                msg << "FIPPIP Programming failed. "
                    << "You probably have to boot the module";
                break;
            case 5:
                msg << "Failed to find BLcut.  "
                    << "You probably have to boot the module";
                break;
            case 6:
                msg << " Unable to set the DACS. "
                    << "You probably have to boot the module";
                break;
            default:
                msg <<"Undocumented error " << stat
                    << "Check Pixie16Msg.txt for hopefully more info";
                break;
        }
        
        throw std::invalid_argument(msg.str());
    }
}

}                               // DDAS Namespace.
