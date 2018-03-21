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

/** @file:  CS800FragmentHandler.h
 *  @brief: Define the class that handles S800 fragments.
 */

#ifndef CS800FRAGMENTHANDLER_H
#define CS800FRAGMENTHANDLER_H

#include "CFragmentHandler.h"

#include <map>
#include <string>
#include <cstdint>

/**
 * CS800FragmentHandler -- see https://wikihost.nscl.msu.edu/S800Doc/doku.php?id=event_filter
 *                         for a description of the data in the body of this
 *                         fragment.
 */
class CS800FragmentHandler : public CFragmentHandler
{
private:
    /*  This map will be a map from subpacket id to textual names of the packets
     *  it'll get filled in at construction time.
     */
    std::map<std::uint16_t, std::string> m_typeStrings;
    
    /*
     *  A packet looks like this:
     */
    typedef struct _packet {
        std::uint16_t s_length;
        std::uint16_t s_type;
        std::uint16_t s_body[];
    } Packet, *pPacket;
public:
    CS800FragmentHandler();
    void operator()(FragmentInfo& frag);
private:
    void timestampPacket(std::uint16_t* p);
    void eventNoPacket(std::uint16_t* p);
    void crdcPacket(std::uint16_t* p);
};

#endif