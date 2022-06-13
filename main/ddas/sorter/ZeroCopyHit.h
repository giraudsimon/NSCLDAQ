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

/** @file: ZeroCopyHit.h
 *  @brief: Class to manage a zero copy RawChannel that comes from insode
 *          a buffer from a buffer arena.
 */
#ifndef ZEROCOPYHIT_H
#define ZEROCOPYHIT_H
#include "RawChannel.h"
namespace DDASReadout {

struct ReferenceCountedBuffer;
class  BufferArena;
    
/**
 *  ZeroCopyHit
 *     This class extends RawChannel to produce a raw channel that is
 *     zerocopied from a reference counted buffer that comes from
 *     a buffer arena.  This is a key data structure in the zero copy
 *     DDAS readout.
 *
 *     This acts like a RawChannel, but on destruction, if it is the last
 *     reference to the buffer, returns it to the arena from whence it came.
 *
 *     Copy construction and assignment are supported with appropriate
 *     semantics to handle proper reference counting.
 *
 *     I'm sure I'm hearing a pile of people ask me why not use
 *     std::shared_pointer - the answer is that I don't actually  want the
 *     buffers to be destroyed as that involves expensive dynamic memory management,
 *     I want storage returned to a pre-allocated buffer arena from which it can
 *     be quickly re-gotten.  Yeah I suppose I could use custom new/delete methods
 *     but that seems pretty painful and at least as error prone as this code.
 */
class ZeroCopyHit : public RawChannel
{
private:
    ReferenceCountedBuffer*    m_pBuffer;   // The hit lives in here.
    BufferArena*               m_pArena;    // The buffer came from this arena.
public:
    ZeroCopyHit();
    ZeroCopyHit(
        size_t nWords, void* pHitData, ReferenceCountedBuffer* pBuffer,
        BufferArena* pArena
    );
    ZeroCopyHit(const ZeroCopyHit& rhs);
    ZeroCopyHit& operator=(const ZeroCopyHit& rhs);
    virtual ~ZeroCopyHit();
    
    //  Support for recycling ZeroCopyHhits.
    
    void setHit(
        size_t nWords, void* pHitData, ReferenceCountedBuffer* pBuffer,
        BufferArena* pArena
    );                        
    void freeHit();

private:
    void reference();
    void dereference();
    
};




}                                // namespace.


#endif