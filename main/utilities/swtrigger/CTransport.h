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

/** @file:  CTransport.h
 *  @brief: Abstract base class for data transports.
 */
#ifndef CTRANSPORT_H
#define CTRANSPORT_H
#include <stddef.h>                     // size_t?
/**
 * @class CTransport
 *    Defines the interface for classes that transport data around the
 *    software trigger based system.
 */
class CTransport
{
public:
    virtual ~CTransport() {}             // Support for destructor chaining.
    void    recv(void** ppData, size_t& size);
};

#endif