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

/** @file:  CNullTransport.cpp
 *  @brief: implements the class.
 */

#include "CNullTransport.h"


void CNullTransport::recv(void** ppData, size_t& size)
{
    void*   pJunk(nullptr);
    *ppData = pJunk;
    size    = 0;
}

void CNullTransport::send(iovec* parts, size_t numParts)
{
    
}