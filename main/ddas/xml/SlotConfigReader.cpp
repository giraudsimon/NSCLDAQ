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

/** @file:  SlotConfigReader.cpp
 *  @brief: Read slot configurations.
 */
 
#include "SlotConfigReader.h"
#include <stdexcept>
#include <sstream>
#include <set>

namespace DDAS {

/**
 * constructor
 *    Just save the filename.
 *
 *  @param file - name of the file to process.
 */
SlotConfigReader::SlotConfigReader(const char* file) :
    m_filename(file)
{}
/**
 * copy constructor
 */
SlotConfigReader::SlotConfigReader(const SlotConfigReader& rhs) :
    m_filename(rhs.m_filename)
{}

/**
 * assignment
 */
SlotConfigReader&
SlotConfigReader::operator=(const SlotConfigReader& rhs)
{
    if (this != &rhs) {
        m_filename = rhs.m_filename;
    }
    return *this;
}
/**
 * read
 *    Process the configuration file document.
 *
 * @return std::vector<unsigned> - vector of slots in the order specified by the XML file.
 * @note see SlotConfigReader.h for the format of the file.
 * @note the order of the slots specified implies an assignment of
 *       module ID to slots.
 */
std::vector<unsigned>
SlotConfigReader::read()
{
    tinyxml2::XMLDocument doc;
    checkError(doc.LoadFile(m_filename.c_str()), doc);
    
    // Root element must be a <DDDASCrate> element:
    
    tinyxml2::XMLElement& root(*(doc.RootElement()));
    const char* rootName = root.Name();
    if (std::string("DDASCrate") != root.Name()) {
        std::stringstream msg;
        msg << "Root element for Slot maps must be <DDASCrate> was: "
            << rootName;
        throw std::invalid_argument(msg.str());
    }
    // Get the slot map from the contents of the <DDASCrate> tag
    
    std::vector<unsigned> result;
    result = processSlots(root);       // Process <DDASCrateContents>
    checkSlotMap(result);              // Check result validity.
    return result;
    
}
////////////////////////////////////////////////////////////////
// Private utilitiy methods.

/**
 * processSlots
 *    Process the contents of the <DDASCrate> root element.
 *    This should be a gaggle of child-less <slot> tags that
 *    all have a "number" attribute that has an integer.
 * @param root - reference to the root element.
 */
std::vector<unsigned>
SlotConfigReader::processSlots(tinyxml2::XMLElement& root)
{
    std::vector<unsigned> result;
    tinyxml2::XMLElement* slotEl = root.FirstChildElement();
    
    while (slotEl) {
        result.push_back(getSlot(*slotEl));
        slotEl = slotEl->NextSiblingElement(); // ignore comments/text.
    }
    return result;
}
/**
 * getSlot
 *    Check that
 *    - The element is  a "slot"
 *    - The element has no children.
 *    - We can extract an unsigned slot number from the "number" attribute
 *      of the element.
 *
 * @param el - <slot> element
 * @return unsigned - value of the number attribute.
 */
unsigned
SlotConfigReader::getSlot(tinyxml2::XMLElement& slot)
{
    // Element must be a <slot> tag.
    const char* name;
    if (std::string("slot") != name) {
        std::stringstream msg;
        msg << "Contents of <DDASCrate> must be <slot> tags. Got: "
            << name;
        throw std::invalid_argument(msg.str());
    }
    // Element can have no children:
    
    if (!(slot.NoChildren())) {
        throw std::invalid_argument(
            "<slot> tags cannot have children."
        );
    }
    // Attempt to extract the unsigned number value.
    
    tinyxml2::XMLDocument& doc(*slot.GetDocument());
    unsigned result;
    checkError(slot.QueryUnsignedAttribute("number", &result), doc);
    
    return result;
}
/**
 * checkSlotMap
 *    Just because the slot map can be built up without errors does not
 *    imply the map is valid.  Valid maps:
 *    -   Have no duplicates.
 *    -   Have no slots less than 2.
 *    -   I've never seen a backplane with more than 18 slots
 *        for now limit the slot number to < 19
 *   @param slots - proposed slot map.
 *   @throw std::invalid_argument if there are problems with that map.
 *   
 */
void
SlotConfigReader::checkSlotMap(const std::vector<unsigned>& slots)
{
    std::set<unsigned> dupcheck;         // Used to check for duplicates.
    
    for (int i =0; i < slots.size(); i++) {
        unsigned s = slots[i];
        if (s < 2) {
            std::stringstream msg;
            msg << "Slot numbers must be >= 2. You gave me : " << s;
            throw std::invalid_argument(msg.str());
        }
        if (s > 18) {
            std::stringstream msg;
            msg << "Slot numbers must be < 18.  You gave me: " << s;
            throw std::invalid_argument(msg.str());
        }
        if (dupcheck.count(s)) {
            std::stringstream msg;
            msg << "Slot maps must have unique slot numbers. You duplicated: "
                << s;
            throw std::invalid_argument(msg.str());
        } else {
            dupcheck.insert(s);
        }
    }
    // We probably should not have an empty slot map:
    
    if (slots.empty()) {
        throw std::invalid_argument("Empty slot map - this is madness.");
    }
    // We survived so all is good.  Don't throw any exceptions.
}
/**
 * checkError
 *    Tinyxml2 error checking
 *    - If the status code handed to us is ok, just return.
 *    - If not, format an error message based on the document and the
 *     error code.
 *  @param err - tinyxml2::XMLError error code.
 *  @param doc - References the document we're processing when this
 *               status was created.
 *  @throw std::invalid_argument if the status is an error.
 *  @todo - We may wish to factor this out of here and XMLSettingsReader.
 */
void
SlotConfigReader::checkError(
        tinyxml2::XMLError err, tinyxml2::XMLDocument& doc
)
{
    if (err != tinyxml2::XML_SUCCESS) {
        throw std::invalid_argument(doc.ErrorStr());
    }    
}

}                               // DDAS Namespace.