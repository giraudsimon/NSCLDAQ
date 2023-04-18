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

/** @file:  XMLSettingsWriter.h
 *  @brief: Sublcass of SettingsWriter that writes DSP settings in xml
 */
#ifndef XMLSETTINGSWRITER_H
#define XMLSETTINGSWRITER_H


#include "SettingsWriter.h"
#include <string>

namespace tinyxml2 {
class XMLPrinter;
}
namespace DDAS {
    /**
     * @class XMLSettingsWriter
     *     Writes settings to a group of xml files.  One file is written per
     *     modules.  On construction a base filename is passed in.
     *     Each module's settings are written into a per module file
     *     named base_Module_n.xml  n is the module number (not the slot).
     *
     *     The structure of each XML file is:
     *      <module>
     *          <csra value='nnn' />
     *          <csrb value='nnn' />
     *          <format value='nnnn' />
     *          <maxevents value='nn' />
     *          <synchwait value='0|1' />
     *          <insych value='0|1' />
     *          <SlowFilterRange value='nnn' />
     *          <FastFilterRange value='nnn' />
     *          <EnableFastBackplaneTrigger value='0|1' />
     *          <trigConfig0-3 value='n' />
     *          <HostRTPreset  value='n' />
     *          <!--  channel parameters:  />
     *
     *          <channel id='0-15'>
     *              <TriggerRiseTime units='microseconds' value='n' />
     *              <TriggerFlatTop units='microseconds' value='n' />
     *              <TriggerThreshold units='adccounts' value='n' />
     *              <EnergyRiseTime units='microseconds' value ='n' />
     *              <EnergyFlatTop  units='microseconds' value='n' />
     *              <Tau units='microseconds' value='n' />
     *              <TraceLength units='microseconds'  value='n' />
     *              <TraceDelay units='microseconds'  value='n' />
     *              <VOffset units='volts' value='n' />
     *              <XDT units='microseconds'  value ='n' />
     *              <Baseline units='percent' value='n' />
     *              <EMin units='none' value='n' />
     *              <BinFactor units='non' value='n' />
     *              <BaselineAverage units='none' value='n' />
     *              <CSRA units='bitmask' value='n' />
     *              <CSRB units='bitmask' value='n' />
     *              <BlCut units='none' value='n' />
     *              <Integrator units='none' value='n' />
     *              <FastTriggerBackLen units='microseconds'  value='n' />
     *              <CFDDelay units='microseconds' value='n' />
     *              <CFDScale units='none' value='n' />
     *              <CFDThresh units='none' value='n' />
     *              <QDCLen0-7 units='microseconds' value='n' />
     *              <ExtTrigStretch units='microseconds'  value='n' />
     *              <VetoStretch units='microseconds'  value='n' />
     *              <MultiplicityMasks low='n' high='n' />
     *              <ExternDelayLen units='microseconds' value='n' />
     *              <FTrigoutDelay units='microseconds' value='n'  />
     *              <ChanTrigStretch units='microseconds' 'value='n' />
     *          </channel>
     *      </module>
     */
    class XMLSettingsWriter : public SettingsWriter
    {
    private:
        std::string m_filename;
    public:
        XMLSettingsWriter(const char* filename);
        virtual ~XMLSettingsWriter() {}
        virtual void write(const ModuleSettings& dspSettings);
    private:
        
        void writeModule(tinyxml2::XMLPrinter& printer, const ModuleSettings& settings);
        void writePerModule(tinyxml2::XMLPrinter& printer, const ModuleSettings& settings);
        void writePerChannel(
            tinyxml2::XMLPrinter& printer, const ModuleSettings& settings,
            unsigned chan
        );
    public:
        static void writeValue(tinyxml2::XMLPrinter& printer, unsigned value);
        static void writeValue(tinyxml2::XMLPrinter& printer, double value);
        static void writeValue(tinyxml2::XMLPrinter& printer, bool value);
        static void writeMultiplicityMasks(
            tinyxml2::XMLPrinter& printer, uint32_t  low, uint32_t high
         );
    };
}                                      // DDAS Namespace.
 
 #endif