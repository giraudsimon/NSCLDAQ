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

/** @file:  CRingItemSorter.cpp
 *  @brief: Implements the sorter.
 */
#include "CRingItemSorter.h"
#include "CSender.h"
#include "CReceiver.h"

/**
 * constructor
 *    @param fanin - the receiver for data fanned in from the data sources.
 *    @param sink -  Where we send sorted data.
 *    @param window - the timestamp tick window that determines when to emit.
 *    @param nWorkers - Numbrer of workers that will send us end data messages.
 */
CRingItemSorter::CRingItemSorter(
    CReceiver& fanin, CSender& sink, uint64_t window, size_t nWorkers
) : m_pDataSource(&fanin), m_pDataSink(&sink), m_nTimeWindow(window),
    m_nEndsRemaining(nWorkers)
{
    
}

/**
 * destructor
 */
CRingItemSorter::~CRingItemSorter()
{
    delete m_pDataSource;
    delete m_pDataSink;
}

/**
 * operator()
 *   Main flow of control of the processing element.
 */
void
CRingItemSorter::operator()()
{
    
}

/**
 * process
 *    Called when a clump of ring items has been rpesented to the
 *    sorter from one of the clients.
 *
 * @param pData - pointer to the ring items.
 * @param nBytes - Number of bytes of data.
 */
void
CRingItemSorter::process(void* pData, size_t nBytes)
{

}