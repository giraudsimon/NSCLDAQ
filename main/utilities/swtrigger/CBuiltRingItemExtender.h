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

/** @file:  CBuiltRingItemExtender.h
 *  @brief: Defines a classto extend data in ring items in event built data.
 */
#ifndef CBUILTRINGITEMEXTENDER_H
#define CBUILTRINGITEMEXTENDER_H

#include "CBuiltItemWorker.h"

#include <stdint.h>
#include <sys/uio.h>
#include <DataFormat.h>


/**
 * @class CBuiltRingItemExtender
 *
 *    This class is a worker that takes as input ring items that came from
 *    a single level event builder.  Each ring item in the built event
 *    is passed to a CRingItemExtender object.  That object examines  the
 *    ring item and has the opportunity to provide additional data to
 *    extend that ring item.
 *
 */
class CBuiltRingItemExtender : public CBuiltItemWorker
{
public:
    // An instance of a concrete subbclass of this must be provided to do
    // the actual per ring item work.
    
    class CRingItemExtender {
    public:
            virtual iovec operator()(pRingItem item)  = 0; // Produce extension
            virtual void free(iovec& extension) = 0;             // Free memory associated with extension.
    };
private:
    CRingItemExtender* m_pExtender;    // Concrete extender object.
    uint32_t           m_nId;          // My identifier.
    size_t             m_nBlocks;
    
    iovec*             m_pIoVectors;
    size_t             m_nIoVectorCount;
    size_t             m_nUsedIoVectors;
public:
    CBuiltRingItemExtender(
        CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
        CRingItemExtender* pExtender
    );
    virtual ~CBuiltRingItemExtender();
    
    virtual void process(void* pData, size_t nBytes);
private:
    
    size_t iovecsNeeded(const void* pData, size_t nBytes);
    void   allocateIoVectors(size_t needed);    
};

#endif