/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file   CRingMarkingWorker.h
* @brief  Worker that marks ring items in a body header extension.
*/
#ifndef CRINGITEMMARKINGWORKER_H
#define CRINGITEMMARKINGWORKER_H
#include "CParallelWorker.h"
#include <stdint.h>
#include <sys/uio.h>
class CRingItem;
/**
 * @class CRingMarkingWorker
 *    This class is intended to allow ring items to be classified according
 *    to some criterion specified by a contained processing object.  The
 *    classificiation is a uint32_t that is appended to the body header
 *    of the ring item which is then passed to the sink.
 *
 *    One use case would be that subsequent processing stages could then
 *    filter out ring items that are not in some set of acceptable
 *    classification(s).
 */
class CRingMarkingWorker : public CParallelWorker
{
public:
    // An instance of a concrete subclass must be provided to
    // do the classification.
    
    class Classifier {
    public:
        virtual uint32_t operator()(CRingItem& item) = 0;
    };
    
private:
    Classifier*   m_pClassifier;
    size_t        m_nItemsProcessed;
public:
    CRingMarkingWorker(
        CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
        Classifier* pClassifier
    );
    virtual ~CRingMarkingWorker() {}
    virtual void process(void* pData, size_t nBytes);
private:
    size_t countItems(const void* pData, size_t nBytes);
    void*  nextItem(const void* pData);
    size_t messageSize(const void* pData);
    size_t   createClassifiedParts(
        iovec* vec, void* pData, uint32_t& classification
    );

};

#endif