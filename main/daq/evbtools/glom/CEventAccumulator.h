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

/** @file:  CEventAccumulator.h
 *  @brief: Accumulates several events for output.
 */
#ifndef CEVENTACCUMULATOR_H
#define CEVENTACCUMULATOR_H

#include <DataFormat.h>

#include <deque>
#include <time.h>
#include <stddef.h>
#include <sys/uio.h>

namespace EVB
{
    typedef struct _FlatFragment FlatFragment;
    typedef FlatFragment *pFlatFragment;
}
/**
 ** @class CEventAccumulator
 **    This class provides a method to accumulate fragments into events
 **    which can then later be written to file.  Writing occurs when:
 **    -  Explicitly requested via a call to flushEvents()
 **    -  Adding a fragment to the event would overflow the fragment buffer
 **    -  A fragment is added and some fixed amount of time has passed since
 **       the last write. operation (intended to ensure responsive data flow
 **       even if rates are low).
 **
 **       @note, This is intended to accumulate PHYSICS_EVENT fragments
 **              actually will accumulate any homogenous fragment types.
 **
 **       @todo - mechanism for out of band data like scalers.
*/

class CEventAccumulator {
public:
    typedef enum _tsPolicy {
        first, last, average
    } TimestampPolicy;
private:
    struct EventHeader {
        RingItemHeader s_itemHeader;
        BodyHeader     s_bodyHeader;
    };
    struct EventAccumulation {
        size_t   s_nBytes;                // Includes body size field.
        size_t   s_nFragments;
        uint64_t s_TimestampTotal;
    };
    
    typedef struct EventInformation {
        EventHeader        s_eventHeader;
        EventAccumulation  s_eventInfo;
        void*              s_pBodyStart;
        void*              s_pInsertionPoint;
    } *pEventInformation;
    
    typedef std::deque<pEventInformation> EventInfoQ;
    
    
private:
    
    int             m_nFd;
    time_t          m_maxFlushTime;
    time_t          m_lastFlushTime;             //
    TimestampPolicy m_tsPolicy;

    void*   m_pBuffer;
    size_t  m_nBufferSize;
    size_t  m_nMaxFrags;
    size_t  m_nBytesInBuffer;
    EventInfoQ m_fragsInBuffer;
    EventInfoQ m_freeFrags;
    pEventInformation m_pCurrentEvent;
    
    iovec*   m_pIoVectors;
    size_t   m_nMaxIoVecs;
    size_t   m_nIoVecs;
    
public:
    CEventAccumulator(
        int fd, time_t maxFlushTime, size_t bufferSize, size_t maxfrags,
        TimestampPolicy policy
    );
    virtual ~CEventAccumulator();
    
public:
    void addFragment(EVB::pFlatFragment pFrag, int outputSid);
    void addOOBFragment(EVB::pFlatFragment pFrag, int outputSid);
    void finishEvent();
    void flushEvents();
private:
    pEventInformation allocEventInfo(EVB::pFlatFragment pFrag, int sid); //tested
    void              freeEventInfo(pEventInformation pInfo); // tested
    void              sizeIoVecs(size_t nVecs);               // tested
    size_t            makeIoVectors();
    void              slideCurrentEventToFront();
    size_t            freeSpace();                           // Tested.
    uint32_t          itemType(EVB::pFlatFragment pFrag);    // tested
    void              appendFragment(EVB::pFlatFragment pFrag); // tested.
    void              reserveSize();                         // tested
    
};

#endif