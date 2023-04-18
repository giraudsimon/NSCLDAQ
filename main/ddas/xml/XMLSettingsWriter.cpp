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

/** @file:  XMLSettingsWriter.cpp
 *  @brief: Implement the settings writer class.
 */
#include "XMLSettingsWriter.h"
#include <tinyxml2.h>
#include <stdio.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>


namespace DDAS {
    
template<class T>
void writeValueElement(tinyxml2::XMLPrinter& p, const char* elname, T& value)
{
    p.OpenElement(elname);
    XMLSettingsWriter::writeValue(p, value);
    p.CloseElement();
}
template<class T>
void writeUnitsValueElement(
    tinyxml2::XMLPrinter& p, const char* elname, const char* units, T& value
)
{
    p.OpenElement(elname);
    p.PushAttribute("units", units);
    XMLSettingsWriter::writeValue(p, value);
    p.CloseElement();
}
/**
 * constructor
 *    Just save the filename for now:
 * @param 
 */
XMLSettingsWriter::XMLSettingsWriter(const char* filename) :
    m_filename(filename)
{}

/**
 * write
 *    For each module in the settings vector:
 *    - Generate a module filename.
 *    - Open a file pointer for the module XML file.
 *    - Create an XMLPrinter for the module file.
 *    - Write the module settings.
 *
 *    @param dspSettings - the module settings to write.
 *    @throw std::runtime_error - unable to open the output file.
 */
void
XMLSettingsWriter::write(const ModuleSettings& dspSettings)
{    
    FILE* fp = fopen(m_filename.c_str(), "w");
    if (!fp) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open XML output file: "
            << m_filename << " :  " << strerror(e);
        throw std::runtime_error(msg.str());
    }
    tinyxml2::XMLPrinter printer(fp);
    printer.PushHeader(true, true);
    printer.OpenElement("Module");
    writeModule(printer, dspSettings);
    printer.CloseElement();
        fclose(fp);
}
////////////////////////////////////////////////////////////////////////////
//  Private utility methods.


/**
 * writeModule
 *    Write the data for a single module.
 *
 * @param printer - the XML printer doig the writing.
 * @param module  - The module being written.
 *
 */
void
XMLSettingsWriter::writeModule(
    tinyxml2::XMLPrinter& printer, const ModuleSettings& settings
)
{
    writePerModule(printer, settings);
    for (int i =0; i < 16; i++) {
        writePerChannel(printer, settings, i);
    }
}
/**
 * writePerModule
 *    Write the per module settings.
 *
 *  @param printer - The xml printer.
 *  @param settings - the settings.
 */
