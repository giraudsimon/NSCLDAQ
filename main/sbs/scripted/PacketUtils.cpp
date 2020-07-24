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

/** @file:  PacketUtils.cpp
 *  @brief: Implement PacketUtils.h
 */
#include "PacketUtils.h"

/**
 * startPacket
 *    Begin a packetized entity. Packet have a 32 bit size
 *    and a 16 bit id.  We hold space for the size
 *    and set the id.
 *
 *  @param p - where the packet tarts.
 *  @param id - the id
 *  @return uint16_t* - pointer to packet payload.
 */
uint16_t*
PacketUtil::startPacket(uint16_t* p, uint16_t id)
{
	p += 2;
  *p++ = id;
	return p;
}
/**
 * endPacket
 *    Compute the size of  a packet in uint16_t's and put it in
 *    the packet header:
 *  @param uint16_t* pkstart - pointer to the start of the packet.
 *  @param uint16_t* p       - pointer off end of packet.
 *  @return packet size in words.
 */
int
PacketUtil::endPacket(uint16_t* pkstart, uint16_t* p)
{
	int subsize =  (p - pkstart);    // # of 16 bit words.
  *pkstart = (subsize & 0xffff);   // Lower 16 bits of size.
	++pkstart;
	*pkstart = (subsize >> 16) & 0xffff;  // Upper 16 bits of size
	return subsize;
}
/**
 * startPacket16
 *    Start a packet with a 16 bit size.. same drill as startPacket.
 */
uint16_t*
PacketUtil::startPacket16(uint16_t* p, uint16_t id)
{
	p++;
	*p++ = id;
	return p;
}
/**
 * endPacket16
 *    end a packet with a 16 bit size.
 */
int
PacketUtil::endPacket16(uint16_t* pkt, uint16_t* pktend)
{
	int size =  (pktend - pkt);
	*pkt  = size;
	return size;
}