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

/** @file:  ToUtils.cpp
 *  @brief: Utilities common to e.g. toxml and tocrate
 */
#include <ToUtils.h>
#include <sstream>
#include <stdexcept>

/**
 * parseMspsOption
 *    Given the MSPS string of the form slot:speed
 *    returns the pair of {slot,speed}.
 *
 * @param opt - the option value
 * @return std::pair<unsigned short, unsigned short>
 *             first - the slot, second the speed in MHz.
 */
std::pair<unsigned short, unsigned short>
parseMspsOption(const char* opt)
{
    std::string sopt(opt);
    size_t idx = sopt.find(':');
    if (idx == std::string::npos) {
        std::stringstream msg;
        msg << "slot/speed option values must be of the form slot:speed "
            << "got: " << opt;
        throw std::invalid_argument(msg.str());
    }
    std::string sslot = sopt.substr(0, idx);
    std::string sspeed = sopt.substr(idx+1);
    
    unsigned short slot = std::stoi(sslot);
    unsigned short speed = std::stoi(sspeed);
    std::pair<unsigned short, unsigned short> result = {slot, speed};
    return result;
}


/**
 * makeSlotVector
 *    Given the slot information map, crate a vector of slots.
 *  @param slotInfo - slot information vetor
 *  @param crate    - Crate settings.
 *  @return std::vector<unsigned short>
 */
std::vector<unsigned short>
makeSlotVector(
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotInfo    
)
{
    std::vector<unsigned short> result;
    for (auto p = slotInfo.begin(); p != slotInfo.end(); p++) {
        result.push_back(p->first);
    }
    return result;
}
std::vector<unsigned short> makeSlotVector(const DDAS::Crate& crate)
{
    std::vector<unsigned short> result;
    
    const std::vector<DDAS::Slot>& slots(crate.s_slots);
    for (int i =0; i < slots.size(); i++) {
        result.push_back(slots[i].s_slotNum);
    }
    
    return result;
}