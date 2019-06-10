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

/** @file:  CRingItemTransport.h
 *  @brief: Abstrct base class for ring item transports.
 */
#ifndef CRINGITEMTRANSPORT_H
#define CRINGITEMTRANSPORT_H
#include "CTransport.h"

/**
 * @class CRingItemTransport
 *    This is a placeholder class that allows the CRingItemTransportFactory
 *    to return a common base class for ring item transports.
 *    Note that while ring item transports have send/recv methods as required
 *    by the CTransport base class, in fact a connection to a ring item
 *    transport is unidirectional.  This is part of why receivers and
 *    Senders are used rather than transports directly.
 */

class CRingItemTransport : public CTransport
{
    
};

#endif