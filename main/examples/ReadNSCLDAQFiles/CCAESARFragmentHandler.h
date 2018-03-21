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

/** @file:  CCaesarFragmentHandler.h
 *  @brief: Define the handler for CAESAR fragments.
 *
 */

#ifndef CCAESARFRAGMENTHANDLER_H
#define CCAESARFRAGMENTHANDLER_H
#include "CFragmentHandler.h"

#include <map>
#include <string>
#include <cstdint>

struct FragmentInfo;

/**
 *  CCaesarFragmentHandler - handles caesar event fragments.  for the most
 *                           part we're just going to callout the sizes and
 *                           packet types of the constituent packets.
 */


class CCAESARFragmentHandler : public CFragmentHandler
{
private:
    /**
     * maps packet ids to packet name strings.
     */
    std::map<std::uint16_t, std::string> m_packetNames;
public:
    CCAESARFragmentHandler();
    
    void operator()(FragmentInfo& frag);
private:
    void timestampPacket(std::uint16_t* pPacket);
};

#endif