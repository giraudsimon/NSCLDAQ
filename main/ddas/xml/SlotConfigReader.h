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

/** @file:  SlotConfigReader.h
 *  @brief: Read slot configuration XML files.
 */
#ifndef SLOTCONFIGREADER_H
#define SLOTCONFIGREADER_H
#include <vector>
#include <string>
#include <tinyxml2.h>

namespace DDAS {
 
/**
 *  @class SlotConfigReader
 *      Reads a slot configuration for a Crate.  These look like e.g.:
 *
 * \verbatim
 *     <DDASCrate>
 *      <slot number="2" />
 *      slot number="5" />
 *      slot number="6" />
 *     </DDASCrate>
 * \endverbatim
 *
 * The sample file below specifies there are modules in slots 2,5, and 6.
 */
class SlotConfigReader {
private:
    std::string m_filename;
public:
    SlotConfigReader(const char* file);
    SlotConfigReader(const SlotConfigReader& rhs);
    SlotConfigReader& operator=(const SlotConfigReader& rhs);
private:
    int operator==(const SlotConfigReader& rhs);
    int operator!=(const SlotConfigReader& rhs);
public:
    std::vector<unsigned> read();
private:
    std::vector<unsigned> processSlots(tinyxml2::XMLElement& root);
    unsigned getSlot(tinyxml2::XMLElement& el);
    void checkSlotMap(const std::vector<unsigned>& slots);
    void checkError(tinyxml2::XMLError err, tinyxml2::XMLDocument& doc);
};
}                                   // DDAS namespace.
 
 
 #endif