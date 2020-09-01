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

/** @file:  CFullEventEditor.cpp
 *  @brief: Implement the full event editor class.
 */

#include "CFullEventEditor.h"
#include "CFanoutClientTransport.h"
#include "CSender.h"

#include <stdlib.h>
#include <new>
#include <stdexcept>


/**
 * constructor
 *    Constructs   a new object:
 *
 *  @param fainin   - the fanin transport that is giving us data.
 *  @param sink     - Where we send our resulting events.
 *  @param clientId - our fanout id.
 *  @param pEditor  - Pointer to our actual editing object.
 */
CFullEventEditor::CFullEventEditor(
    CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
    Editor* pEditor
) : CBuiltItemWorker(fanin, sink, clientId),
    m_pEditor(pEditor),
    m_nId(clientId),
    m_pIovecs(nullptr),
    m_nIoVecCapacity(0)
{}
/**
 *  destructor
 *     The iovectors we created must be released:
 */
CFullEventEditor::~CFullEventEditor()
{
    free(m_pIovecs);
}
/**
 * process
 *    Called when a block of data is ready to be processed.  We produce an output
 *    block of data that consist of:
 *    -   The client id (uint32_t).
 *    =   A list of timstamp ring items pairs.  Where each ring item:
 *      * Is assumed to have a body header
 *      * Is assumed to have come from an event builder.
 *
 *  We iterate over the ring items.   Non physics ring items are passed without
 *  modification.  Physics ring items are edited.  After editing we
 *  adjust the:
 *     -  Ring item header size.
 *     -  Body header size.
 *     -  the uint32_t size of the event bodyt returned by each editor.
 * @param pData - points to the data to process.
 * @param nBytes - is the number of bytes in the message.  If this is zero,
 *                 this is an end of data indicator.
 */
void
CFullEventEditor::process(void* pData, size_t nBytes)
{
    if (nBytes) {
        
        std::vector<SegmentDescriptor> outputSegments;   // Describes the full output.
        SegmentDescriptor id(sizeof(m_nId), &m_nId);
        outputSegments.push_back(id);                    // preface output with id.
        
        size_t nEvents = countItems(pData, nBytes);   // # of events.
        uint8_t* p     = static_cast<uint8_t*>(pData);
        
        // Iterate over the events.  Note that we need to advance our pointer
        // to the next event _before_ any editing is done that might
        // modify the event size is applied.
        
        
        for (int i =0; i < nEvents; i++) {
            pEventHeader pEH = reinterpret_cast<pEventHeader>(p);
            uint8_t*     pEventBytes = p;
            SegmentDescriptor ts(sizeof(uint64_t), &(pEH->s_timestamp));
            outputSegments.push_back(ts);    // preceding timestamp.
            
            pRingItemHeader pRHeader = &(pEH->s_ringHeader);
            
            p = reinterpret_cast<uint8_t*>(pRHeader);          // Before this can get changed.
            p += pRHeader->s_size;
            
            if (pRHeader->s_type == PHYSICS_EVENT) {
                std::vector<SegmentDescriptor> newBody = editEvent(pRHeader);
                
                // We have to edit the ring item header and the event body size
                // fields to represent the new event size _and_ produce a
                // descriptor for them -- allow for the fact a prior process
                // may have modified the body header size!!
                
                size_t newSize    = getEventBodySize(newBody);
                size_t headerSize =
                    sizeof(RingItemHeader) + pEH->s_bodyHeader.s_size +
                    sizeof(uint32_t);
                pEventBytes += headerSize + sizeof(uint64_t) - sizeof(uint32_t);  // points to size uint32_t.
                uint32_t* pBodySize = reinterpret_cast<uint32_t*>(pEventBytes);
                
                *pBodySize = newSize + sizeof(uint32_t);
                pRHeader->s_size = newSize + headerSize;
                
                
                SegmentDescriptor newHeader(headerSize, pRHeader);
                outputSegments.push_back(newHeader);
                outputSegments.insert(
                    outputSegments.end(), newBody.begin(), newBody.end()
                );
                
            } else {
                // Pass the event through:
                
                SegmentDescriptor s(pRHeader->s_size, pRHeader);
                outputSegments.push_back(s);
            }
        }
        outputData(outputSegments);
        freeData(outputSegments);
        
        
    } else {
        // pass on the end of event indicator in this case just our id:
        
        getSink()->sendMessage(&m_nId, sizeof(m_nId));
        
    }
}
/**
 * outputData
 *    - marshall a vector of output descriptors into a soup of io vects.
 *    - writes the resulting soup to the sink.
 *    @note the output vector soup storage is re-used from event to event if possible.
 *
 * @param outputData - SegmentDescriptors that say what to write.
 * @note We don't free any output descriptor dynamic data at this stage.
 */
