/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  PacketUtils.h
 *  @brief: Provide some packet handling utilities.
 */

#ifndef PACKETUTILS_H
#define PACKETUTILS_H

#include <cstdint>
#include <map>
#include <string>
/**
 * The PacketUtils namespace will hold any methods that deal with packet
 * structure that are common to all packet handlers
 */
namespace PacketUtils { 
    std::uint16_t* nextPacket(std::uint16_t& nRemaining, std::uint16_t* here);
    std::string    packetName(
        std::uint16_t type, const std::map<std::uint16_t,
        std::string>& typeMap, const char* defaultName);
}


#endif
