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

/** @file:  XMLCrateReader.cpp
 *  @brief: Implement the XML based crate configuration reader.
 */
#include "XMLCrateReader.h"
#include "XMLSettingsReader.h"
#include <tinyxml2.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>
#include <config.h>
#include <config_pixie16api.h>

namespace DDAS {

 /**
  * constructor
  *    Given the crate information file, process it and use the
  *    resulting information to fill in the base class attributes:
  *
  *    *  m_slots   - The slot map.
  *    *  m_crateId - the id of this crate.
  *
  * @param configFile - the crate configuration file.
  * @throw std::invalid_argument if there are errors:
  *       - Opening the file.
  *       - Parsing it to XML
  *       - Semantics of the XML file.
  *   
  */
 XMLCrateReader::XMLCrateReader(const char* configFile)
 {
    FILE* fp = fopen(configFile, "r");
    if (!fp) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open XML Crate file: " << configFile
            << " : " << strerror(e);
        throw std::invalid_argument(msg.str());
    }
    processCrateFile(fp);
    fclose(fp);
    
    // Now setup the m_slots, m_crateId was already set.
    
    for (auto p = m_slotInfo.begin(); p != m_slotInfo.end(); p++) {
        m_slots.push_back(p->second.s_slot);
    }
    
 }
 /**
  * destructor just let the map destroy itself
  */
 XMLCrateReader::~XMLCrateReader() {}
 
/**
 * createReader
 *    Given a slot number:
 *    - Get the slot information.
 *    - Create an XMLSettingsReader on the filename that's in the
 *      SlotInformation struct for that slot.
 *  @param slot - slot number.
 *  @return SettingsReader* pointer to a dynamically created XMLSettingsReader
 */
SettingsReader*
XMLCrateReader::createReader(unsigned short slot)
{
    const SlotInformation& s(getSlotInfo(slot));   // Can throw.
    return new XMLSettingsReader(s.s_configFile.c_str());
}
/**
 * getEvtLen
 *    Returns the event length for a slot.
 *
 * @param slot - the slot number.
 * @return unsigned - Event length for the slot.
 * 
 */
unsigned
XMLCrateReader::getEvtLen(unsigned short slot)
{
    return getSlotInfo(slot).s_evtlen;
}
/**
 * getFifoThreshold
 *    Return the fifo threshold for a slot.
 *    
 * @param slot - slot number.
 * @return unsigned FIFO trigger threshold for the slot.
 */
unsigned
XMLCrateReader::getFifoThreshold(unsigned short slot)
{
    return getSlotInfo(slot).s_fifothreshold;
}
/**
 * isInfinityClock
 *    @param slot - slot to get the flag for.
 *    @bool       - Flag of whether or not to use infinity clock.
 */
bool
XMLCrateReader::isInfinityClock(unsigned short slot)
{
    return getSlotInfo(slot).s_infinityclock;
}
/**
 * getTimestampScale
 *    Returns the timestamp scale factor.
 * @param slot  - slot number to use.
 * @return double - multiplier for the timstamp, whatever the source.
 */
double
XMLCrateReader::getTimestampScale(unsigned short slot)
{
    return getSlotInfo(slot).s_timestampscale;
}
/**
 * isExternalClock
 *   @param slot - slot number to check.
 *   @return bool - true if the external timestamp should be used for
 *                  event building.
 */
bool
XMLCrateReader::isExternalClock(unsigned short slot)
{
    return getSlotInfo(slot).s_externalclock;
}
//////////////////////////////////////////////////////////////////////
// Private utility functions.


/**
 * getSlotInfo
 *    Returns a reference to the slot information associated with a
 *    slot number.  If that does not exist, throws an invalid_argument
 *
 * @param slot  The slot number.
 * @return SlotInformation& - reference to that slots information struct.
 */
const XMLCrateReader::SlotInformation&
XMLCrateReader::getSlotInfo(unsigned short slot)
{
    auto p = m_slotInfo.find(slot);
    if (p == m_slotInfo.end()) {
        std::stringstream msg;
        msg << "Nonexistent slot number passed to getSlotInfo: "
            << slot;
        throw std::invalid_argument(msg.str());
    }
    return p->second;
}
/**
 *  processCrateFile
 *     -  Parse the crate file.
 *     -  Get the root element and be sure it's a <crate>
 *     -  Extract the id from the crate.
 *     -  Iterate over all top level children:
 *        * Make sure they're valid slot tags.
 *        * Make a slot information struct for them.
 *        * Add the slot to those known by the base class.
 *        * Put the slot information in the map.
 *  @param pFile - FILE* open on the file.
 *
 *  @note passing a FILE*, allows testing with memory resident XML
 *        (see e.g. fmemopen(3)).
 */
