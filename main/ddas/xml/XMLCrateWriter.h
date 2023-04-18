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

/** @file:  XMLCrateWriter.h
 *  @brief: Defines the interfaces to write a crate using XML.
 */
#ifndef XMLCRATEWRITER_H
#define XMLCRATEWRITER_H
#include "CrateWriter.h"

#include <string>
#include <vector>
#include <tinyxml2.h>

namespace DDAS {
    /**
     * @class XMLCrateWriter
     *     Writes a crate worth of settings via XML
     *     This requires a few things:
     *     - A crate file to which to write the top level
     *       definitions.
     *     - A description of the stuff to write at top level.
     *  The second one is somewhat problematic.  Each slot has a
     *  <slot> tag in the main file that contains several bits of
     *  mandatory information and potentially optional information.
     *  Mandatory information is the
     *    - Slot number for the module (comes from the base class)
     *    - evtlen for the module
     *    - configuration file into which the module settings are written
     * Optional parameters:
     *    - fifo_threshold for the module, defines what it is that
     *                     makes a module 'trigger' the read.
     *    - infinity_clock - boolean true, if the module is run
     *                     with infinity clock mode
     *    - timestamp_scale - Multiplier for the timestamps emitted.
     *    - external_clock - Boolean, true if the module uses
     *                       external clock.
     *  All but the external_clock must come external to the
     *  settings.  Therefore, when the XMLCrateWriter is
     *  constructed, substantial additiona information must be provided
     *  for each module.
     *
     *  See the ModuleInformation struct in the class for that.
     */
    class XMLCrateWriter : public CrateWriter {
    public:
        struct ModuleInformation {
            
            // Mandatory things
            
            unsigned     s_eventLength;
            std::string  s_moduleSettingsFile;
            
            // Optionals have a flage to specify an a value if the flag is true
            
            bool         s_specifyFifoThreshold;
            unsigned     s_fifoThreshold;
            
            bool         s_specifyTimestampScale;
            double       s_timestampScale;
            
            bool         s_specifyInfinityClock;
            bool         s_infinityClock;
            
            bool         s_specifyExternalClock;
            bool         s_externalClock;
            
            ModuleInformation() :
                s_specifyFifoThreshold(false), s_specifyTimestampScale(false),
                s_specifyInfinityClock(false),
                s_specifyExternalClock(false) {}
        };
    private:
        std::vector<ModuleInformation> m_additionalInfo;
        std::string                    m_crateFile;
        tinyxml2::XMLPrinter*          m_pPrinter;
        std::vector<unsigned short>    m_slots;
    public:
        XMLCrateWriter(
            std::string  crateFile,
            const Crate& settings,
            const std::vector<ModuleInformation>& metadata
        );
        virtual ~XMLCrateWriter();
        
        // The interface required by the strategy pattern:
        
        virtual void startCrate(
            int id, const std::vector<unsigned short>& slots
        );
        virtual void endCrate(
            int id, const std::vector<unsigned short>& slots
        );
        virtual SettingsWriter* getWriter(unsigned short slotNum);
    private:
        void writeSlotAttributes(const ModuleInformation& info);
        unsigned slotId(unsigned short slotNumber);
        
        static void writeAttribute(
            tinyxml2::XMLPrinter& p, const char* name, int value
        );
        static void writeAttribute(
            tinyxml2::XMLPrinter& p, const char* name, bool value
        );
        static void writeAttribute(
            tinyxml2::XMLPrinter& p, const char* name, double value
        );
        static void writeAttribute(
            tinyxml2::XMLPrinter& p, const char* name, const char* value
        );
        static void writeAttribute(
            tinyxml2::XMLPrinter& p, const char* name, unsigned value
        );
    };
}

#endif
