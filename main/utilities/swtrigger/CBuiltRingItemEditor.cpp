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
#include <iostream>

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
m_nBlocks(0), m_pIoVectors(nullptr), m_nIoVectorCount(0), m_nUsedIoVectors(0)
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
            // p is really a pEventHeader.
            pEventHeader pEH = static_cast<pEventHeader>(p);
            BodySegment ts(sizeof(uint64_t), pEH);
            outputSegments.push_back(ts);
            pRingItemHeader pH = &(pEH->s_ringHeader);
            
            p = nextItem(p);    // Before the ring item gets edite.
            
            if (pH->s_type == PHYSICS_EVENT) {
                std::vector<BodySegment> segs =  editItem(pH);
                
                // An empty result means something bad enough
                // happened we need to kill off the event.
                
                if (segs.size() == 0) {
                    // To finish getting the event retracted we need
                    // to remove the timestamp we've already put in the
                    // output segments - the last item:
                    
                    outputSegments.pop_back();    // timestamp -- not dynamic.
                    
                } else {
                    outputSegments.insert(
                        outputSegments.end(), segs.begin(), segs.end()
                    );                    
                }
                

            } else {
                // pass non physics type along:
                
                BodySegment s(pH->s_size, pH);
                outputSegments.push_back(s);
            }
            
            
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
    if (!hasBodyHeader(pRItem)) {
        throw std::invalid_argument("Physics ring item is missing a body header!");
    }
    
    uint32_t* pbhdr = static_cast<uint32_t*>(bodyHeader(pRItem));
    uint32_t  bodyHeaderSize = *pbhdr;

    
    uint8_t*  pBodyB = reinterpret_cast<uint8_t*>(pbhdr);
    pBodyB += bodyHeaderSize;
    uint32_t* pBody = reinterpret_cast<uint32_t*>(pBodyB);
    


    size_t nBytes = *pBody;
    BodySegment hdr(
        sizeof(RingItemHeader) + bodyHeaderSize + sizeof(uint32_t),
        pItem
    );
    result.push_back(hdr);
    
    // iterate over the fragments editing them:
    
    nBytes -= sizeof(uint32_t);
    void*  p = firstFragment(pBody);
    while(nBytes) {
        EVB::pFlatFragment pFrag = static_cast<EVB::pFlatFragment>(p);
        EVB::pFragmentHeader pFragHeader = reinterpret_cast<EVB::pFragmentHeader>(pFrag);  // Fragment header only.
        pRingItemHeader      pfRitemHdr  = reinterpret_cast<pRingItemHeader>(pFragHeader+1); // Ring item header follows.
        std::vector<BodySegment> fragSegs;
        
        // There's something seriously wrong with the event if:
        // 1. The ring item length is > nBytes remaining.
        // 2. The type is not PHYSICS_EVENT
        //  In that case we stop processing the event and break out of the
        //  loop.
        
        if (((sizeof(EVB::FragmentHeader) + pfRitemHdr->s_size) > nBytes) || (pfRitemHdr->s_type != PHYSICS_EVENT)) {
            std::cerr << "The ring item header of a fragment either goes out of\n";
            std::cerr << "event bounds or the type is no PHYSICS_EVENT\n";
            std::cerr << "Going on to the next event, the fragments so far will be output\n";
            std::cerr << "The remainder of the event will not be output\n";
            
            std::cerr << "Item size: " << sizeof(EVB::FragmentHeader) + pfRitemHdr->s_size <<"  bytes left: " << nBytes << std::endl;
            std::cerr << "Item type: " << pfRitemHdr->s_type << " Should be: " << PHYSICS_EVENT << std::endl;
            
            // This is safe because the caller uses the fragment descriptors so far
            // (which are valid) to size the event and the body size.
            
            break;      
        }
        // Another failure we've seen is a corrupt body header.  Specifically,
        // the body header looks like it's a bit of trace which, in turn
        // pushes us off the end of the datablock.
        // This code:
        // 1. Assumes there's a body header.
        // 2. Gets the size of the body header
        // 3. If it pushes us off the end of the data block,
        //    whines and aborts this ring item:
        
        pRingItem pfragRingItem = reinterpret_cast<pRingItem>(pfRitemHdr);
        uint32_t   fBodyHeaderSize = *(static_cast<uint32_t*>(bodyHeader(pfragRingItem)));
        
        // nbytes includes the ring item header and fragment header that precede
        // the body header:
        
        if (fBodyHeaderSize +sizeof(RingItemHeader) + sizeof(EVB::FragmentHeader) > nBytes) {
            std::cerr << " I got a body header size of " << fBodyHeaderSize
                << " which, with the ring item and frag headers would push  me off the end of the data "
                << nBytes << std::endl;
            std::cerr << " Skipping this event\n";
            result.clear();
            return result;                    // empty should be gone.
        }
        
        try {
            fragSegs = editFragment(pFrag);
            result.insert(result.end(), fragSegs.begin(), fragSegs.end());
        } catch (std::exception& e) {
            std::cerr << " User's fragment editing code threw an exception: \n";
            std::cerr << e.what() << std::endl;
            std::cerr << std::endl << "This fragment will be removed from the output\n";
            fragSegs.clear();               // Probably this is the case but...
        } catch (...) {
            std::cerr << " User's fragment editing code threw an unanticipated exception type: \n";
            std::cerr << std::endl << "This fragment will be removed from the output\n";
            fragSegs.clear();               // Probably this is the case but...
        }
        
        // This has to come before the size adjustment.
        
        p = nextFragment(p);
        nBytes -= (sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size);
                   
	// The fragment header and ring item header sizes must be adjusted.
	// There's already a segment to write them to the output we just
	// need to modify the data in place.

        uint32_t fragmentSize = countBytes(fragSegs); // Includes the ring item hdr and frag hdr.
        if (fragmentSize) {
            uint32_t finalFragBodySize = fragmentSize - sizeof(EVB::FragmentHeader);
            pFragHeader->s_size = finalFragBodySize;
            pfRitemHdr->s_size  = finalFragBodySize;
        }
	
       
    }
    
    // Fix up the full size and the event builder's size.
    
    size_t finalBytes = countBytes(result);
    pItem->s_size = finalBytes;
    *pBody       =
        finalBytes - (sizeof(RingItemHeader) + bodyHeaderSize);
    
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

    // Get pointers to each of the chunks of data - ring item header.
    // body header, body.  We already have the fragment pointer.
    // Note that the body header may have an extension.  We can
    // already assume there is a body header.

    pRingItemHeader pRItemHdr = reinterpret_cast<pRingItemHeader>(pFrag+1);
    pBodyHeader     pBHeader  = reinterpret_cast<pBodyHeader>(pRItemHdr+1);
    uint32_t bhdrSize = pBHeader->s_size;
    uint8_t* pBody    = reinterpret_cast<uint8_t*>(pBHeader) + bhdrSize;
    uint32_t bodySize = pRItemHdr->s_size - bhdrSize - sizeof(pRItemHdr);
    
    // Make the first body segment which includes up through the
    // fragment's body header.

    BodySegment hdr( // Wrong if there's an extension.
        sizeof(EVB::FragmentHeader) + sizeof(RingItemHeader) +
        bhdrSize, pFrag
    );
    result.push_back(hdr);
    
    // Produce the pointers needed by the user code and invoke it.
    std::vector<BodySegment> segs =
        (*m_pEditor)(pRItemHdr, pBHeader, bodySize, pBody);
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
