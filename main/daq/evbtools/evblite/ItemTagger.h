/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef  ITEMTAGGER_H
#define  ITEMTAGGER_H

#include <stddef.h>
#include <stdint.h>
#include <CRingFileBlockReader.h>
#include <fragment.h>
#include <DataFormat.h>

/** @file:  ItemTagger.h
 *  @brief: Class definition for the item tagger used by evbtaggerMain
 */

class ItemTagger {
private:
    size_t m_blockSize;          // Must fit at least one ring item.
    int    m_sourceFd;
    int    m_sinkFd;
    bool   m_resetLastTSOnBegin; // If true begin run resets m_lastTimestamp->0
    uint64_t m_lastTimestamp;    // Timestamp for items that want one assigned.
    uint32_t m_defaultSid;       // default source id.
public:
    ItemTagger(int in, int out, size_t blockSize, bool resetOnBegin, uint32_t sid);
    
    void operator()();
    
    // private utilities:
private:
    void processItems(const CRingBlockReader::DataDescriptor&  info);
    void fillFragHeader(EVB::FragmentHeader& hdr, const RingItem* pItem);
    
};


#endif