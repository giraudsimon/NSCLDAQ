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

/** @file:  CFanoutTransport.h
 *  @brief: Abstract base class of a tranpsort to fanout data to workers.
 */
#ifndef CFANOUTTRANSPORT_H
#define CFANOUTTRANSPORT_H
#include "CTransport.h"

/**
 * @class CFanoutTransport
 *     Abstract base class for fanout transports.  Fanout transports accept
 *     data from some source and fan it out for parallel processing by
 *     workers.  The thing that distinguishes fanout transports from others
 *     is the need to tell _all_ clients there's no more data.
 *
 *     Concrete implementations often do this by having clients register their
 *     presence and using a pull protocol which allows transports to
 *     handle requests for data when no more exists with a special end of data
 *     message.
 */
class  CFanoutTransport
{
public:
    virtual void end() = 0;                     // Indicate no more data available.
};

#endif