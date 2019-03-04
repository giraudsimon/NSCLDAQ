/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CFragwriter.cpp
 * @brief Fragment writer (fragment -> ring item -> file number) class implementation
 */

#include "CFragWriter.h"
#include "fragment.h"
#include <io.h>

#include <DataFormat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <CRingItemFactory.h>
#include <CRingItem.h>

/**
 * Constructor
 *
 * @param fd - File descriptor to output ring fragments on.
 */
CFragWriter::CFragWriter(int fd, bool stripHeaders) :
  m_fd(fd), m_stripHeaders(stripHeaders) {}

/**
 * operator() 
 *
 * Depending on whether the user wants to convert to ring items by stripping
 * off the fragment header or by simply converting to EVB_FRAGMENT types, this
 * will call the correct method. 
 *
 * @param pFragment - Really a pointer to a 'flattened' fragment.  A
 *                    flattened fragment is a fragment header that is immediately
 *                    followed by its payload.
 */
void
CFragWriter::operator()(void* pFragment)
{
  if (m_stripHeaders) {
    stripAndWrite(pFragment);
  } else {
    convertAndWrite(pFragment);
  }
}

/**
 * Convert a fragment to an EVB_FRAGMENT ring item and output it to
 * stdout.
 *
 * @param pFragment - Really a pointer to a 'flattened' fragment.  A
 *                    flattened fragment is a fragment header that is immediately
 *                    followed by its payload.
 */
void CFragWriter::convertAndWrite(void* pFragment) {

    // Cast the pointer to a fragment header and also locate the payload.
  
    EVB::pFragmentHeader pHeader = reinterpret_cast<EVB::pFragmentHeader>(pFragment);
//    std::cerr << "pHeader  @ 0x" << std::hex << std::setw(8) << pFragment << std::dec << std::endl;
    void*                pPayload = reinterpret_cast<void*>(pHeader+1);
//    std::cerr << "pPayload @ 0x" << std::hex << std::setw(8) << pPayload << std::dec << std::endl;
  
//    std::cerr << std::flush;
    /*
      Create the ring item except for the data section
      the - sizeof(uint8_t) removes the stub of a 1 byte payload that is part of the
      struct (s_body).
    */
//    std::cerr << "Size of event = " << pHeader->s_size << " bytes" << std::endl; 
    pEventBuilderFragment pItem = formatEVBFragment(
        pHeader->s_timestamp, pHeader->s_sourceId, pHeader->s_barrier,
        pHeader->s_size, pPayload
    );
  
//    std::cerr << "pItem    @ 0x" << std::hex << std::setw(8) << pItem << std::dec << std::endl;
    if (pItem==0) throw std::string("CFragWriter::convertAndWrite(void*) Failed to allocate new pEventBuilderItem");
    // Write the header and the payload.
  
    Write(pItem->s_header.s_size, pItem);

 //   std::cerr << "Freeing pItem @ 0x" << std::hex << std::setw(8) << pItem << std::dec << std::endl;
    free(pItem);  
}


/**
  * Strip off the fragment header and write the ring item payload
  * 
  * This assumes that the fragment header is actually appended onto a valid ring item.
  * If for some reason the user is using the event builder for non-NSCLDAQ type data,
  * and he/she is interested in using this class, this would be a bad method to use.
  * The only safe thing to do is to convert to EVB_FRAGMENT ring items.
  *
  * \param pFragment  a pointer to a EVB::FragmentHeader with payload 
  *
  * 
  */
void CFragWriter::stripAndWrite(void* pFragment) {

    EVB::pFragmentHeader pHeader = reinterpret_cast<EVB::pFragmentHeader>(pFragment);
    pRingItem           pPayload = reinterpret_cast<pRingItem>(pHeader+1); // payload after the fragment.
  
    // fill in the body header if it doesn't exist.
    CRingItem* pItem = CRingItemFactory::createRingItem(pPayload);
    if (!pItem->hasBodyHeader()) {
      pItem->setBodyHeader( pHeader->s_timestamp, pHeader->s_sourceId, 
                            pHeader->s_barrier);
      // update our payload to the new item with the body header
      pPayload = reinterpret_cast<pRingItem>(pItem->getItemPointer());
    }

    // Write the payload only
    Write(pPayload->s_header.s_size, pPayload);

    delete pItem;
}

/**
 * Write
 *
 * Write a block of data to the file descriptor.  If needed multiple underlying
 * write(2) calls are made.
 * 
 * @param size - Number of bytes to write.
 * @param pBuffer - Pointer to the first word of the buffer to write.
 */
void
CFragWriter::Write(size_t nBytes, void* pBuffer)
{
  try {
    io::writeData(m_fd, pBuffer, nBytes);
  }
  catch(int e) {
    if(e) {
      throw std::string(strerror(e));
    } else {
      throw std::string("Premature end of file");
    }
  }
}

