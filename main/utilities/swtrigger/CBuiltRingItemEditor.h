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

/** @file:  CBuiltRingItemEditor.h
 *  @brief: Define generic ring item editor for event built fragments.
 */

#ifndef CBUILTRINGITEMEDITOR_H
#define CBUILTRINGITEMEDITOR_H
#include "CBuiltItemWorker.h"

#include <DataFormat.h>
#include <fragment.h>

#include <sys/uio.h>
#include <vector>
#include <stdint.h>
#include <stddef.h>



/**
 * @class CBuiltRingItemEditor
 *    This class provides a mechanism for users to do complete editing of
 *    event fragments in event built data.
 *
 *    The class iterates over the fragments of a ring buffer and
 *    passses each one off to your editor object.  The editor object
 *    returns a description of the new ring item body.   The editor
 *    does the appropriate editing of counts and sets up so that the
 *    edited ring item  body will be output to the next stage of the
 *    program rather than the original item.  One use, for example,
 *    is to remove traces from DDAS Fit data (that's actualy our original)
 *    use case.
 *
 */
class CBuiltRingItemEditor : public CBuiltItemWorker
{
    // Public data type and classes:
public:
    typedef struct _BodySegment {
        iovec   s_description;
        bool    s_isDynamic;
        _BodySegment(size_t nBytes, void* pData, bool isdyn = false) :
            s_isDynamic(isdyn) {
                s_description.iov_len = nBytes;
                s_description.iov_base = pData;
            }
    } BodySegment, *pBodySegment;
    class BodyEditor {
    public:
        virtual std::vector<BodySegment> operator()(
            pRingItemHeader pHdr, pBodyHeader hdr, size_t bodySize, void* pBody
        ) = 0;
        virtual void free(iovec& item) = 0;
    };
    
    // Instance data:
private:
    BodyEditor*   m_pEditor;
    uint32_t      m_nId;
    size_t        m_nBlocks;           // In case we want statistics later.
    
    iovec*        m_pIoVectors;
    size_t        m_nIoVectorCount;
    size_t        m_nUsedIoVectors;
    
public:
    CBuiltRingItemEditor(
        CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
        BodyEditor* pEditor
    );
    virtual ~CBuiltRingItemEditor();
    
    virtual void process(void* pData, size_t nBytes);

private:
    
    void outputData(std::vector<BodySegment>& segs);
    void freeData(std::vector<BodySegment>& segs);
    void resizeIoVecs(size_t n);
    
    std::vector<BodySegment> editItem(pRingItemHeader pItem);
    std::vector<BodySegment> editFragment(EVB::pFlatFragment pFrag);
    size_t countBytes(const std::vector<BodySegment>& segs);
    
};


#endif
