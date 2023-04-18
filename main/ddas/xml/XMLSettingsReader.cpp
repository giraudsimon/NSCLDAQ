/*
    This software is Copyright by the of Trustees of Michigan
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

/** @file:  XMLSettingsReader.cpp
 *  @brief: Implement DDAS::XMLSettingsReader class (see XMLSettingsReader.h).
 */
 
#include "XMLSettingsReader.h"
#include <tinyxml2.h>
#include <sstream>
#include <string.h>
#include <stdexcept>

#include "tinyxmlutil.h"

namespace DDAS {
 /**
 *  constructor
 *     Save the module file for later.
 * @param filename - name of the XML file to process.
 */
XMLSettingsReader::XMLSettingsReader(const char* file) :
    m_filename(file)
{
    
}
 /**
  * destructor
  *    Just kill off the documents in m_modules
  */
 XMLSettingsReader::~XMLSettingsReader()
 {
    
 }
 /**
  * get
  *     Return the module settings for all of the
  *     documents stored.  The first element of the pair in m_modules
  *     will be substituted into the s_modId field overriding the
  *     value in the XML.
  * @return std::vector<ModuleSettings>  - the settings.
  */
 
///////////////////////////////////////////////////////////////////////
// Private utilities:
ModuleSettings
XMLSettingsReader::get()
{
    auto pDocument = loadModuleFile(m_filename.c_str());
    try {
        auto result =  getModule(*pDocument);
        delete pDocument;
        return result;
    }
    catch (...) {
        delete pDocument;
        throw;
    }
    

}

/**
 * loadModuleFile
 *     Loads a module XML file into an XMLDocument and returns that document
 *
 *  @param filename - filename for the document to load.
 *  @return tinyxml2::XMLDocument*
 */
tinyxml2::XMLDocument*
XMLSettingsReader::loadModuleFile(const char* pFile)
{
    tinyxml2::XMLDocument* pResult = new tinyxml2::XMLDocument;
    checkXmlError(pResult->LoadFile(pFile), *pResult);
    return pResult;
}


/**
 * getModule
 *    Get the module settings from a document.  This is a matter of
 *    getting the module level stuff with getModuleSettings then we
 *    iterate over the channnel tags calling getChannelSettings
 *    for each of those.
 *
 *    @todo  Some error checking to ensure that all top level tags and
 *           all channels are present, however this file could,
 *           after all be used to load a Pixie-4 so maybe missing channels
 *           are ok?
 * @param doc - References the document containing the definition.
 *              Must have a <Module> root tag.
 * @return ModuleSettings - the settings extracted from the file.
 */
ModuleSettings
XMLSettingsReader::getModule(tinyxml2::XMLDocument& doc)
{
    ModuleSettings result;
    memset(&result, 0, sizeof(ModuleSettings));   // Null it out.
    
    // Must have a root and it must be Module:
    
    tinyxml2::XMLElement* pRoot = doc.RootElement();
    if (std::string("Module") != pRoot->Name()) {
        std::stringstream msg;
        msg << "All module XML files must have <Module> root tags. Got: "
            << pRoot->Name();
        throw std::invalid_argument(msg.str());
    }
    getModuleSettings(result, *pRoot);
    
    // Now iterate over the channels:
    
    tinyxml2::XMLElement* pChan = pRoot->FirstChildElement("channel");
    while (pChan) {
        getChannelSettings(result, *pChan);
        pChan = pChan->NextSiblingElement("channel");
    }
    
    return result;
    
}
/**
 * getModuleSettings
 *    Get the module level settings into the settings structure.
 *    These are in well known tagsthat we will explicitly look for
 *    Once we have the tag we can use readValue to get the value of
 *    the parameter it holds.
 * @param[out] settings - the settings struct we're filling in.
 * @param root - the root document element (<Module>).
 * @note s_number and s_modId are the same.
 */
void
XMLSettingsReader::getModuleSettings(
    ModuleSettings& settings, tinyxml2::XMLElement& root
)
{
    tinyxml2::XMLElement* p;
    p = haveChild(root, "csra");
    readValue(settings.s_csra, *p);
    
    p = haveChild(root, "csrb");
    readValue(settings.s_csrb, *p);
    
    p = haveChild(root, "format");
    readValue(settings.s_format, *p);
    
    p = haveChild(root, "maxevents");
    readValue(settings.s_maxEvents, *p);
    
    p = haveChild(root, "synchwait");
    readValue(settings.s_synchWait, *p);
    
    p = haveChild(root, "insynch");
    readValue(settings.s_inSynch, *p);
    
    p = haveChild(root, "SlowFilterRange");
    readValue(settings.s_SlowFilterRange, *p);
    
    p = haveChild(root, "FastFilterRange");
    readValue(settings.s_FastFilterRange, *p);
    
    p = haveChild(root, "BackplaneTriggerEnables");
    readValue(settings.s_FastTrgBackPlaneEnables, *p);
    
    
    p = haveChild(root, "trigConfig0");
    readValue(settings.s_trigConfig0, *p);
    
    p = haveChild(root, "trigConfig1");
    readValue(settings.s_trigConfig1, *p);
    
    p = haveChild(root, "trigConfig2");
    readValue(settings.s_trigConfig2, *p);
    
    p = haveChild(root, "trigConfig3");
    readValue(settings.s_trigConfig3, *p);
    
    p = haveChild(root, "HostRTPreset");
    readValue(settings.s_HostRtPreset, *p);
}
/**
 * getChannelSettings
 *    Pull the settings for a single channel into the Module settings
 *    struct.
 *
 * @param settings  - the settings we're building up.
 * @param channel   - The <channel> tag element.
 */
void
XMLSettingsReader::getChannelSettings(
    ModuleSettings& settings, tinyxml2::XMLElement& channel
)
{
    // We need to figure out which channel we're talking about:
    
    uint32_t chan;
    getAttribute(chan, channel, "id");
    
    // Ok let's start grabbing what we need. We'll not care about
    // any extra tags for now:
    
    tinyxml2::XMLElement* p;
    
    p = haveChild(channel, "TriggerRiseTime");
    readValue(settings.s_triggerRiseTime[chan], *p);
    
    p = haveChild(channel, "TriggerFlatTop");
    readValue(settings.s_triggerFlattop[chan], *p);
    
    p = haveChild(channel, "TriggerThreshold");
    readValue(settings.s_triggerThreshold[chan], *p);
    
    p  = haveChild(channel, "EnergyRiseTime");
    readValue(settings.s_energyRiseTime[chan], *p);
    
    p = haveChild(channel, "EnergyFlatTop");
    readValue(settings.s_energyFlattop[chan], *p);
    
    p = haveChild(channel, "Tau");
    readValue(settings.s_tau[chan], *p);
    
    p = haveChild(channel, "TraceLength");
    readValue(settings.s_traceLength[chan], *p);
    
    p = haveChild(channel, "TraceDelay");
    readValue(settings.s_traceDelay[chan], *p);
    
    p = haveChild(channel, "VOffset");
    readValue(settings.s_vOffset[chan], *p);
    
    p = haveChild(channel, "XDT");
    readValue(settings.s_Xdt[chan], *p);
    
    p = haveChild(channel, "Baseline");
    readValue(settings.s_BaselinePct[chan], *p);
    
    p = haveChild(channel, "EMin");
    readValue(settings.s_Emin[chan], *p);
    
    p = haveChild(channel, "BinFactor");
    readValue(settings.s_binFactor[chan], *p);
    
    p = haveChild(channel, "BaselineAverage");
    readValue(settings.s_baselineAverage[chan], *p);
    
    p = haveChild(channel, "CSRA");
    readValue(settings.s_chanCsra[chan], *p);
    
    p = haveChild(channel, "CSRB");
    readValue(settings.s_chanCsrb[chan], *p);
    
    p = haveChild(channel, "BlCut");
    readValue(settings.s_blCut[chan], *p);
        
    p = haveChild(channel, "FastTriggerBacklen");
    readValue(settings.s_fastTrigBackLen[chan], *p);
    
    p = haveChild(channel, "CFDDelay");
    readValue(settings.s_CFDDelay[chan], *p);
    
    p = haveChild(channel, "CFDScale");
    readValue(settings.s_CFDScale[chan], *p);
    
    p = haveChild(channel, "CFDThresh");
    readValue(settings.s_CFDThreshold[chan], *p);
    
    p = haveChild(channel, "QDCLen0");
    readValue(settings.s_QDCLen0[chan], *p);
     
    p = haveChild(channel, "QDCLen1");
    readValue(settings.s_QDCLen1[chan], *p);
    
    p = haveChild(channel, "QDCLen2");
    readValue(settings.s_QDCLen2[chan], *p);
    
    p = haveChild(channel, "QDCLen3");
    readValue(settings.s_QDCLen3[chan], *p);
    
    p = haveChild(channel, "QDCLen4");
    readValue(settings.s_QDCLen4[chan], *p);
    
    p = haveChild(channel, "QDCLen5");
    readValue(settings.s_QDCLen5[chan], *p);
    
    p = haveChild(channel, "QDCLen6");
    readValue(settings.s_QDCLen6[chan], *p);
    
    p = haveChild(channel, "QDCLen7");
    readValue(settings.s_QDCLen7[chan], *p);
    
    p = haveChild(channel, "ExtTrigStretch");
    readValue(settings.s_extTrigStretch[chan], *p);
    
    p = haveChild(channel, "VetoStretch");
    readValue(settings.s_vetoStretch[chan], *p);
    
    p = haveChild(channel, "MultiplicityMasks");
    getAttribute(settings.s_multiplicityMaskL[chan], *p, "low");
    getAttribute(settings.s_multiplicityMaskH[chan], *p, "high");
    
    p = haveChild(channel, "ExternDelayLen");
    readValue(settings.s_externDelayLen[chan], *p);
    
    p = haveChild(channel, "FTrigoutDelay");
    readValue(settings.s_FTrigoutDelay[chan], *p);
    
    p = haveChild(channel, "ChanTrigStretch");
    readValue(settings.s_chanTriggerStretch[chan], *p);
    
}

}                                            // namespace DDAS.