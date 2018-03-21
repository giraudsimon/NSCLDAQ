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

/** @file:  PacketUtils.cpp
 *  @brief: Implement methods in the packet utilities namespace.
 */
#include "PacketUtils.h"

/**
 * nextPacket
 *    Given a pointer to a packet, returns a pointer to the next packet,
 *    or a null if there are no more packets.  See
 *    the internal Packet struct in the header.  See also
 *   https://wikihost.nscl.msu.edu/S800Doc/doku.php?id=event_filter
 *
 * @param[inout] remainingWords -number of words remaining in the event
 * @param here           -Pointer t the front of the packet.
 *
 * Note that remainingWords should be the number of words left as of the
 * packet we're about to skip. It will be updated with the new remainingWords.
 * The here pointer must be pointing to the size of the packet we're skipping.
 */
std::uint16_t*
PacketUtils::nextPacket(std::uint16_t& remainingWords, std::uint16_t* p)
{
    if (remainingWords) {
        std::uint16_t packetSize = *p;
        remainingWords -= packetSize;
        if (remainingWords) {               // There's more data:
            return p + packetSize;    
        } else {
            return nullptr;                 // p pointed to the last packet.
        }
        
    } else {
        return nullptr;                    // Called with nothing left.
    }
}

/**
 * packetName
 *    Return the name of a packet given its type:
 *
 *  @param type -packet type
 *  @param typeMap - map between ids and names.
 *  @param defaultName - name to use if there's no match in typemap.
 *  @return std::string - packet type name.
 */
std::string
PacketUtils::packetName(
    std::uint16_t type,
    const std::map<std::uint16_t, std::string>& typeMap, const char* defaultName
)
{
    std::map<std::uint16_t, std::string>::const_iterator p = typeMap.find(type);
    return (p == typeMap.end()) ? std::string(defaultName) : p->second;
}
