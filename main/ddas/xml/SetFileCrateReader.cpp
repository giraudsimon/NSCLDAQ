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

/** @file:  SetFileCrateReader.cpp
 *  @brief: Implement the SetFileCrateReader class.
 */
#include "SetFileCrateReader.h"
#include "SetFileReader.h"
#include <sstream>

namespace DDAS {
/**
 * constructor
 *  - Construct the base class passing our slot map vector.
 *  - Build the maps for the var file an speeds.
 *
 * @param crateId - Id of the crate.
 * @param slots  - map of slots to read.
 * @param varFiles - vector of corresponding var files.
 * @param MHz      - Vector of corresponding speeds.
 */
SetFileCrateReader::SetFileCrateReader(
    const char* setFile, unsigned crateId,
    std::vector<unsigned short> slots, std::vector<std::string> varFiles,
    std::vector<unsigned short> MHz
) :
    CrateReader(crateId, slots),
    m_setFile(setFile)
{
    // All three vectors must share a common size.
    
    if(slots.size() != varFiles.size()) {
        std::stringstream s;
        s << "The number of slots specified was " << slots.size()
          << " but the number of varfiles specified differed and was: "
          << varFiles.size();
        throw std::invalid_argument(s.str());
    }
    if (slots.size() != MHz.size()) {
        std::stringstream s;
        s << "The number of slots specified was " << slots.size()
          << " but the number of speed entries was " << MHz.size();
        throw std::invalid_argument(s.str());
    }
    
    // Build the maps:
    
    for (int i =0; i < slots.size(); i++) {
        m_varFiles[slots[i]] = varFiles[i];
        m_MHz[slots[i]]      = MHz[i];
    }
}
/** createReader
 *   Create the reader for a slot.
 *
 *  @param slot - slot number to read.
 *  @return SettingsReader* - pointer to the new'd SetFileReader
 *                            object.
 */
SettingsReader*
SetFileCrateReader::createReader(unsigned short slot)
{
    // Get the per slot metadata:
    
    std::string varFile = m_varFiles.at(slot);
    unsigned    MHz     = m_MHz.at(slot);
    
    return new SetFileReader(
        m_setFile.c_str(), varFile.c_str(), MHz, slot
    );
}
}