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

/** @file:  XMLCrateWriter.cpp
 *  @brief: Implements the crate writer for XML files.
 */
#include "XMLCrateWriter.h"
#include "XMLSettingsWriter.h"
#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iterator>
namespace DDAS {

/**
 * constructor
 * We just save the data for now.
 *    @param crateFile - Where we're going to write the crate file.
 *    @param settings  - Settings info (passed to the base class).
 *    @param metadata  - Per slot metadata for the <slot> tag.
 */
XMLCrateWriter::XMLCrateWriter(
    std::string  crateFile,
    const Crate& settings,
    const std::vector<ModuleInformation>& metadata
) :
    CrateWriter(settings),
    m_additionalInfo(metadata), m_crateFile(crateFile), m_pPrinter(nullptr)
{
    // There must be metadata for each slot in the slot map:
    
    if (settings.s_slots.size() != metadata.size()) {
        std::stringstream msg;
        msg << "XML Crate writer metadata argument must have the same "
            << "number of elements as the slot map.\n";
        msg << "Slot map size: " << settings.s_slots.size()
            << " metadata size: " << metadata.size();
            
        throw std::invalid_argument(msg.str());
    }
}

/**
 * destructor
 *    Kill off the XMLPrinter if it's still alive.
 */
XMLCrateWriter::~XMLCrateWriter()
{
    delete m_pPrinter;;
}

/**
 * startCrate
 *    - Create the printer to print to strings.
 *    - Open the crate tag giving it the crate id.
 * Notes:
 *    -   Each slot tag is issued in GetWriter.
 *    -   The XMLPrinter is opened in string mode.
 *    -   On end crate, the crate file is  opened and the
 *        XML string written to it.   
 *  @param id - Id of the crate.
 *  @param slots (unused) slot map of the crate.
 */
void
XMLCrateWriter::startCrate(
    int id, const std::vector<unsigned short>& slots
)
{
    m_slots = slots;                // So we can compute slot ids
    m_pPrinter = new tinyxml2::XMLPrinter();
    m_pPrinter->PushHeader(true, true);
    
    m_pPrinter->OpenElement("crate");
    writeAttribute(*m_pPrinter, "id", id);
    
}
/**
 * endCrate
 *    Called when the entire crate has been writeen out.
 *    - Close the top level element.
 *    - Open the crate file.
 *    - Write the XML string from the printer.
 *    - delete the printer and set m_pPrinter -> nullptr.
 *      (this allows multiple writes).
 * @param id - crate id.
 * @param slots - slot map.
 */
void
XMLCrateWriter::endCrate(int id, const std::vector<unsigned short>& slots)
{
    m_pPrinter->CloseElement();           // Close the <crate> tag.
    std::ofstream s(m_crateFile);
    if (!s) {
        std::stringstream msg;
        msg << "Unable to open the crate output file: " << m_crateFile;
        throw std::invalid_argument(msg.str());
    }
    
    s << m_pPrinter->CStr();
    s.close();
    
    delete m_pPrinter;
    m_pPrinter = nullptr;
}
/**
 * getWriter
 *    Our responsibility from the point of view of the base class is
 *    to produce an XMLSettingsWriter for it to write the
 *    settings file for a single module.  Additionally, we need to
 *    add a <slot> tag to the crate file.  To do this we need to:
 *    - open the tag,
 *    - Write the slot attribute.
 *    - Figure out the id of the slot and look up the corresponding
 *      ModuleInformation record.
 *    - Use the module info record to write the remaining attributes.
 *    - Close the <slot> tag.
 *    We can use the filename in the ModuleSettings to create the settings
 *    writer which we pass back to the caller.
 *
 * @param slotNum - number of the slot to write.
 */
SettingsWriter*
XMLCrateWriter::getWriter(unsigned short slot)
{
    assert(m_pPrinter);
    m_pPrinter->OpenElement("slot");
    writeAttribute(*m_pPrinter, "number", slot);
    
    int id = slotId(slot);
    auto& info = m_additionalInfo[id];
    
    writeSlotAttributes(info);
    m_pPrinter->CloseElement();
    
    return new XMLSettingsWriter(info.s_moduleSettingsFile.c_str());
}
//////////////////////////////////////////////////////////////////////
// private utilities:

/**
 * writeSlotAttributes.
 *   Given module information for a slot element that's open,
 *   write the appropriate mandatory and optional elements.
 *
 * @param info - references the module information.
 */
void
XMLCrateWriter::writeSlotAttributes(const ModuleInformation& info)
{
    // Mandatory attributes:
    
    writeAttribute(*m_pPrinter, "evtlen", info.s_eventLength);
    writeAttribute(
        *m_pPrinter, "configfile", info.s_moduleSettingsFile.c_str()
    );
    // Optional attributes - only write if the corresponding flag is true.
    
    if (info.s_specifyFifoThreshold) {
        writeAttribute(
            *m_pPrinter, "fifo_threshold", info.s_fifoThreshold
        );
    }
    
    if(info.s_specifyTimestampScale) {
        writeAttribute(
            *m_pPrinter, "timestamp_scale", info.s_timestampScale
        );
    }
    if (info.s_specifyInfinityClock) {
        writeAttribute(
            *m_pPrinter, "infinity_clock", info.s_infinityClock
        );
    }
    if (info.s_specifyExternalClock) {
        writeAttribute(
            *m_pPrinter, "external_clock", info.s_externalClock
        );
    }
    
}
/**
 * writeAttribute
 *    This set of overloaded methods adds an attribute/value to the
 *    currently open element.  Note that they are designed to be
 *    hoisted into the tinyxmlutil module.  For now, however we
 *    put them here.
 * @parma p   - references the printer to which we're writing.
 * @param name - Name of the attribute being written.
 * @param value - This is the overload that defines the value.
 *
 * @todo could do this as a templated class with a specialization
 * for const char* attribute values(?).
 */
void
XMLCrateWriter::writeAttribute(
    tinyxml2::XMLPrinter& p, const char* name, const char* value
)
{
    p.PushAttribute(name, value);
}
void
XMLCrateWriter::writeAttribute(
    tinyxml2::XMLPrinter& p, const char* name, int value
)
{
    std::stringstream s;
    s << value;
    writeAttribute(p, name, s.str().c_str());
}
void
XMLCrateWriter::writeAttribute(
    tinyxml2::XMLPrinter& p, const char* name, bool value
)
{
    writeAttribute(p, name, (value ? "true" : "false"));
}

void XMLCrateWriter::writeAttribute(
    tinyxml2::XMLPrinter& p, const char* name, double value
)
{
    std::stringstream s;
    s << value;
    writeAttribute(p, name, s.str().c_str());
}
void XMLCrateWriter::writeAttribute(
    tinyxml2::XMLPrinter& p, const char* name, unsigned value
)
{
    std::stringstream s;
    s << value;
    writeAttribute(p, name, s.str().c_str());
}
/**
 * slotId
 *   Translalte a crate number into a slot id.
 * @param slot - Slot number.
 * @return unsigned - corresponding slot id.
 */
unsigned
XMLCrateWriter::slotId(unsigned short slot)
{
    auto p = std::find(m_slots.begin(), m_slots.end(), slot);
    if (p == m_slots.end()) {
        std::stringstream s;
        s << "No such slot : " << slot;
        throw std::invalid_argument(s.str());
    } else {
        return std::distance(m_slots.begin(), p);
    }
}
}                            // namespace DDAS