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

/** @file:  PixieCrateReader.cpp
 *  @brief: Implement the PixieCrateReader class.
 */
#include "PixieCrateReader.h"
#include "PixieSettingsReader.h"
#include "CrateManager.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace DDAS {
/**
 * constructor
 *    Just initializes the base class:
 *
 *  @param crate - id of the crate.
 *  @param slots - Vector of slots.
 */
PixieCrateReader::PixieCrateReader(
    unsigned crate, const std::vector<unsigned short>& slots
) : CrateReader(crate, slots), m_pManager(new CrateManager(slots))
{
    
}
/**
 * destructor
 *    Release the crate manager.
 */
PixieCrateReader::~PixieCrateReader()
{
    delete m_pManager;
}
/**
 * createReader
 *    Creates a reader for a single slot.  We just need
 *    to translate the slot number to a module id.  The module
 *    id is the corresponding index in the m_slots vector.
 *
 * @param slot - slot number to create a reader for.
 * @return SettingsReader* - pointer to a dynamically created
 *         Pixie settings reader object.
 */
SettingsReader*
PixieCrateReader::createReader(unsigned short slot)
{
    // can't just use find as that makes a pointer like object.
    // fortunately distance tells us how far into the container
    // our iterator is:
    
    unsigned short id = m_pManager->moduleId(slot);
    m_pManager->loadDSPAddressMap(id);
    
    return new PixieSettingsReader(id);
}

}                    // DDAS Namespace