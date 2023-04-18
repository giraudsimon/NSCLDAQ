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

/** @file:  SetFileReader.cpp
 *  @brief: Implement the SetFileReader class.
 */

#include "SetFileReader.h"
#include <sstream>
#include <stdexcept>
#include <config.h>
#include <config_pixie16api.h>
#include <assert.h>
#include <math.h>

namespace DDAS {

/**
 * constructor
 *   @param setfile - path to setfile.
 *   @param varfile - Path to the variable file.
 *   @param Mhz     - Digitizer speed in Mhz.
 *   @param slot    - Number of the slot in the DSP Para file we want.
 */
SetFileReader::SetFileReader(
    const char* setfile, const char* varfile, unsigned MHz, unsigned slot
) :
    m_setfile(setfile), m_varfile(varfile),
    m_Mhz(MHz), m_slotId(slot)
{}

/**
 * get
 *    Gets all module settings from the set file.
 *    A set file actually has up to 24 module settings.  To get them
 *    all we assume that they are exactly end to end and that the last
 *    item in each setfile is a module level thing (they actually do that for
 *    us).
 *
 *  @return std::vector<ModuleSettings> - the module settings for each item.
 */
DDAS::ModuleSettings
SetFileReader::get()
{
    DDAS::ModuleSettings result;
    auto vars     = DDAS::SetFile::readVarFile(m_varfile.c_str());
    auto settings = DDAS::SetFile::readSetFile(m_setfile.c_str());
  
    try {
        // Read the set file and the vars file:
        

        
        
        // Locate our settings file:
        
        auto myslot   = findSlot(settings, vars);  // Throws if not found.
        result= getModuleSettings(myslot.first, myslot.second, vars);
        
    } catch (...) {
        // Free the settings and rethrow:
        
        DDAS::SetFile::freeSetFile(settings.second);
        throw;
    }
    DDAS::SetFile::freeSetFile(settings.second);
    return result;
    
}

//////////////////////////////////////////////////////////////////////
// Private utility methods:

/**
 *  findSlot
 * try to find the settings that corresond to our slot (m_nbslotId):
 *
 * @param vars - describes the settings file.
 * @param map  - The parsed DSP memory offsets map file.
 * @return std::pair<unsigned, uint32_t*>
 *             - first is the number of remaining longs in the file.
 *               second is the pointer to our vars.
 * @throws std::invalid_argument - if the slot is not found.
 */
std::pair<unsigned, uint32_t*>
SetFileReader::findSlot(
    const std::pair<unsigned, uint32_t*>& vars,
    const VarOffsetArray& map
)
{
    auto here = vars;
    auto nameMap = DDAS::SetFile::createVarNameMap(map);
    auto& slotInfo = nameMap["SlotID"];
    
    while(here.second[slotInfo.s_longoff] != m_slotId) {
        here = next(here.first, here.second, map);
        if (here.first == 0) {
            // Not found.
            
            std::stringstream  msg;
            msg << " Unable to find settings information for slot "
                << m_slotId;
            throw std::invalid_argument(msg.str());
        }
    }
    return here;
    
    
}

ModuleSettings
SetFileReader::getModuleSettings(
    unsigned nLongs, uint32_t* pVars, const VarOffsetArray& map
)
{
    auto settings = SetFile::populateSetFileMap(nLongs, pVars, map);
    
    
    // Alas pulling stuff out into the Module Settings is a painful process.
    // Made no easier by the fact that ModuleSettings is in useful units
    // like microseconds and the setfile is in digitizer units.
    
    // The module level items are, at least, as is:
    
    ModuleSettings  result;
    
    result.s_csra      = getSingleItem(settings, "ModCSRA");
    result.s_csrb      = getSingleItem(settings, "ModCSRB");
    result.s_format    = getSingleItem(settings, "ModFormat");
    result.s_maxEvents = getSingleItem(settings, "MaxEvents");
    result.s_synchWait = getSingleItem(settings, "SynchWait");
    result.s_inSynch   = getSingleItem(settings, "InSynch");
    result.s_SlowFilterRange = getSingleItem(settings, "SlowFilterRange");
    result.s_FastFilterRange = getSingleItem(settings, "FastFilterRange");
    result.s_FastTrgBackPlaneEnables =
        getSingleItem(settings, "FastTrigBackplaneEna");
    auto triggers      = getItemVector(settings, "TrigConfig");
    result.s_trigConfig0 = triggers[0];
    result.s_trigConfig1 = triggers[1];
    result.s_trigConfig2 = triggers[2];
    result.s_trigConfig3 = triggers[3];
    result.s_HostRtPreset = getSingleItem(settings, "HostRunTimePreset");
    uint32_t msps      = getMsPs();
    
    // The remainder of the items are per channel.  Each one
    // will give a vector of 16 elements.  Some will need to be
    // turned into usec.
    
    double ffRange = result.s_FastFilterRange;
    setChannelParamsUs(
        result.s_triggerRiseTime, settings, "FastLength", msps, ffRange
    );
    setChannelParamsUs(
        result.s_triggerFlattop, settings,  "FastGap", msps, ffRange
    );
    setChannelParamsD(result.s_triggerThreshold, settings,  "FastThresh");
    convertTriggerThresholds(
        result.s_triggerThreshold, settings, msps
    );
    
    setChannelParamsD(result.s_energyRiseTime, settings, "SlowLength");
    convertEnergyTimes(
        result.s_energyRiseTime, result.s_SlowFilterRange, msps
    );

    setChannelParamsD(result.s_energyFlattop, settings, "SlowGap");
    convertEnergyTimes(
        result.s_energyFlattop,  result.s_SlowFilterRange, msps
    );
    
    
    auto taus = getFloats(settings, "PreampTau");
    for (int i =0; i < taus.size(); i++) {
        result.s_tau[i] = taus[i];
    }
    setChannelParamsD(result.s_traceLength, settings, "TraceLength");
    convertTraceLength(
        result.s_traceLength, result.s_FastFilterRange, msps
    );
    setChannelParamsD(result.s_traceDelay, settings, "TriggerDelay");
    convertTraceDelay(
        result.s_traceDelay, settings, result.s_FastFilterRange, msps
    );
    
    setChannelParamsV(
        result.s_vOffset, settings, "OffsetDAC", DAC_VOLTAGE_RANGE
    );
    setChannelParamsD(result.s_Xdt, settings, "Xwait");
    computeXdt(result.s_Xdt);
    setChannelParamsD(result.s_BaselinePct, settings, "BaselinePercent");
    setChannelParamsI(result.s_Emin, settings, "EnergyLow");
    setChannelParamsI(result.s_binFactor, settings, "Log2Ebin");
    convertBinFactor(result.s_binFactor);
    setChannelParamsI(result.s_baselineAverage, settings, "Log2Bweight");
    convertBlAverage(result.s_baselineAverage);
    setChannelParamsI(result.s_chanCsra, settings, "ChanCSRa");
    setChannelParamsI(result.s_chanCsrb, settings, "ChanCSRb");
    setChannelParamsD(result.s_blCut, settings, "BLcut");
    setChannelParamsUs(
        result.s_fastTrigBackLen, settings, "FastTrigBackLen", msps, ffRange
    );
    setChannelParamsUs(
        result.s_CFDDelay, settings, "CFDDelay", msps, ffRange
    );
    setChannelParamsD(result.s_CFDScale, settings, "CFDScale");
    setChannelParamsD(result.s_CFDThreshold, settings, "CFDThresh");
    
    setChannelParamsD(result.s_QDCLen0, settings, "QDCLen0");
    convertQdcLen(result.s_QDCLen0, msps);
    setChannelParamsD(result.s_QDCLen1, settings, "QDCLen1");
    convertQdcLen(result.s_QDCLen1, msps);
    setChannelParamsD(result.s_QDCLen2, settings, "QDCLen2");
    convertQdcLen(result.s_QDCLen2, msps);
    setChannelParamsD(result.s_QDCLen3, settings, "QDCLen3");
    convertQdcLen(result.s_QDCLen3, msps);
    setChannelParamsD(result.s_QDCLen4, settings, "QDCLen4");
    convertQdcLen(result.s_QDCLen4, msps);
    setChannelParamsD(result.s_QDCLen5, settings, "QDCLen5");
    convertQdcLen(result.s_QDCLen5, msps);
    setChannelParamsD(result.s_QDCLen6, settings, "QDCLen6");
    convertQdcLen(result.s_QDCLen6, msps);
    setChannelParamsD(result.s_QDCLen7, settings, "QDCLen7");
    convertQdcLen(result.s_QDCLen7, msps);
    
    setChannelParamsUs(
        result.s_extTrigStretch, settings, "ExtTrigStretch", msps, ffRange
    );
    setChannelParamsUs(
        result.s_vetoStretch, settings, "VetoStretch", msps, ffRange
    );
    setChannelParamsI(
        result.s_multiplicityMaskL, settings, "MultiplicityMaskL"
    );
    setChannelParamsI(
        result.s_multiplicityMaskH, settings, "MultiplicityMaskH"
    );
    setChannelParamsUs(
        result.s_externDelayLen, settings, "ExternDelayLen", msps, ffRange
    );
    setChannelParamsUs(
        result.s_FTrigoutDelay, settings, "FtrigoutDelay", msps, ffRange
    );
    setChannelParamsUs(
        result.s_chanTriggerStretch, settings, "ChanTrigStretch", msps,
        ffRange
    );
    
    return result;                 // Whew.
}
/**
 * getSingleItem
 *    Gets the value of a single item (module level).
 *
 * @param settings - the set file map.
 * @param name     - name of the key to get.
 * @return uint32_t - The value from the variable
 */
uint32_t
SetFileReader::getSingleItem(
    const DDAS::SetFileByName& settings, const char* name
)
{
    const Variable& v(throwIfNotPresent(settings, name));
    return v.s_value;
}
/**
 * getItemVector
 *    Get a a vector of items from the hashed out set file:
 * @param settings - settings file map.
 * @param name     - Name of item to get.
 */
std::vector<uint32_t>
SetFileReader::getItemVector(
    const DDAS::SetFileByName& settings, const char* name
)
{
    const Variable& v(throwIfNotPresent(settings, name));
    if (v.s_values.size() == 0) {
        std::stringstream msg;
        msg << "getItemVector for " << name << " Item is not a vector";
        throw std::invalid_argument(msg.str());
    }
    return v.s_values;
}
/**
 * getFloats
 *    Some items are actually floats stored in a uint32_t container.
 *  @param settings - the settings data
 *  @param which    - The setting we want.
 *  @return std::vector<float>
 */
std::vector<float>
SetFileReader::getFloats(const DDAS::SetFileByName& settings, const char* name)
{
    const Variable& v(throwIfNotPresent(settings, name));
    std::vector<float> result;
    const float* p = reinterpret_cast<const float*>(v.s_values.data());
    
    // Only works if:
    
    assert(sizeof(float) == sizeof(uint32_t));
    
    for (int i =0; i < v.s_values.size(); i++) {
        result.push_back(p[i]);
    }
    
    return result;
}
/**
 * setChannelParamsUs
 *     Convert per channel parameters to microseconds and store them
 *      in the storage for them.:
 *
 *  @param[out] dest - pointer to where the results should be stored.
 *  @param src  - References the source of the data.
 *  @param which  - Describes what to look for in src.
 *  @param msps   - Number o MegaSamples per second of the digitizer's adc.
 *  @param fastFilterRange - range of the fast filter 
 */
void
SetFileReader::setChannelParamsUs(
    double* dest, const SetFileByName& src,
    const char* which, uint32_t msps, double fastFilterRange
)
{
    // Get the source data:
    
    const Variable& v(throwIfNotPresent(src, which));
    
    
    // The integer division by 100 gives 1, 2, 5 accordingly
    
    double divisor = (double)(msps)/(msps/100);
    double mult    = 1 << (int)fastFilterRange;
    for (int  i =0; i < v.s_values.size(); i++) {
        dest[i] = double(v.s_values[i]) * mult/divisor;
    }
}
/**
 * setChannelParamsV
 *    Set a voltage parameter.  This is assuming the DAC range
 *    is the voltage range allowed for a value between 0 and 65535
 *
 *  @param[out] dest - points to where the voltage parametesr are stored.
 *  @param src       - The map of set file parameters.
 *  @param which     - Name of the item to use.
 *  @param range     - Voltage range.
 */
void
SetFileReader::setChannelParamsV(
    double* dest, const SetFileByName& src,
    const char* which, uint32_t range
)
{
    const Variable& v(throwIfNotPresent(src, which));
    
    for (int i =0; i < v.s_values.size(); i++) {
        dest[i] =
            double(v.s_values[i])/65536.0*range - range/2.0;
            
    }
}
/**
 * setChanelParamsI
 *    Set an integer channel parameter that requires no conversion.
 *
 * @param[out] dest - pointer to where the integers get put.
 * @param      src  - The channel map from the set file.
 * @param      which - The key in the map to fetch.
 */
void
SetFileReader::setChannelParamsI(
    uint32_t* dest, const SetFileByName& src,
    const char* which
)
{
    const Variable& v(throwIfNotPresent(src, which));
    for (int i =0; i < v.s_values.size(); i++) {
        dest[i] = v.s_values[i];
    }
}
/**
 * setChannelParamsD
 *     Sets a double precision channel parameter that requires no
 *     conversions (other thatn integer -> double).
 *
 * @param[out] dest - arra into which the channel data are put.
 * @param      src  - The map of the set file.
 * @param      which- The key to fetch.
 */
void
SetFileReader::setChannelParamsD(
    double* dest, const SetFileByName& src,
    const char* which
)
{
    const Variable& v(throwIfNotPresent(src, which));
    for (int i =0; i < v.s_values.size(); i++) {
        dest[i] = double(v.s_values[i]);
    }
}
/**
 * convertTriggerThresholds
 *    Trigger thresholds also depend on the trigger rise time and the
 *    digitizer speed.
 *    Therefore this computation must be done:
 *
 * @param[inout] rawThresholds - input and output values for thresholds.
 * @param src          - set file, we need the raw "FastLength" again.
 * @param msps        - Digitizer MHz.
 */
 void
 SetFileReader::convertTriggerThresholds(
    double* rawThresholds, const DDAS::SetFileByName& src, uint32_t msps
 )
 {
    double multiplier;                   // divisor multiplier depends on msps
    if (msps == 100) multiplier = 1.0;
    else if (msps == 250) multiplier = 2.0;
    else if (msps == 500) multiplier = 5.0;
    else {
        throw std::invalid_argument("convertTriggerThresholds, unrecognized MSPS");
    }
    const Variable& fl(throwIfNotPresent( src, "FastLength"));
    
    for (int i =0; i < 16; i++) {
        double fastl = fl.s_values[i];
        rawThresholds[i] = rawThresholds[i]/(fastl*multiplier);
    }
 }
 /**
  * convertEnergyTimes
  *    These convert to microseconds depending on the slow Filter range
  *    and the digitizer MHz
  * @param[inout] raw - A raw energy value.
  * @param multiplier - The power of 2 of the multiplier.
  * @param msps       - Digitizer MSPS.
  */
 void
 SetFileReader::convertEnergyTimes(
    double* raw, double multiplier, uint32_t msps
 )
 {
    // Figure out the divisor:
    
    double divisor;
    if (msps == 100) divisor = 1.0;
    else if (msps == 250) divisor = 2.0;
    else if (msps == 500) divisor = 5.0;
    else {
        throw std::invalid_argument("convertEnergyTimes, unrecognized MSPS");
    }
    double mult = pow(2.0, multiplier);
    for  (int i = 0; i < 16; i++) {
        raw[i] = raw[i]*mult/(msps/divisor);
    }
    
 }
 /**
  *  convertTraceLength
  *     Conver the raw trace length to usec:
  *
  * @param[inout]  - the raw trace lengths.
  * @param ffr     - the fast filter range.
  * @param msps    - speed of the digitizer in msps
  */
 void
 SetFileReader::convertTraceLength(double* raw, double ffr, uint32_t msps)
 {
    double divisor = msps*pow(2.0, ffr);
    for (int i =0; i < 16; i++) {
        raw[i] = raw[i]/divisor;
    }
 }
 /**
  * convertTraceDelay
  *   This depends on one of the FIFO parameters (PAFlength), the
  *   fast filter range and the msps of the digitizer:
  *
  * @param[inout] raw - the raw trace delay.
  * @param src        - references the settings so we can get "PAFlength"
  * @param ffr        - fast filter range.
  * @param msps       - Digitizer speed.
  *
  */
 void
 SetFileReader::convertTraceDelay(
    double* raw, const DDAS::SetFileByName& src, double ffr, uint32_t msps
 )
 {
    double divisor;
    if (msps == 100) divisor = 1.0;
    else if (msps == 250) divisor = 2.0;
    else if (msps == 500) divisor = 5.0;
    else {
        throw std::invalid_argument("convertTraceDelay - invalid msps parameter");
    }
    
    double pow2ffr = pow(2.0, ffr);
    const Variable& paflen = throwIfNotPresent(src, "PAFlength");
    
    for (int i =0; i < 16; i++) {
        double pfl = double(paflen.s_values[i]);
        double tdelay = raw[i];
        
        raw[i] =
            (double)(pfl - (unsigned int)((double)tdelay / pow2ffr)) /
            ((double)msps/divisor) * pow2ffr;
    }
 }
 /**
  *  computeXdt
  *    Raw set file values must be divided by the DSP clock:
  *
  * @param[inout] raw - the raw values will be replaced by compued ones.
  */
 void
 SetFileReader::computeXdt(double* raw)
 {
    for (int i =0; i < 16; i++) {
        raw[i] = raw[i]/(double)(DSP_CLOCK_MHZ);
    }
 }
 /**
  * convertBinFactor
  *   @param[inout] raw bin factor value
  */
 void
 SetFileReader::convertBinFactor(uint32_t* raw)
 {
    for (int i =0; i < 16; i++) {
        raw[i] = (uint32_t)(pow(2.0, 32.0) - (double)raw[i]);
    }
 }
 /**
  * convertBlAverage
  *    Almost the same as above but 0.0 is a special case:
  * @param[inout] raw  bl average raw value
  *
  */
 void
 SetFileReader::convertBlAverage(uint32_t* raw)
 {
    for (int i = 0; i < 16; i++) {
        if (raw[i] != 0.0) {
            raw[i] = (uint32_t)(pow(2.0, 32.0) - (double)raw[i]);
        }
    }
 }
 /**
  * convertQdcLen
  *    Convert the raw QDC value to an integration length in usec.
  *
  * @param[inout] raw  -- the raw value from the set file.
  * @param msps -  Digitizer frequency in MHz
  *
  */
 void
 SetFileReader::convertQdcLen(double* raw, uint32_t msps)
 {
    double divisor = msps;
    if (msps == 500) divisor = divisor/5;
    
    for (int i =0; i < 16; i++) {
        raw[i] = raw[i]/divisor;
    }
 }
/**
 * throwIfNotPresent
 *    Throws an exception if a key is not present in a set file map
 *    else returns a reference to the value associated with that key
 *
 *  @param src   - set file map.
 *  @param which - Key the caller wants.
 *  @return Variable& - reference to the contents of the map.
 *  @throw std::invalid_argument - if no such key.
 */
const Variable&
SetFileReader::throwIfNotPresent(
    const SetFileByName& src, const char* which
)
{
    auto p = src.find(which);
    if (p == src.end()) {
        std::stringstream msg;
        msg << "Unable to find key " << which << " in the setfile map";
        throw std::invalid_argument(msg.str());
    }
    return p->second;
}
/**
 * getMsPs
 *    Get megasamples/second for the digitizer.  If there's
 *    an entry for the slot id in the m_slotToMHzMap we use that
 *    otherwise we default to 250MHz.
 * 
 * @return uint32_t MHz sampling frequency.
 */
uint32_t
SetFileReader::getMsPs()
{
    return m_Mhz;
}

/**
 * next
 *   Given where we are in the setfile buffer, return information about
 *   how to find the next module in the file:
 * @param nLnogs - number of longs remaining in the file.
 * @param pVars  - Pointer to the current module.
 * @param map    - Array of set file offsets (in offset order).
 * @return std::pair<nLongs, pVars> - describes the remaining file and pointer.
 * @retval {0, anything} - N_DSP_PAR is defined by XIA to be the uint32_t's in
 *          a set file module segment.
 */
std::pair<unsigned, uint32_t*>
SetFileReader::next(
    unsigned nLongs, uint32_t* pVars, const VarOffsetArray& map
)
{
  uint32_t offsetToNext = N_DSP_PAR;
    if (offsetToNext >= nLongs) {
        nLongs = 0;
    } else {
        nLongs -= offsetToNext;
    }
    pVars += offsetToNext;
    return {nLongs, pVars};
}
}                    // DDAS Namespace
