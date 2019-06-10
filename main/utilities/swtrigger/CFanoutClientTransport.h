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

/** @file:  CFanoutClientTransport.h
 *  @brief: ABC for recipients of data from a fanout transport.
 */

#ifndef CFANOUTCLIENTTRANSPORT_H
#define CFANOUTCLIENTTRANSPORT_H

#include "CTransport.h"
#include <stdint.h>

/**
 * @class CFanoutClientTransport
 *
 *    Abstract base classs for a recipient of data from a
 *    CFanoutTransport.  Fanout transports require that recipients
 *    of their data pull data from them.  Clients must identify themselves
 *    using a unique uint64_t client id.  The purpose of this base class
 *    is to provide the method by which users of this transport
 *    estblish this client id.
 */
class CFanoutClientTransport : public CTransport
{
public:
    virtual void setId(uint64_t id) = 0;
};

#endif