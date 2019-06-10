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

/** @file:  ReferenceCountedBuffer.h
 *  @brief: Provides a reference counted buffer.
 */
#ifndef REFERENCECOUNTEDBUFFER_H
#define REFERENCECOUNTEDBUFFER_H
#include <stddef.h>
#include <stdint.h>

/**
 * @struct ReferenceCountedBuffer
 *   This provides a reference counted buffer with dynamic storage.
 *   The idea is that rather than doing data movement, pointers can be
 *   registered with the buffer and unregistered.  The buffer can then
 *   be queried to determine if it can be released, or put in a free list.
 *
 *   One feature of this buffer is that, if there are not references,
 *   it can be resized.  Since resizing is only legal if there are
 *   no references, the contents of the buffer are not preserved across
 *   a resize.
 *
 *   The intended use for this data structure is to provide support for
 *   zero copy operations in the DDAS Readout program.  The idea is that
 *   a block of data can be read from a digitizer into one of these and
 *   then pointers to each hit created rather than performing copies into other
 *   data structures.
 *
 */

namespace DDASReadout {
    
struct ReferenceCountedBuffer {
    size_t s_size;                   // Number of bytes of data.
    size_t s_references;             // Number of references.
    void*  s_pData;                  // data pointer.
    
    ReferenceCountedBuffer(size_t initialSize = 0);
    virtual ~ReferenceCountedBuffer();
    
    // Reference count management:
    
    void reference();
    void dereference();
    bool isReferenced();
    
    // Other:
    
    void resize(size_t newSize);
    
    // Syntactical sugar for the most common casts; still nothing to stop
    // someone from mytyp* p = static_cast<mytyp*>(somebuffer.s_pData);
    
    operator uint8_t*()  { return static_cast<uint8_t*>(s_pData);}
    operator uint16_t*() {return static_cast<uint16_t*>(s_pData);}
    operator uint32_t*() {return static_cast<uint32_t*>(s_pData);}
    
};

}                            // namespace DDASReadout
#endif