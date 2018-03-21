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

/** @file:  CCAESARFragmentHandler.cpp
 *  @brief: Implement the CCAESARFragmentHandler class.
 */

#include "CCAESARFragmentHandler.h"
#include "FragmentIndex.h"
#include "PacketUtils.h"

#include <iostream>

/**
 * constructor
 *   Fill in the m_packetNames member.
 */
CCAESARFragmentHandler::CCAESARFragmentHandler()
{
    m_packetNames[std::uint16_t(0x2301)] = "Energy FERAS";
    m_packetNames[std::uint16_t(0x2302)] = "Time FERAS";
    m_packetNames[std::uint16_t(0x2303)] = "Timestamp";
    m_packetNames[std::uint16_t(0x23ff)] = "End/error flag";
    
}
/**
 * operator()
 *    This is called when a CAESAR packet is encountered in the event.
 *    - Output the fact that this is a CAESAR packet
 *    - Output the the two words in the packet header.
 *    - Iterate over the subpackets outputting their sizes and types.
 *    -  For the timestamp, reconstruct the full timestamp and output it
 *    
 * @param frag - reference to the fragment information from the fragment iterator.
 */
void
CCAESARFragmentHandler::operator()(FragmentInfo& frag)
{
    std::cout << "==== CEASAR Fragment: \n";
    std::cout << "   Timestamp: " << frag.s_timestamp << std::endl;
    std::cout << "   Sourceid:  " << std::hex << frag.s_sourceId << std::dec << "(hex)\n";
    
    /*
     * Now we pull out the body.  The body should look like (all items 16 bits
     * unless otherwise specifiec)
     *    +-------------------------------+
     *    | size of fragment (32 bits)    |
     *    +-------------------------------+
     *    | Size of CAESAR pkt            |
     *    +-------------------------------+
     *    | 0x2300                        |
     *    +-------------------------------+
     *    |   0x0002                      |   \
     *    +-------------------------------+    > Don't know what these words are
     *    |   0x0010                      |   /
     *    +-------------------------------+
     *    |   Sub packets....             |
     *
     */
    std::uint16_t* p = frag.s_itembody;
    p += sizeof(std::uint32_t)/sizeof(std::uint16_t);   // Skip the 32 bits of frag size.
    
    std::uint16_t remaining = *p++;                     // Size of the CAESAR pkt.
    std::uint16_t caesarType = *p++;                    // Overall packet type.
    remaining -= 2;                                     // Size is self inclusive.
    /**
     * If the overall packet type isn't a 0x2300, then we've got a problem and
     * we don't know how to decode this.  Complain and return.
     */
    if (caesarType != 0x2300) {
        std::cout << " *** Error - CAESAR packet id should be 0x2300 but was: "
            << std::hex << caesarType << std::dec << std::endl;
        return;                                      
    }
    
    /*  Skip over the header words to point at the first subpacket: */
    
    p += 2;
    remaining -= 2;
    
    /*  This loop assumes there's at least one subpacket (this is true as
     *  there must be a timestamp).
     */
    
    while (p) {
        std::uint16_t subPktSize = *p;
        std::uint16_t subPktType = p[1];
        
        // Process this packet.
        
        std::string subPacketTypeName =
            PacketUtils::packetName(subPktType, m_packetNames, "Unknown");
        
        std::cout << "   Subpacket for " << subPacketTypeName << " " << subPktSize << " words long\n";
        
        //  If we have a timetamp packet construct and output the timestamp:
        
        if (subPktType == 0x2303) {
            std::uint64_t timestamp(0);
            std::uint64_t part;
            std::uint16_t* pTimestamp = &(p[2]);
            for (int i = 0; i < 4; i++) {
                part = *pTimestamp;
                timestamp  |= (part << (16*i));
                
                pTimestamp++;
            }
            std::cout << "   Timestamp value from packet: " << timestamp << std::endl;
            
        }
        
        // Point to next packet or possbily done.
        
        p = PacketUtils::nextPacket(remaining, p);
    }
    
}