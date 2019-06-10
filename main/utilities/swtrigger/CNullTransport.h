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

/** @file:  CNullTransport.h
 *  @brief: Sends drop data on the floor, recv's immediately end.
 */
#ifndef CNULLTRANSPORT_H
#define CNULLTRANSPORT_H
#include "CTransport.h"
#include <stddef.h>

class CNullTransport :  public CTransport
{
    void recv(void** ppData, size_t& size);
    void send(iovec* parts, size_t numParts);
};

#endif