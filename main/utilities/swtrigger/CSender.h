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

/** @file:  CSender.h
 *  @brief: Object that sends data somewhere over a transport.
 */
#ifndef CSENDER_H
#define CSENDER_H

#include <stddef.h>
#include <sys/uio.h>

class CTransport;

/**
 * @class CSender - sends data somewhere via a transport.
 */
class CSender {
private:
    CTransport* m_pTransport;
public:
    CSender(CTransport& transport);
    
    void sendMessage(iovec* parts, size_t numParts);  // Multipart
    void sendMessage(void* pBase, size_t nBytes);     // Single part.
    void end();
};

#endif