void
XMLCrateReader::processCrateFile(FILE* pFile)
{
    tinyxml2::XMLDocument doc;
    auto err  = doc.LoadFile(pFile);
    checkXmlError(err, doc);
    
    // The root element must be a <crate> tag with an id unsigned
    // attribute:
    
    auto root = doc.RootElement();
    const char* rootName = root->Name();
    if (std::string("crate") != rootName) {
        std::stringstream msg;
        msg << "Crate XML file root tag must be <crate> but was: "
            << rootName;
        throw std::invalid_argument(msg.str());
    }
    // Get the crate id - we don't check for that to be the only attrib.
    
    
    checkXmlError(
        root->QueryUnsignedAttribute("id", &m_crateId), doc
    );
    // Now we iterate through the children of the root element
    // - They must be elements.
    // - They must be valid <slot> elements.
    // - Process the tag to get a SlotInformation struct and
    // - add that to the map:
    
    auto pNode = root->FirstChild();
    while (pNode) {
        auto pSlot = pNode->ToElement();
        if (!pSlot) {
            
            // Comments are allowed and ignored:
            
            if (!pNode->ToComment()) {
                
                throw std::invalid_argument(
                    "All document elements below <crate> must be <slot> tags"
                );
            }
        } else {
            
            // I have a <slot> tag, process it.
            
            SlotInformation info = processSlotInfo(*pSlot);
            checkDuplicateSlot(info);
            m_slotInfo[info.s_slot] = info;
           
        }
        pNode = pNode->NextSibling();
    }
}
/**
 * processSlotInfo
 *    At this point we know the slot tag is ok:
 *    -  Ensure we have the mandatory slot, configfile and evtlen
 *       attributes
 *    -  If there are other optional attributes pull them out too.
 *    -  Bundle all the stuff into a SlotInformation struct
 *       and return that to the caller.
 * @note we just let slide any attributes that are not recognized.
 *
 * @param slot - reference to the XML <slot> element tag.
 * @return SlotInformation
 * @throw std::invalid_argument if required attributes are missing
 *                              or if attributes are malformed.
 */
XMLCrateReader::SlotInformation
XMLCrateReader::processSlotInfo(tinyxml2::XMLElement& slot)
{
    auto pDoc = slot.GetDocument();   // for checkXmlError.
    SlotInformation result;
    // Let's set default settings for the optional attributes:
    
    result.s_fifothreshold  = EXTFIFO_READ_THRESH*10;
    result.s_infinityclock  = false;
    result.s_timestampscale = 1.0;
    result.s_externalclock  = false;
    
    // We must have slot, configfile and evtlen attributes.
    
    getAttribute(result.s_slot, slot, "number");
    
    const char* file;
    
    checkXmlError(
        slot.QueryStringAttribute("configfile", &file),
        *pDoc
    );
    result.s_configFile = file;
    
    getAttribute(result.s_evtlen, slot, "evtlen");

    // The following attributes are optional.
    
    result.s_fifothreshold =
        slot.UnsignedAttribute("fifo_threshold", result.s_fifothreshold);
    result.s_infinityclock =
        slot.BoolAttribute("infinity_clock", result.s_infinityclock);
    result.s_timestampscale =
        slot.DoubleAttribute("timestamp_scale", result.s_timestampscale);
    result.s_externalclock =
        slot.BoolAttribute("external_clock", result.s_externalclock);
    
    // If we got this far, return the info
    
    return result;
}
/**
 * checkDuplicateSlot
 *    XML File semantics require that all physical slots only
 *    have one <slot> tag for them.   Given a parsed slot tag,
 *    this method determines if there's already a SlotInformation
 *    record for this slot in m_slotInfo
 *
 *  @param slot - the slot to check.
 *  @throw std::invalid_argument if that slot already was seen.
 */
void
XMLCrateReader::checkDuplicateSlot(const SlotInformation& slot)
{
    if (m_slotInfo.find(slot.s_slot) != m_slotInfo.end()) {
        std::stringstream msg;
        msg << "There is a duplicate slot tag for slot number: "
            << slot.s_slot;
        throw std::invalid_argument(msg.str());
    }
}

}                                  // DDAS Namespace.
