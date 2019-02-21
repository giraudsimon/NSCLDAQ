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

/** @file:  BufferArena.h
 *  @brief: Manager for a set of ReferenceCountedBuffer objects
 */

#ifndef BUFFERARENA_H
#define BUFFERARENA_H

#include <deque>
#include <stddef.h>

namespace DDASReadout {

struct ReferenceCountedBuffer;
/**
 * @class BufferArena
 *    Provides a class for memory management in reference counted buffers.
 *    Clients request storage of a specific size, and return it later.
 *    The storage is provided as a reference counted buffer.
 *
 *    Storage allocation strategy is relatively simplistic with the idea
 *    that statistically, all storage managed by this object will wind up
 *    eventually being resized to the biggest required block.
 *
 *    This is suitable for I/O buffers but  very wasteful for ordinary
 *    storage management.  The primary use case is for buffers for PXI readout.
 */
class BufferArena {
private:
    std::deque<ReferenceCountedBuffer*> m_BufferPool;
public:
    virtual ~BufferArena();
    
    ReferenceCountedBuffer* allocate(size_t nBytes);
    void free(ReferenceCountedBuffer* pBuffer);
    
};

}                        // Namespace.
#endif