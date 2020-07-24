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

/** @file:  PacketUtils.h
 *  @brief: define utilities for word counted packets.
 */
#ifndef SCRIPTED_PACKETUTILS_H
#define SCRIPTED_PACKETUTILS_H
#include <stdint.h>

namespace PacketUtil {
    uint16_t* startPacket(uint16_t* p, uint16_t id);
    int      endPacket(uint16_t* pkt, uint16_t* pktend);
    uint16_t* startPacket16(uint16_t* p, uint16_t id);
    int      endPacket16(uint16_t* pkt, uint16_t* pktend);
};

#endif