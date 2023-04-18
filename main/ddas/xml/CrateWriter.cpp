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

/** @file:  CrateWriter.cpp
 *  @brief: Implement the strategy pattern main logic for writing crates
 *  
 */

#include "CrateWriter.h"
#include "SettingsWriter.h"
#include <sstream>
#include <stdexcept>
namespace DDAS {
/**
 * constructor
 *   Simply save the crate configuration struct locally.
 * @param crate - crate settings/definition.
 * @param slots - Slot number vector.
 */
CrateWriter::CrateWriter(
    const Crate& settings
) :
    m_settings(settings)
{
}
/**
 * write
 *    Write the crate.  The sequence (strategy) is:
 *   \verbatim
 *     startCrate
 *     foreach module
 *       getWriter
 *       write module via writer.
 *    endCrate
 */
void
CrateWriter::write()
{
    std::vector<unsigned short> slots = makeSlotVector();
    startCrate(m_settings.s_crateId, slots);
    for (int i =0; i < slots.size(); i++) {
        SettingsWriter* pWriter = getWriter(slots[i]);
        pWriter->write(m_settings.s_slots[i].s_settings);
        delete pWriter;
    }
    endCrate(m_settings.s_crateId, slots);
}
///////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * makeSlotVector
 *   Make a slot map vector for the modules in the crate.
 *
 * @return std::vector<unsigned short>
 */
std::vector<unsigned short>
CrateWriter::makeSlotVector() const
{
    std::vector<unsigned short> result;
    for (int i = 0; i < m_settings.s_slots.size(); i++) {
        result.push_back(m_settings.s_slots[i].s_slotNum);
    }
    return result;
}
}