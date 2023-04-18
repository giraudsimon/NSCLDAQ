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

/** @file: PixieSettingsReader.cpp
 * 
 *  @brief: Implement class to read settings from a device.
 */

#include "PixieSettingsReader.h"
#include <config.h>
#include <config_pixie16api.h>
#include <stdexcept>
#include <sstream>
#include <system_error>
#include <HardwareRegistry.h>
#include <FirmwareVersionFileParser.h>
#include <Configuration.h>

#include <fstream>

//  Constants that are file level:



namespace DDAS {
    /**
     * constructor
     *   - Initialize access to the PXI crate
     *   - We assume there are 14 slots (maximum) and 13 modules in slots 2-14
     *     respectively.
     *   -  We probe for the set of modules actually in the crate by getting module
     *      information.  If a requested module is not present, std::invalid_argument
     *      is thrown
     *
     *  @param id       - id of the module we want to read from.
     */
    PixieSettingsReader::PixieSettingsReader(unsigned id) :
        m_moduleId(id)
    {
    }
    /**
     * destructor does nothing for now:
     */
    PixieSettingsReader::~PixieSettingsReader()
    {

    }
    /**
     * get
     *    Get the settings from the hardware
     *
     * @return std::vector<ModuleSettings> - the module settings.
    */
    ModuleSettings
    PixieSettingsReader::get()
    {
        
        return readSettings(m_moduleId);
    }
    /////////////////////////////////////////////////////////////////////////////
    // Private utility methods.
    
