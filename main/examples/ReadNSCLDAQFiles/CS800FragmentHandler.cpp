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

/** @file:  CS800FragmentHandler.cpp
 *  @brief: Implement the fragment handler for s800 data.
 */

#include "CS800FragmentHandler.h"
#include "FragmentIndex.h"
#include "PacketUtils.h"

#include <iostream>


/**
 * constructor
 *    We must stock the subpacket map.   The definitions here are from
 *    the s800 wiki: https://wikihost.nscl.msu.edu/S800Doc/doku.php?id=event_filter
 */
CS800FragmentHandler::CS800FragmentHandler()
{
    m_typeStrings[std::uint16_t(0x5803)] = "Timestamp";
    m_typeStrings[std::uint16_t(0x5804)] = "Event number";
    m_typeStrings[std::uint16_t(0x5801)] = "Trigger";
    m_typeStrings[std::uint16_t(0x5802)] = "Time of flight (Phillips)";
    m_typeStrings[std::uint16_t(0x5810)] = "Scintillator";
    m_typeStrings[std::uint16_t(0x5820)] = "Ion Chamber";
    m_typeStrings[std::uint16_t(0x5840)] = "CRDC";
    m_typeStrings[std::uint16_t(0x58b0)] = "Hodoscope";
    m_typeStrings[std::uint16_t(0x5870)] = "Tracking PPAC";
    m_typeStrings[std::uint16_t(0x58f0)] = "MTDC32";
     
}
/**
 * operator()
 *    Called to process the packet.
 *  We're going to output some overall stuff then iterate over packets
 *  providing at last the size and packet type.  For the following packet types
 *  we'll output more information:
 *    - Timestamp - The timestamp as a unified hex number.
 *    - Event number - The event number.
 *    - CRDC     - The CRDC number.
 *
 *  @param frag - reference to a fragment that describes the s800 event.
 */
void
CS800FragmentHandler::operator()(FragmentInfo& frag)
{
    std::cout << "===== S800 Fragment:\n";
    std::cout << "  Timestsamp: " << frag.s_timestamp << std::endl;
    std::cout << "  sourceid:   " << frag.s_sourceId   << std::endl;
    
    std::uint16_t* p  = frag.s_itembody;         // points to overall size.
    std::uint16_t      nwds = *p++; nwds--;             // Pull out event size.
    nwds             = *p++;
    std::cout << "   Top level s800 packet has " << nwds << " words\n";
    std::uint16_t tag = *p++; nwds -= 2;
    
    if (tag != 0x5800) {
        std::cout << "Expected an s800 tag of 5800 but got " << std::hex << tag << std::dec << std::endl;
        return;                                    // Abort the processing.
    }
    std::cout << "  S800 event format is: " << *p++ << std::endl; nwds--;
    
    // p should now point to the first packet and nwds should be the correct
    // number of words remaining in our event.  We assume there's at least
    // one packet in the event.
    
    while (p) {
        std::uint16_t size = *p;
        tag =  p[1];
        std::string typeName = PacketUtils::packetName(tag, m_typeStrings, "UnKnown");
        std::string tagString = typeName;
        std::cout << typeName
        << std::hex << "(" << tag << ")" << std::dec
        << " packet with " << size << " words\n";
        
        /** This switch dispatches to the packet handlers we decided to do something
         *  with
         */
        
        switch (tag) {
        case 0x5803:                      // timestamp.
            timestampPacket(p);
            break;
        case 0x5804:                      // Event number,.
            eventNoPacket(p);
            break;
        case 0x5840:                      // CRDC.
            crdcPacket(p);
            break;
        // all other types...just skip.
        }
        p = PacketUtils::nextPacket(nwds, p);
    }
    
}

/**
 * timestampPacket - put together the timestamp (little endian) and
 *                   output it:
 *  @param p - pointer to the packet.
 */
void
CS800FragmentHandler::timestampPacket(std::uint16_t* p)
{
    std::uint64_t timestamp(0);
    p += 2;             // Skip packet header.
    
    for (int i =0; i < 4; i++) {
        std::uint64_t part = *p++;
        part = part << (16*i);    // position the field.
        timestamp |= part;
    }
    
    std::cout << "     Timestamp: " <<  timestamp << std::endl;
}
/**
 * eventNoPacket
 *    Process an event number packet - output the event number.
 *
 * @param p  - pointer to the packet.
 */
void
CS800FragmentHandler::eventNoPacket(std::uint16_t* p)
{
    std::uint64_t eventNum(0);
    p += 2;                            // Skip header.
    
    for  (int i =0; i < 3; i++) {      // only 48 bits.
        std::uint64_t part = *p++;
        part   = part << (16 * i);
        eventNum |= part;
    }
    
    std::cout << "    Event number: " << eventNum << std::endl;
}

/**
 * crdcPacket
 *    process a crdc packet.  Just let the world know which CRDC this is.
 *  @param p - points to the packet.
 */
void
CS800FragmentHandler::crdcPacket(std::uint16_t* p)
{
    p += 2;            // point at the crd number:
    
    std::cout <<      " Data from CRDC " << *p << std::endl;
}