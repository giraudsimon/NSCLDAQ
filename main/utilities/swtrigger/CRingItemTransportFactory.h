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

/** @file:  CRingItemTransportFactory.
 *  @brief: Factory to create ring item transports from URI's and modes.
 */
#ifndef CRINGITEMTRANSPORTFACTORY_H
#define CRINGITEMTRANSPORTFACTORY_H
#include <CRingBuffer.h>

class CRingItemTransport;

/**
 * @class CRingItemTransportFactory
 *     Given a URI and an access mode (reader/writer),
 *     produces an appropriatering buffer transport object.
 *     The object is produced dynamically and must be deleted by the caller
 *     at some point.
 *
 *     Rules for creation:
 *
 *     - If the protocol is tcp:  a CRingBufferTransport will be created but:
 *       *  if mode is CRingBuffer::producer, the host must be "localhost"
 *       *  if mode is CRingBuffer::consumer, the remote ring item access
 *          package is used to create a CRingBuffer on which a
 *          CRingBufferChunkAccess object is created.
 *     - If the protocol is file:  A CRingIteFileTransport will be created
 *       connected to the specified file.
 *
 */
class CRingItemTransportFactory
{
public:
    static CRingItemTransport* createTransport(
        const char* uri, CRingBuffer::ClientMode accessMode
    );
};
#endif