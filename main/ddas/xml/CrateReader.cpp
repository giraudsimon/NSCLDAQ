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

/** @file:  CrateReader.cpp
 *  @brief: Implementation of concrete methods of the CrateReader.
 */
#include "CrateReader.h"
#include "SettingsReader.h"

namespace DDAS {
/**
 * constructor
 *    Save the slots we're reading in.
 *
 *  @param crate - Id of the crate we're reading.
 *  @param slots - the slotst to read.
 */
CrateReader::CrateReader(unsigned crate, const std::vector<unsigned short>& slots) :
    m_slots(slots), m_crateId(crate)
{}

/**
 * destructor
 */
CrateReader::~CrateReader() {}

/**
 * readCrate
 *    This is a strategy driver for the reader.  It has helpers that
 *    must be implemented to function:
 *    -   crateReader - must return a dynamically allocated reader for the
 *        data in a slot.
 *    -   getEvtLen - must return the expected event length for a slot
 * These are called in that order for each slot.
 * @return Crate - the processed crate description.
 */
Crate
CrateReader::readCrate()
{
    Crate result;
    result.s_crateId = m_crateId;
    for (int i =0; i < m_slots.size(); i++) {
        unsigned short slot = m_slots[i];
        SettingsReader* pReader = createReader(slot);
        
        Slot s;
        s.s_slotNum = slot;
        s.s_settings = pReader->get();
        result.s_slots.push_back(s);
	delete pReader;
    }
    
    return result;
}

}                            // DDAS Namespace.
