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

/** @file:  CFullEventEditor.h
 *  @brief: Allow global editing of event bodies.
 */
#ifndef CFULLEVENTEDITOR_H
#define CFULLEVENTEDITOR_H
#include "CBuiltItemWorker.h"
#include <DataFormat.h>

#include <sys/uio.h>                   // For iovec
#include <vector>
#include <stdint.h>
#include <stddef.h>

class CFanoutTransport;
class CSender;

/**
 * @classs CFullEventEditor
 *   This class supports arbitrary editing of event bodies.  The publi
 *   nested class EventEditor provides an abstract base class for concrete
 *   classes that must do the actual editing.  As usual editing is done by
 *   providing iovec like structures that, when gathered produce the final event
 *   body.   This class will adjust the event's ring item count and the body size
 *   in the first (uint32_t) - the data are assumed to have come from the event
 *   builder.   The user code must, however adjust internal counts like the
 *   fragment headers, body header sizes
 *   and fragment ring item headers as well as e.g. DDAS.
 *   event sizes if stuff is added to them.
 */
class CFullEventEditor : public CBuiltItemWorker
{
public:
    /**  Describes a segment of output data. */
    
    typedef struct _SegmentDescriptor {
        iovec s_description;
        bool  s_dynamic;            // Storage needs deletion.
       _SegmentDescriptor(size_t n, void* p, bool dynamic = false) :
          s_dynamic(dynamic) {s_description.iov_len = n; s_description.iov_base = p;
       }
    } SegmentDescriptor, *pSegmentDescriptor;
    /**
     * @class CFullEventEditor::Editor
     *   ABC for the actual editor used to modify event bodies.
     */
    class Editor {
    public:
        virtual std::vector<SegmentDescriptor> operator()(void* pBody) = 0;
        virtual void free(iovec& desc) = 0;
    };
private:
    Editor*     m_pEditor;
    uint32_t    m_nId;
    iovec*      m_pIovecs;
    size_t      m_nIoVecCapacity;
    
public:
    CFullEventEditor(
        CFanoutClientTransport& fanin, CSender& sink, uint64_t clientId,
        Editor* pEditor
    );
    virtual ~CFullEventEditor();
    virtual void process(void* pData, size_t nBytes);
private:
    void outputData(std::vector<SegmentDescriptor>& segs);
    void freeData(std::vector<SegmentDescriptor>& segs);
    iovec* getIoVectors(size_t n);
    size_t getEventBodySize(const std::vector<SegmentDescriptor>& segs);
    
    std::vector<SegmentDescriptor> editEvent(pRingItemHeader pItem);
    
};
#endif