void
CFullEventEditor::outputData(std::vector<SegmentDescriptor>& segs)
{
    // Marshall the segments:
    
    iovec* pVectors = getIoVectors(segs.size());
    for (int i = 0; i < segs.size(); i++) {
        pVectors[i] = segs[i].s_description;
    }
    getSink()->sendMessage(pVectors, segs.size());
}
/**
 * freeData
 *    For each item in the descriptor vector we're passed, we call the
 *    if the item is marked dynamic, the editor is asked to free the data
 *    associated with that descriptor
 *
 *    @param segs - descriptor segments affected.
 */
void
CFullEventEditor::freeData(std::vector<SegmentDescriptor>& segs)
{
    for (auto& item : segs) {
        if (item.s_dynamic) m_pEditor->free(item.s_description);
    }
}
/**
 * getIoVectors
 *    Returns a pointer to an iovec soup with sufficient entries to
 *    contain the requested number of entries.
 *
 *  @param n - number of entries needed.
 *  @return iovec* - pointer to the io vector soup.
 */
iovec*
CFullEventEditor::getIoVectors(size_t n)
{
    if (n > m_nIoVecCapacity) {
        free(m_pIovecs);
        m_pIovecs = static_cast<iovec*>(malloc(n * sizeof(iovec)));
        if (!m_pIovecs) {
            throw std::bad_alloc();
        }
        m_nIoVecCapacity = n;
    }
    return m_pIovecs;
}
/**
 * getEventBodySize
 *    Given a vector of descriptors determines how large a chunk of data is
 *    described by that vector.
 * @param segs - references the vector of segments.
 * @return size_t
 */
size_t
CFullEventEditor::getEventBodySize(const std::vector<SegmentDescriptor>& segs)
{
    size_t result(0);
    for (auto& s : segs) {
        result += s.s_description.iov_len;
    }
    return result;
}

/**
 * editEvent
 *    Requests the editor to edit the event.   The assumption is that the
 *    event is an event built event and, therefore has a uint32_t at the front
 *    of the body that:
 *    -  Is a self inclusive count of the number of bytes in the body.
 *    -  Will be adjusted  by the caller to reflect the new body size.
 *
 *   @param pItem - points to the event.  The Event is assumed to have a body
 *                  header.
 *  @return std::vector(SegmentDescriptor) - describing the new event body.
 */
std::vector<CFullEventEditor::SegmentDescriptor>
CFullEventEditor::editEvent(pRingItemHeader pItem)
{
    // Figure out where the body starts.. it's after the ring item header
    // which is fixed sized and after the body header which has a size
    // field that's non-zero if there's a body header.
    
    pItem++;              // Points to the body  header:
    pBodyHeader pBHeader = reinterpret_cast<pBodyHeader>(pItem);
    if(!pBHeader->s_size) {
        throw std::invalid_argument("Go an event without a body header!!");
    }
    uint8_t* p = reinterpret_cast<uint8_t*>(pBHeader);
    p += pBHeader->s_size;
    
    return (*m_pEditor)(p);
}
