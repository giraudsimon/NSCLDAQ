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

/** @file:  CReceiver.h
 *  @brief: Base class for receiving data.
 */
#ifndef CRECEIVER_H
#define CRECEIVER_H
#include <stddef.h>                  // size_t?
class CTransport;

/**
 * @class CReceiver
 *    An abstract  class for receiving data in a message oriented way
 *    over some  transport.
 */
class CReceiver
{
private:
    CTransport*   m_pTransport;
public:
    CReceiver(CTransport& rTransport);
    
    void getMessage(void** ppData, size_t& size);
    CTransport* setTransport(CTransport& rTransport);
    
};
#endif