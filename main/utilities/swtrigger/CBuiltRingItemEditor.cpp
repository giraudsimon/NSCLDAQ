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

/** @file:  CBuiltRingItemEditor.cpp
 *  @brief: Implement the CBuiltRingItemEditor class.
 */

#include "CBuiltRingItemEditor.h"
#include <fragment.h>
#include <DataFormat.h>
#include <CSender.h>


#include <stdlib.h>
#include <new>
#include <stdexcept>

/**
 * constructor
 *    @param fanin - client of the fanout transport.
 *    @param sink  - Where we send our data in the end.
 *    @param clientId - Our client id - messages have that for the sorter.
 *    @param pEditor - pointer to the ring item editing object.
 */
CBuiltRingItemEditor::CBuiltRingItemEditor(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    BodyEditor* pEditor
) : CBuiltItemWorker(fanin, sink, clientId), m_nId(clientId), m_pEditor(pEditor),
m_nBlocks(0)
{}
/**
 * destructor
 */
CBuiltRingItemEditor::~CBuiltRingItemEditor()
{
    free(m_pIoVectors);
}

/**
 * process
 *    Process a work item, or end of work items indicator:
 *
 *  @param pData  - Pointer to the data which are just a soup of ring items.
 *  @param nBytes - number of bytes in the work item.
 */
void
CBuiltRingItemEditor::process(void* pData, size_t nBytes)
{
    if (nBytes) {
        //
        // Because we can't a-priori size the data,
        // we'll put out discriptors (including the ones from the user code)
        // into this vector.  After all ring items are processed we pull out the
        // iovecs and do the write.
        
        std::vector<BodySegment>  outputSegments;
        
        // The first item always points to our id:
        
        BodySegment id(sizeof(uint32_t), &m_nId);
        outputSegments.push_back(id);
        
        // Now we iterate over the ring items.  Non physics ones just
        // get passed through.  Physics ones need to be handed to the
        // user code for body editing.
        
        size_t nEvents = countItems(pData, nBytes);
        void* p = pData;
        for (int i = 0; i < nEvents; i++ ) {
            // p is really a ring item pointer. what we do depends on the type:
            
            pRingItemHeader pH = static_cast<pRingItemHeader>(p);
            if (pH->s_type == PHYSICS_EVENT) {
                std::vector<BodySegment> segs =  editItem(pH);
                outputSegments.insert(
                    outputSegments.end(), segs.begin(), segs.end()
                );
            } else {
                // pass non physics type along:
                
                BodySegment s(pH->s_size, pH);
                outputSegments.push_back(s);
            }
            
            p = nextItem(p);
        }
        outputData(outputSegments);
        freeData(outputSegments);
        
    } else {
        // Have an end data indicator.
        
        getSink()->sendMessage(&m_nId, sizeof(m_nId));
    }
}

/**
 * outputData
 *    Ensure that we have enough io vectors.
 *    Fill them in from the output segments.
 *    do the send.
 *
 * @param segs  - Body Segments to send.
 */
void
CBuiltRingItemEditor::outputData(
    std::vector<BodySegment>& segs
)
{
    resizeIoVecs(segs.size());        // Make sure we have enough iovecs.
    
    // Fill the io vectors:
    
    m_nUsedIoVectors = segs.size();
    for (
        int i = 0; i < segs.size();
        i++
    )  {
        m_pIoVectors[i].iov_base = segs[i].s_description.iov_base;
        m_pIoVectors[i].iov_len   = segs[i].s_description.iov_len;
    }
    
    // Send the data.
    
    getSink()->sendMessage(m_pIoVectors, m_nUsedIoVectors);
}
/**
 * freeData
 *    Call the editor's free method for each iovec that's marked as
 *    dynamic:
 *
 * @param segs - references the segments:
 */
void
CBuiltRingItemEditor::freeData(std::vector<BodySegment>& segs)
{
    for (int i = 0; i < segs.size(); i++) {
        if (segs[i].s_isDynamic) {
            m_pEditor->free((segs[i].s_description));
        }
    }
}
/**
 * resizeIoVecs
 *    If necessary resize the m_pIoVectors storage.
 *  @param n - number of vectors needed.
 */
