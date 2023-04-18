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

/** @file:  ToUtils.h
 *  @brief: Utilities for toxml, tocrate etc.
 */
#ifndef TOUTILS_H
#define TOUTILS_H
#include <vector>
#include <map>
#include <utility>

#include <ModuleSettings.h>
#include <XMLCrateReader.h>

std::pair<unsigned short, unsigned short> parseMspsOption(
    const char* opt
);

std::vector<unsigned short> makeSlotVector(
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotInfo    
);
std::vector<unsigned short> makeSlotVector(const DDAS::Crate& crate);

#endif