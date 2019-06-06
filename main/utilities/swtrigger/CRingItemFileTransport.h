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

/** @file:  CRingItemFileTransport.h
 *  @brief: Get or put ring items to file.
 */
#ifndef CRINGITEMFILETRANSPORT_H
#define CRINGITEMFILETRANSPORT_H

#include "CRingItemTransport.h"
#include <CRingFileBlockReader.h>

namespace io {
    class CBufferedOutput;
}

/**
 * @class CRingITemFileTransport
 *     Transport for ring items to/from files.
 */
class CRingItemFileTransport : public CRingItemTransport
{
private:
    CRingFileBlockReader*      m_pReader;       // Only one of these two
    io::CBufferedOutput*       m_pWriter;       // is non-null.
    
    // Used in reading:
    
    CRingFileBlockReader::pDataDescriptor m_pDescriptor;
    uint8_t*                          m_pCursor;
    size_t                            m_nItemsRemaining;
    
public:
    CRingItemFileTransport(CRingFileBlockReader& reader); // For input.
    CRingItemFileTransport(io::CBufferedOutput& writer);      // For output.
    virtual ~CRingItemFileTransport();
    
    virtual void recv(void** ppData, size_t& size);
    virtual void send(iovec* parts, size_t numParts);
    virtual void end();
    
    // Read utilities:
    
private:
    void nextDescriptor();
    void freeDescriptor();
};


#endif