void
XMLSettingsWriter::writePerModule(
        tinyxml2::XMLPrinter& printer, const ModuleSettings& settings
)
{
    
    writeValueElement(printer, "csra", settings.s_csra);
    writeValueElement(printer, "csrb", settings.s_csrb);
    writeValueElement(printer, "format", settings.s_format);
    writeValueElement(printer, "maxevents", settings.s_maxEvents);
    writeValueElement(printer, "synchwait", settings.s_synchWait);
    writeValueElement(printer, "insynch", settings.s_inSynch);
    writeValueElement(printer, "SlowFilterRange", settings.s_SlowFilterRange);
    writeValueElement(printer, "FastFilterRange", settings.s_FastFilterRange);
    writeValueElement(
        printer, "BackplaneTriggerEnables", settings.s_FastTrgBackPlaneEnables
    );
    writeValueElement(printer, "trigConfig0", settings.s_trigConfig0);
    writeValueElement(printer, "trigConfig1", settings.s_trigConfig1);
    writeValueElement(printer, "trigConfig2", settings.s_trigConfig2);
    writeValueElement(printer, "trigConfig3", settings.s_trigConfig3);
    writeValueElement(printer, "HostRTPreset", settings.s_HostRtPreset);
    
    
    
}
/**
 * writePerChannel
 *    Write the per channel settings for channel.
 * @param printer - the XML Printer object.
 * @param settings - the settings object.
 * @param chan     - Channel number ([0,15])
*/
void
XMLSettingsWriter::writePerChannel(
        tinyxml2::XMLPrinter& printer, const ModuleSettings& settings,
        unsigned chan
)
{
    printer.OpenElement("channel");
    std::stringstream idval; idval << chan;
    printer.PushAttribute("id", idval.str().c_str());
    
    writeUnitsValueElement(
        printer, "TriggerRiseTime", "microseconds",
        settings.s_triggerRiseTime[chan]
    );
    writeUnitsValueElement(
        printer, "TriggerFlatTop", "microseconds",
        settings.s_triggerFlattop[chan]
    );
    writeUnitsValueElement(
        printer, "TriggerThreshold", "adccounts",
        settings.s_triggerThreshold[chan]
    );
    writeUnitsValueElement(
        printer, "EnergyRiseTime", "microseconds", settings.s_energyRiseTime[chan]
    );
    writeUnitsValueElement(
        printer, "EnergyFlatTop", "microseconds", settings.s_energyFlattop[chan]
    );
    writeUnitsValueElement(printer, "Tau", "microseconds", settings.s_tau[chan]);
    writeUnitsValueElement(
        printer, "TraceLength", "microseconds", settings.s_traceLength[chan]
    );
    writeUnitsValueElement(
        printer, "TraceDelay", "microseconds", settings.s_traceDelay[chan]
    );
    writeUnitsValueElement(printer, "VOffset", "volts", settings.s_vOffset[chan]);
    writeUnitsValueElement(printer, "XDT", "microseconds" ,settings.s_Xdt[chan]);
    writeUnitsValueElement(
        printer, "Baseline", "percent", settings.s_BaselinePct[chan]
    );
    writeUnitsValueElement(
        printer, "EMin", "none", settings.s_Emin[chan]
    );
    writeUnitsValueElement(
        printer, "BinFactor", "none", settings.s_binFactor[chan]
    );
    writeUnitsValueElement(
        printer, "BaselineAverage", "none", settings.s_baselineAverage[chan]
    );
    writeUnitsValueElement(printer, "CSRA", "bitmask", settings.s_chanCsra[chan]);
    writeUnitsValueElement(printer, "CSRB", "bitmask", settings.s_chanCsrb[chan]);
    writeUnitsValueElement(printer, "BlCut", "none", settings.s_blCut[chan]);

    writeUnitsValueElement(
        printer, "FastTriggerBacklen", "microseconds", settings.s_fastTrigBackLen[chan]
    );
    writeUnitsValueElement(
        printer, "CFDDelay", "microseconds" ,settings.s_CFDDelay[chan]
    );
    writeUnitsValueElement(
        printer, "CFDScale", "none", settings.s_CFDScale[chan]
    );
    writeUnitsValueElement(
        printer, "CFDThresh", "none", settings.s_CFDThreshold[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen0", "microseconds", settings.s_QDCLen0[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen1", "microseconds", settings.s_QDCLen1[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen2", "microseconds", settings.s_QDCLen2[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen3", "microseconds", settings.s_QDCLen3[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen4", "microseconds", settings.s_QDCLen4[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen5", "microseconds", settings.s_QDCLen5[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen6", "microseconds", settings.s_QDCLen6[chan]
    );
    writeUnitsValueElement(
        printer, "QDCLen7", "microseconds", settings.s_QDCLen7[chan]
    );
    writeUnitsValueElement(
        printer, "ExtTrigStretch", "microseconds",
        settings.s_extTrigStretch[chan]
    );
    writeUnitsValueElement(
        printer, "VetoStretch", "microseconds", settings.s_vetoStretch[chan]
    );
    writeMultiplicityMasks(
        printer, settings.s_multiplicityMaskL[chan],
        settings.s_multiplicityMaskH[chan]
    );
    writeUnitsValueElement(
        printer, "ExternDelayLen", "microseconds", settings.s_externDelayLen[chan]
    );
    writeUnitsValueElement(
        printer, "FTrigoutDelay", "microseconds", settings.s_FTrigoutDelay[chan]
    );
    writeUnitsValueElement(
        printer, "ChanTrigStretch", "microseconds",
        settings.s_chanTriggerStretch[chan]
    );
    
    
    
    printer.CloseElement();
}
/**
 * writeValue
 *    Write an integer value attribute:
 * @param printer - the XML printer.
 * @param value   - the unsigned integer value.
 */
void
XMLSettingsWriter::writeValue(tinyxml2::XMLPrinter& printer, unsigned value)
{
    std::stringstream v;
    v << value;
    printer.PushAttribute("value", v.str().c_str());
}
/**
 * writeElement
 *    Same as above with a double value.
 */
void
XMLSettingsWriter::writeValue(tinyxml2::XMLPrinter& printer, double value)
{
    std::stringstream v;
    v << value;
    printer.PushAttribute("value", v.str().c_str());
}
/**
 * writeElement
 *    same as above but with boolean value
 */
void
XMLSettingsWriter::writeValue(tinyxml2::XMLPrinter& printer, bool value)
{
    printer.PushAttribute("value", value ? "true" : "false");
}
/**
 * writeMultiplicityMasks
 *    Writes the multiplicity mask element
 *    <MultiplicityMasks low="lowmask" high="highmask"/>
 * @param printer - referencs the tinxyml2 printer.
 * @param low     - low order mask bits.
 * @param high    - high order mask bits.
 */
void
XMLSettingsWriter::writeMultiplicityMasks(
    tinyxml2::XMLPrinter& printer, uint32_t low, uint32_t high
)
{
    // Stringize the low and high values:
    
    std::stringstream l;  l << low;
    std::stringstream h;  h << high;
    
    printer.OpenElement("MultiplicityMasks");
    printer.PushAttribute("low", l.str().c_str());
    printer.PushAttribute("high", h.str().c_str());
    
    printer.CloseElement();
}
}                               // namespace DDAS