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

/** @file:  CEvents2Fragments.cpp
 *  @brief: Implements CEvents2Fragments
 */

#include "CEvents2Fragments.h"
#include "CFragmentMaker.h"
#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>
#include <DataFormat.h>
#include <fragment.h>

#include <stdlib.h>
#include <stdint.h>
#include <iostream>

#include <stdexcept>
/**
 * constructor
 *    Just saves the stuff we need to operate:
 *
 * @param readSize    - number of bytes we try to get for each read.
 * @param reader      - The reader object that supplies data.
 * @param headerMaker - Object that makes fragment headers for ring items.
 * @param writer      - buffered writer.
 */

CEvents2Fragments::CEvents2Fragments(
    int readSize, CRingFileBlockReader& reader, CFragmentMaker& headerMaker,
    io::CBufferedOutput& writer
) :
    m_nReadSize(readSize), m_Reader(reader), m_HeaderMaker(headerMaker),
    m_Writer(writer)
{}

/**
 * destructor - currently does nothing.
 */
CEvents2Fragments::~CEvents2Fragments() {}

/**
 * operator()
 *    Executes the operation.
 */
void
CEvents2Fragments::operator()()
{
    CRingBlockReader::DataDescriptor desc;
    
    
    desc = m_Reader.read(m_nReadSize);
    while (desc.s_nBytes) {         // s_nBytes == 0 at EOF.
        processBlock(desc);
        free(desc.s_pData);
        
        if (runEnded()) break;
        
        desc = m_Reader.read(m_nReadSize);
    }
    m_Writer.flush();         // output partial buffer.
}

/////////////////////////////////////////////////////////////////////////////////
// Private Utilities.

/**
 * processBlock
 *    Processes a block of data from the block reader.
 * @param desc - reference to the descriptor for the last read.
 */
void
CEvents2Fragments::processBlock(CRingFileBlockReader::DataDescriptor& desc)
{
    uint8_t* pBytes(reinterpret_cast<uint8_t*>(desc.s_pData));
    pRingItem pItem(reinterpret_cast<pRingItem>(desc.s_pData));
    
    
    uint32_t nItems = desc.s_nItems;
    uint32_t nBytes = desc.s_nBytes;
    
    while (nItems) {
        int itemSize = processItem(pItem);
        
        nItems--;
        
        if (itemSize > nBytes) {
            throw std::logic_error("processBlock - buffer over-reach");
        }
        nBytes -= itemSize;
        
        pBytes += itemSize;
        pItem   = reinterpret_cast<pRingItem>(pBytes);
                   
    }
    if (nBytes != 0) {
        std::cerr << nBytes << " left over bytes\n";
        std::cerr << "Originally " << desc.s_nItems << " in "
            << desc.s_nBytes << " bytes\n";
        throw std::logic_error("processBlock - didn't process the whole buffer");
    }
}
/**
 * processItem
 *    Get the fragment header for one item, and then write it and the
 *    fragment to the writer.
 *
 *  @param pItem  - pointer to the item to write.
 */
int
CEvents2Fragments::processItem(void* pItem)
{
    pRingItem p = static_cast<pRingItem>(pItem);
    
    EVB::FragmentHeader hdr = m_HeaderMaker.makeHeader(p);
    
    m_Writer << hdr;
    m_Writer.put(p, itemSize(p));
    
    return itemSize(p);     // ring item size.
}
/**
 * runEnded
 *    @return bool - true if the run ended, else false.
 *    @note there's an assumption that the first block will have at least
 *    one begin run item else this could return true before we actually
 *    even see the begin run item.  This is ensured with a sufficiently big
 *    read size.
 */
bool
CEvents2Fragments::runEnded()
{
    return m_HeaderMaker.getEndRunsRemaining() == 0;
}