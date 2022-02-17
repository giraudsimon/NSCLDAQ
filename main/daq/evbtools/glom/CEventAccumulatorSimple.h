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

/** @file:  CEventAccumulatorSimple.h
 *  @brief: Simplified event accumulator.
 */
#ifndef CEVENTACCUMULATORSIMPLE_H
#define CEVENTACCUMULATORSIMPLE_H
#include <fragment.h>
#include <DataFormat.h>
#include <time.h>

/**
 * @class CEventAccumulatorSimple
 * CEventAccumulator is much more complex than it has to be holding the
 * header information separated from the data etc.  This is a simplified
 * version witht the same external interface that just builds the events into
 * a buffer which can be written with io::writeData e.g.
 *
 * This simplifies the book keeping which, in turn hopefully makes stuff
 * more reliable.
 */

class CEventAccumulatorSimple
{
public:
    typdef enum _tsPolicy {
        first, last, average
    } TimestampPolicy;
private:
    // This is the overall header of an event.
#pragma pack(push, 1)
    typedef struct _EventHeader {
        RingItemHeader s_itemHeader;      // Ring item header.
        BodyHeader     s_bodyHeader;      // Overall body header.
        uint32_t       s_fragBytes;       // Bytes of fragment data (self inclusive).
    } EventHeader, *pEventHeader;
#pragma pack(pop)
    typedef struct _Event {
        pEventHeader s_header;
        uint64_t     s_lastTimestamp;
        uint64_t     s_timestampTotal;
        size_t       s_nFragments;
    } Event, *pEvent;
    
private:
    int    m_nFd;
    time_t m_maxFlushTime;
    time_t m_lastFlushTime;
    TimestampPolicy m_tsPolicy;
    size_t m_nBufferSize;
    size_t m_nMaxFrags;
    
    void* m_pBuffer;
    size_t m_nBytesInBuffer;
    uint8_t* m_pCursor;
    Event  m_currentEvent;
    pEvent m_pCurrentEvent;      // Event being built up null or points to m_currentEvent
public:
    CEventAccumulatorSimple(
        int fd, time_t maxFlushTime, size_t bufferSize, size_t maxfrags,
        TimestampPolicy policy
    );
    virtual ~CEventAccumulatorSimple();
    
    // public manipulators:
    
    void addFragment(EVB::pFlatFragment pFrag, int outputSid);
    void addOOBFragment(EVB::pFlatFragment pFrag, int outputSid);
    void finishEvent();
    void flushEvents();
private:
    bool mustFinish(EVB::pFlatFragment pFrag, int outputSid);
    bool mustFlush(EVB::pFlatFragment pFrag);
    void newEvent(EVB::pFlatFragment pFrag, int outputSid);
    size_t fragmentSize(EVB::pFlatFragment pFrag);

};
#endif