void
CBuiltRingItemEditor::resizeIoVecs(size_t n)
{
    if (n > m_nIoVectorCount) {
        free(m_pIoVectors);
        m_pIoVectors = static_cast<iovec*>(malloc(n * sizeof(iovec)));
        if (!m_pIoVectors) {
            throw std::bad_alloc();
        }
        m_nIoVectorCount = n;
    }
}
/**
 * editItem
 *    Given a pointer to a ring item containing single level
 *    event built data:
 *    - Holds a pointer to the ring item header, the body header and
 *      the size.
 *    - Iterates over the fragments editing them.
 *    - Fixes the sizes in the ring item header and in the
 *      event builder size at the beginning of the body.
 * @note We assume there area body headers.
 *
 * @param pItem - pointer to the event.
 * @return std::vector<BodySegment> - Vector that describes the
 *     full output ring item.
 */
std::vector<CBuiltRingItemEditor::BodySegment>
CBuiltRingItemEditor::editItem(pRingItemHeader pItem)
{
    std::vector<BodySegment> result;
    
    // Make the first extentand a pointer to the size of the
    // event body as seen by the event builder.
    
    pRingItem pRItem = reinterpret_cast<pRingItem>(pItem);
    if (pRItem->s_body.u_noBodyHeader.s_mbz == 0) {
        throw std::invalid_argument("Physics ring item is missing a body header!");
    }
    
    uint32_t* pBody =
        reinterpret_cast<uint32_t*>(pRItem->s_body.u_hasBodyHeader.s_body);
    size_t nBytes = *pBody;
    BodySegment hdr(
        sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(uint32_t),
        pItem
    );
    result.push_back(hdr);
    
    // iterate over the fragments editing them:
    
    nBytes -= sizeof(uint32_t);
    void*  p = firstFragment(pBody);
    while(nBytes) {
        EVB::pFlatFragment pFrag = static_cast<EVB::pFlatFragment>(p);
        std::vector<BodySegment> fragSegs = editFragment(pFrag);
        result.insert(result.end(), fragSegs.begin(), fragSegs.end());
        
        // This has to come before the size adjustment.
        
        p = nextFragment(p);
        nBytes -= (sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size);
                   
        // Adjust the sizes int the fragment header and ring item header
        
        size_t finalSize = countBytes(fragSegs);
        finalSize -= sizeof(EVB::FragmentHeader);   // Ring item size.
        pFrag->s_header.s_size = finalSize;         // is the payload size.
        pRingItemHeader pEditedItem =
            reinterpret_cast<pRingItemHeader>(pFrag+1);
        pEditedItem->s_size = finalSize;            
        
    }
    
    // Fix up the full size and the event builder's size.
    
    size_t finalBytes = countBytes(result);
    pItem->s_size = finalBytes;
    *pBody       =
        finalBytes - (sizeof(RingItemHeader) + sizeof(BodyHeader));
    
    //  Done, return the result.
    
    return result;
}

/**
 * editFragment
 *    Given a pointer to a fragment, edit the fragment.
 *    The caller is responsible for modifying the size fields in both
 *    the fragment header and the ring item header that follows it.
 *
 *    We'll produce the BodySegment for the stuff in front of the
 *    fragment body (fragment header, ring item header, body headers).
 *
 * @param pFrag - pointer to tghe fragment.
 * @return std::vectory<BodySegment> Descriptors for the edited fragment.
 */
std::vector<CBuiltRingItemEditor::BodySegment>
CBuiltRingItemEditor::editFragment(EVB::pFlatFragment pFrag)
{
    std::vector<BodySegment> result;
    
    // Make the first body segment which includes up through the
    // fragment's body header.
    
    BodySegment hdr(
        sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) +
        sizeof(BodyHeader), pFrag->s_body
    );
    result.push_back(hdr);
    
    // Produce the pointers needed by the user code and invoke it.
    
    pRingItemHeader pRHeader =
        reinterpret_cast<pRingItemHeader>(pFrag->s_body);
    pBodyHeader pBHeader = reinterpret_cast<pBodyHeader>(pRHeader+1);
    size_t bodySize =
        pRHeader->s_size - sizeof(RingItemHeader) - sizeof(BodyHeader);
    void* pBody = pBHeader+1;
    
    std::vector<BodySegment> segs =
        (*m_pEditor)(pRHeader, pBHeader, bodySize, pBody);
    result.insert(result.end(), segs.begin(), segs.end());
    
    // Return the full set of body segment descriptors.
    
    
    return result;
}
/**
 * countBytes
 *    Count the number of bytes described by a vector of body segment
 *    descriptors
 *
 * @param segs - the segments.
 * @return size_t
 */
size_t
CBuiltRingItemEditor::countBytes(const std::vector<BodySegment>& segs)
{
    size_t result(0);
    for (int i =0; i < segs.size(); i++) {
        result += segs[i].s_description.iov_len;
    }
    return result;
}