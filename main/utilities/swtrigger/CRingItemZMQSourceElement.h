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

/** @file:  CRingItemZMQSourceElement.h
 *  @brief: <purpose
 */
#ifndef CRINGITEMZMQSOURCEELEMENT_H
#define CRINGITEMZMQSOURCEELEMENT_H
#include "CDataSourceElement.h"
#include <vector>
#include <stdint.h>

/**
 * @class CRingItemZMQSourceElement
 *    Provides a data source of ring items that fans those items out
 *    to workers via a ZMQ Router.
 */
class CRingItemZMQSourceElement : public CDataSourceElement
{
public:
    typedef struct _Message {
        uint64_t  s_timestamp;
        size_t    s_nBytes;
        void*     s_pData;
    } Message, *pMessage;

private:
    size_t m_nChunkSize;
    uint64_t m_nLastTimestamp;
    
 
    std::vector<Message>   m_chunk;
    
public:
    CRingItemZMQSourceElement(
        const char* ringUri, const char* routerUri, size_t chunkSize=1
    );
    virtual ~CRingItemZMQSourceElement() {}
    virtual void operator()();             // Override b/c process frees memory.
    virtual void process(void* pData, size_t nBytes);
private:
    void sendChunk();
    void clearChunk();
};

#endif