    ModuleSettings
    PixieSettingsReader::readSettings(unsigned module)
    {
        ModuleSettings result;
        
        // Read the module parameters:
        
        unsigned ivalue;                    // For elements that are not unsigned int:
        
        checkModuleParamRead(
            Pixie16ReadSglModPar("MODULE_CSRA", &result.s_csra, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("MODULE_CSRB", &result.s_csrb, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("MODULE_FORMAT", &result.s_format, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("MAX_EVENTS", &result.s_maxEvents, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("SYNCH_WAIT", &ivalue, module)
        );
        result.s_synchWait = ivalue != 0;
        
        checkModuleParamRead(
            Pixie16ReadSglModPar("IN_SYNCH", &ivalue, module)
        );
        result.s_inSynch = ivalue != 0;
        
        checkModuleParamRead(
            Pixie16ReadSglModPar(
                "SLOW_FILTER_RANGE", &result.s_SlowFilterRange, module
            )
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar(
                "FAST_FILTER_RANGE", &result.s_FastFilterRange, module
            )
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar(
                "FastTrigBackplaneEna", &result.s_FastTrgBackPlaneEnables,
                module
            )
        );

        checkModuleParamRead(
            Pixie16ReadSglModPar("TrigConfig0", &result.s_trigConfig0, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("TrigConfig1", &result.s_trigConfig1, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("TrigConfig2", &result.s_trigConfig2, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("TrigConfig3", &result.s_trigConfig3, module)
        );
        checkModuleParamRead(
            Pixie16ReadSglModPar("HOST_RT_PRESET", &result.s_HostRtPreset, module)
        );
        
        // Per channel parameters:
        
        double dval;
        for (unsigned c = 0; c < CHANNELS; c++) {
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TRIGGER_RISETIME",
                    &result.s_triggerRiseTime[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TRIGGER_FLATTOP",
                    &result.s_triggerFlattop[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TRIGGER_THRESHOLD",
                    &result.s_triggerThreshold[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("ENERGY_RISETIME",
                    &result.s_energyRiseTime[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("ENERGY_FLATTOP",
                    &result.s_energyFlattop[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TAU",
                    &result.s_tau[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TRACE_LENGTH",
                    &result.s_traceLength[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("TRACE_DELAY",
                    &result.s_traceDelay[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("VOFFSET",
                    &result.s_vOffset[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("XDT",
                    &result.s_Xdt[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("BASELINE_PERCENT",
                    &result.s_BaselinePct[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("EMIN",
                    &dval, module, c
                ), c
            );
            result.s_Emin[c] = dval;
            checkChannelParamRead(
                Pixie16ReadSglChanPar("BINFACTOR",
                    &dval, module, c
                ), c
            );
            result.s_binFactor[c] =  dval;
            checkChannelParamRead(
                Pixie16ReadSglChanPar("BASELINE_AVERAGE",
                    &dval, module, c
                ), c
            );
            result.s_baselineAverage[c] = dval;
            checkChannelParamRead(
                Pixie16ReadSglChanPar("CHANNEL_CSRA",
                    &dval, module, c
                ), c
            );
            result.s_chanCsra[c] = dval;
            
            checkChannelParamRead(
                Pixie16ReadSglChanPar("CHANNEL_CSRB",
                    &dval, module, c
                ), c
            );
            result.s_chanCsrb[c] = dval;
            
            checkChannelParamRead(
                Pixie16ReadSglChanPar("BLCUT",
                    &result.s_blCut[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("FASTTRIGBACKLEN",
                    &result.s_fastTrigBackLen[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("CFDDelay",
                    &result.s_CFDDelay[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("CFDScale",
                    &result.s_CFDScale[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("CFDThresh",
                    &result.s_CFDThreshold[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen0",
                    &result.s_QDCLen0[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen1",
                    &result.s_QDCLen1[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen2",
                    &result.s_QDCLen2[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen3",
                    &result.s_QDCLen3[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen4",
                    &result.s_QDCLen4[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen5",
                    &result.s_QDCLen5[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen6",
                    &result.s_QDCLen6[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("QDCLen7",
                    &result.s_QDCLen7[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("ExtTrigStretch",
                    &result.s_extTrigStretch[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("VetoStretch",
                    &result.s_vetoStretch[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("MultiplicityMaskL",
                    &dval, module, c
                ), c
            );
            result.s_multiplicityMaskL[c] = dval;
            checkChannelParamRead(
                Pixie16ReadSglChanPar("MultiplicityMaskH",
                    &dval, module, c
                ), c
            );
            result.s_multiplicityMaskH[c] = dval;
            checkChannelParamRead(
                Pixie16ReadSglChanPar("ExternDelayLen",
                    &result.s_externDelayLen[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("FtrigoutDelay",
                    &result.s_FTrigoutDelay[c], module, c
                ), c
            );
            checkChannelParamRead(
                Pixie16ReadSglChanPar("ChanTrigStretch",
                    &result.s_chanTriggerStretch[c], module, c
                ), c
            );
        }
        
        return result;
    }

    /**
     * checkModuleParamRead
     *    Does error handling and exception throwing on errors
     *    from Pixie16ReadSglModPar
     * @param code - return code from Pixie16ReadSglModPar
     * @throw std::invalid_argument for errors as those are what can be
     *        returned.
     */
    void
    PixieSettingsReader::checkModuleParamRead(int code)
    {
        switch (-code) {
            case 0:
                return;
            case 1:    
                throw std::invalid_argument(
                    "ReadSglModPar - invalid module number"
                );
            case 2:
                throw std::invalid_argument(
                    "ReadSglModPar - Invalid module parameter name string"
                );
            default:
                throw std::invalid_argument(
                    "ReadSglModPar - unrecognized return value"
                );
        }
        throw std::logic_error(
            "checkModuleParamRead - should not fall through case - contact scientific software group"
        );
    }
    /**
     * checkChannelParamRead
     *    Check/report on errors in Pixie16ReadSglChanPar.
     *
     * @param stat - the status from the operation.
     * @param chan - The channel number.
     * @throw std::invalid_argument - the typial reason for failure.
     * 
     */
    void
    PixieSettingsReader::checkChannelParamRead(int code, int channel)
    {
        std::stringstream msg;
        switch (-code)
        {
        case 0:
            return;                     // Success.
        case 1:
            msg << "ReadSglChanPar failed due to invalid pixie16 module number";
            break;
        case 2:
            msg << "ReadSGlChanPar failed due to invalid channel number " << channel;
            break;
        case 3:
            msg << "ReadSgLChanPar failed due to invalid parmaeter name";
            break;
        default:
            throw std::logic_error(
                "ReadSglChanPar failed with an undocumented error code, contact scientific software"
            );
        }
        // msg jhas the invalid argument string:
        
        throw std::invalid_argument(msg.str());
        
    }
}                                  // DDAS namespace
