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

/** @file:  RawChannel.cpp
 *  @brief: Implement the raw channel struct.
 */

#include "RawChannel.h"
#include <stdlib.h>
#include <string.h>
#include <new>
#include <iostream>

namespace DDASReadout {
static const uint32_t CHANNELIDMASK(0xF);            // bits 0-3 inclusive
static const uint32_t SLOTIDMASK(0xF0);              // bits 4-7 inclusive
static const uint32_t CRATEIDMASK(0xF00);            // bits 8-11 inclusive
static const uint32_t HEADERLENGTHMASK(0x1F000);     // bits 12-16 inclusive
static const uint32_t CHANNELLENGTHMASK(0x3FFE0000); // bits 17-29 inclusive
static const uint32_t CHANNELLENGTHSHIFT(17);        // Shift chan length to right justify.
static const uint32_t OVERFLOWMASK(0x40000000);      // bit 30
static const uint32_t FINISHCODEMASK(0x80000000);    // bit 31
static const uint32_t LOWER16BITMASK(0xFFFF);        // lower 16 bits
/**
 * constructor
 *    Constructs a new raw channel that could be used in either zerocopy or
 *    copy mode.   The size and data are not yet set and the
 *    owndata flag is set false since we never need to delete nothing.
 */
RawChannel::RawChannel() :
    s_time(0.0), s_chanid(0), s_ownData(false), s_ownDataSize(0),
    s_channelLength(0), s_data(nullptr)
{}

/**
 * constructor
 *   Construts a channel for copy in data.  Data are pre-allocated as
 *   demanded but not initialized.  We use malloc rather than new because
 *   new will construct (initialize) ints to zero and we don't want to take
 *   that time.
 *
 *   Data must eventually be provided by calling copyInData.
 *
 * @param nWords - number of words of data to preallocate
 * @note after this call, m_ownData is true and m_ownDataSize is set to
 *       nWords.
 * @throws std::bad_alloc if the malloc fails.
 */
RawChannel::RawChannel(size_t nWords) :
s_time(0.0), s_chanid(0), s_ownData(true), s_ownDataSize(nWords),
    s_channelLength(0), s_data(nullptr)
{
    s_data = static_cast<uint32_t*>(malloc(nWords * sizeof(uint32_t)));
    if (!s_data) {
        throw std::bad_alloc();
    }
}
/**
 * constructor
 *    Constructs, initialized with zero copy data.
 *
 *  @param nWords  - Number of 32 bit words of data in the hit.
 *  @param pZcopyData - pointer to the data of the hit.
 *  @note The data pointed to by pZcopyData must be in scope for the
 *        duration of this object's lifetime else probably segfaults or
 *        buserrors will happen in the best case.
 */
RawChannel::RawChannel(size_t nWords, void* pZCopyData) :
s_time(0.0), s_chanid(0), s_ownData(false), s_ownDataSize(nWords),
    s_channelLength(nWords), s_data(static_cast<uint32_t*>(pZCopyData))
{}
/**
 * destructor
 *   If we own the data, this will free it.
 */
RawChannel::~RawChannel()
{
    if(s_ownData) free(s_data);
}
/**
 * SetTime
 *    Assumes that the data are set (either by zero copy or by copyInData).
 *    determines the raw timestamp from the 48 bit timestamp data in the
 *    hit and set it in s_time.
 *  @return int - 0 - success 1 if the number of words is insufficient.
 *  @note - if the data have not yet been set, number of words is 0 so this is
 *          well behaved.
 */
int
RawChannel::SetTime()
{
    if (s_channelLength >=4) {
        uint64_t t = s_data[2] & LOWER16BITMASK;
        t          = t << 32;
        t         |= (s_data[1]);
        s_time = t;
        return 0;
    } else {
        return 1;
    }
}
/**
 * SetTime
 *    Set the time in ns.
 *    @param ticksPerNs - nanoseconds each tick is worth.
 *    @param useExt     - True if the external clock should be
 *                        used rather than the internal clock.
 */
int
RawChannel::SetTime(double ticksPerNs, bool useExt)
{
    if (useExt) {
        // The external timestamp requires a header length of at
        // least 6 words and is always the last two words of the
        // header:
        
        uint32_t headerSize = (s_data[0] & 0x1f000) >> 12;
        if (headerSize >= 6) {
           uint64_t extStampHi = s_data[headerSize-1] & 0xffff;
           uint64_t extStampLo = s_data[headerSize-2];
           uint64_t stamp      = (extStampHi << 32) | (extStampLo);
           s_time = stamp;
        } else {
            return 1;          // There's no external stamp.
        }
        
    } else {
       if (s_channelLength >= 4) {
            SetTime();
        
        } else {
            return 1;
        }
    }
    s_time *= ticksPerNs;
    return 0;
}
/**
 * SetLength
 *    Set the correct length for the data.
 */
int
RawChannel::SetLength()
{
    s_channelLength = channelLength(s_data);
    return 0;
}

/**
 * SetChannel
 *    Sets the correct channel value from the data.
 *    @return int 0 correct, 1 if failed.  Failure means there's not sufficient
 *    data in the hit or the hit hasn't been set.
 */
int
RawChannel::SetChannel()
{
    if (s_channelLength >=4) {
        s_chanid = (s_data[0] & CHANNELIDMASK);
        return 0;
    } else {
        return 1;
    }
}
/**
 * Validate
 *    Determines if a channel has the correct amount of data.
 * @param expecting
 * @return int 0 - correct, 1, incorrect and a message is output to std::cerr.
 *                Hate the output but this retains rough compatibility with the old
 *                channel class.
 */
int
RawChannel::Validate(int expecting)
{
    if (s_channelLength == expecting) {
        return 0;
    } else {
        std::cerr << "Data is corrupt or the setting for in ModEvtLen.txt is wrong\n";
        std::cerr << "Expected " << expecting << " got " << s_channelLength << std::endl;
        return 1;
    }
}

/**
 * setData
 *    - If we own data already it's freed.
 *    - Our data and channelLength are set from the parameters.
 *
 *    @param nWords - new channelLength
 *    @param pZCopyData  - New value for s_data.
 */
void
RawChannel::setData(size_t nWords, void* pZCopyData)
{
    if (s_ownData) {
        free(s_data);
        s_ownData = false;
        s_ownDataSize = 0;
    }
    
    s_channelLength = nWords;
    s_data          = static_cast<uint32_t*>(pZCopyData);
}
/**
 * copyInData
 *    Copies in data:
 *    - If s_ownData is false, then allocate sufficient storage for the hit.
 *    - If s_ownData is true, and the amount of data we have is too small,
 *      allocate new data to hold it.
 *    - Copy the hit into our owned data.
 *
 * @param nWords - number of uint32_t words.
 * @param pData  - Pointer to the data to copy in.
 */
void
RawChannel::copyInData(size_t nWords, const void* pData)
{
    // we need to allocate unless we're already dynamic and have a big enough
    // block allocated;  This minimizes allocations.
    
    bool mustAllocate = !(s_ownData && (nWords <= s_ownDataSize));
    
    if (mustAllocate) {
        if (s_ownData) free(s_data);
        s_data        = static_cast<uint32_t*>(malloc(nWords * sizeof(uint32_t)));
        s_ownData     = true;
        s_ownDataSize = nWords;
    }
    s_channelLength = nWords;
    memcpy(s_data, pData, nWords * sizeof(uint32_t));
    
}

/**
 * copy Construction
 *    This is just assignment to *this once we're appropriately initialized.
 *
 * @param rhs - the object we're copying into this.
 */
RawChannel::RawChannel(const RawChannel& rhs) :
    s_ownData(false), s_ownDataSize(0), s_data(nullptr)
{
    *this = rhs;
}

/**
 * assignment
 *   Only works if this != &rhs.
 *   There are piles of cases to consider:
 *   - rhs is zero copy - we'll zero copy.
 *   - rhs is dynamic - we'll be a dynamic deep copy.
 *
 * These two cases and their subcases are handled by setData and copyInData
 * respectively.
 *
 *  @param rhs - the object being assigned to us.
 *  @return *this.
 */
RawChannel&
RawChannel::operator=(const RawChannel& rhs)
{
    if (this != &rhs) {
        if (rhs.s_ownData) {
            copyInData(rhs.s_channelLength, rhs.s_data);
        } else {
            const void* p = static_cast<const void*>(rhs.s_data);
            setData(rhs.s_channelLength, const_cast<void*>(p));
        }
        // now all the other stuff not set by the above:
        
        s_time = rhs.s_time;
        s_chanid = rhs.s_chanid;
        
    }
    return *this;
}


/**
 * channelLength
 *    Given a pointer to a hit, extracts the number of words in the hit:
 * @param pData - pointer to the hit.
 * @return uint32_t - length of the hit
 */
uint32_t
RawChannel::channelLength(void* pData)
{
    uint32_t* p = static_cast<uint32_t*>(pData);
    return (*p & CHANNELLENGTHMASK) >> CHANNELLENGTHSHIFT;
}    
 
}                               // Namespace.

// Comparison operations allow sorts to work without anything special
// The assuem that SetTime() has been called.

bool
operator<(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2)
{
    return c1.s_time < c2.s_time;
}
bool
operator>(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2)
{
    return c1.s_time > c2.s_time;
}
bool
operator==(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2)
{
    return c1.s_time == c2.s_time;
}
