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
 * @file unglom.cpp
 * @brief Filter that separates an event file output by glom into its 
 *        constituent fragments. 
 * @author: Ron Fox
 */


#include <config.h>

#include <io.h>
#include <fragment.h>
#include <fragio.h>
#include <Exception.h>
#include <exception>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <DataFormat.h>
#include <CRingItemFactory.h>
#include <CRingItem.h>

#include "cmdline.h"

// forward prototypes:

static void* readEvent();
static void writeEvent(void* pEvent);
static void writeNonPhysicsItem(CRingItem* pItem);
static void writePhysicsItem(CRingItem* pItem);

static void* pLastItem=0;

static uint32_t sourceId;	// Source id for non-event fragments.

/**
 * glom
 *
 *  This file contains a 'simple' filter which takes event data output by glom
 *  and sepates it back into an input stream suitable for use with glom again.
 *  A typical use would be in a pipeline with glom again to rebuild events with a 
 *  different coincidence interval  For example:
 *
 * \verbatim
 *    unglom <old-event-file | glom -dt 1234 >new-event-file
 * \endverbatim
 *
 */
int main(int argc, char**argv)
{
  gengetopt_args_info args;
  cmdline_parser(argc, argv, &args);
  sourceId = args.id_arg;

  try {
    while(1) {
      void* pEvent = readEvent();
      if (!pEvent) break;
      writeEvent(pEvent);
      free(pEvent);
    }
  }
  catch (std::string msg) {
    std::cerr << "unglom: Exception caught: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "unglom: Exception caught: " << msg << std::endl;
  }
  catch (CException& e) {
    std::cerr  << "unglom: Exception caught: " << e.ReasonText() << std::endl;
  }
  catch (int err) {
    std::cerr << "unglom: Exception caught: " << strerror(err) << std::endl;

  }
  catch (std::exception e) {
    std::cerr << "unglom: Exception caught: " << e.what() << std:: endl;
  }
  catch(...) {
    std::cerr << "unglom: Unrecognized exception caught\n";
  }
  
}


/*-----------------------------------------------------
** Static functions:
*/

/**
 * readEvent
 * 
 * Reads an event from stdint.
 * 
 * @return void* Pointer to the event storage which must
 *               be freed using free(3).
 * @retval NULL if an endfile was hit during the read of the
 *              header.
 * @throw int on error.
 *
 * @note - We will catch io::readData's throw and if a 0
 *         value is thrown on the header read we'll return
 *         a null indicating an eof.
 */
static void*
readEvent()
{
  // read the ringheader:

  RingItemHeader hdr;
  try {
    int nRead = io::readData(STDIN_FILENO, &hdr, sizeof(RingItemHeader));
    if (nRead != sizeof(RingItemHeader)) {
	throw std::string("EOF in middle of ring item header!");
    }

  }
  catch (int err) {
    if (err == 0) {
      return NULL;
    } else {
      throw;
    }
  }
  // Allocate the data.  The header indicates 
  // the number of bytes we need.
  

  uint8_t* pData = reinterpret_cast<uint8_t*>(malloc(hdr.s_size));
  if (!pData) {
    throw errno;		// Allocation failed.
  } 
  // Copy the header into the data and point to the
  // body which is then read:

  memcpy(pData, &hdr, sizeof(RingItemHeader));
  uint8_t* pBody = pData + sizeof(RingItemHeader);
  
  try {
    size_t bodySize =  hdr.s_size - sizeof(RingItemHeader);
    size_t nRead = io::readData(STDIN_FILENO, pBody, bodySize); 
    if (bodySize != nRead) {
      throw std::string("EOF in middle of a ring item");
    }

  }
  catch (int e) {
    // free storage:

    free(pData);
    throw;
  }
  // At this time, pData points to the fully read ring
  // item:

  return pData;
  

}
/**
 * writeEvent
 *  Writes an output event
 *  - non physics events are written as barriers of the
 *    'appropriate type' with an assigned source id.
 *  - physics events have their fragments written verbatim
 *    to the output file.
 *
 * @param pEvent - Points to the ring item that is the event.
 *
 * @throw - int from io::writeData
 */
static void writeEvent(void* pEvent)
{
  if(!CRingItemFactory::isKnownItemType(pEvent)) {
    throw std::string("Unrecognized ring item in input stream");
  }
  

  CRingItem* pItem = CRingItemFactory::createRingItem(pEvent);
  //  Save the last item contents:
  //
  //  
  pRingItem pRItem = pItem->getItemPointer();
  pLastItem = realloc(pLastItem, itemSize(pRItem));
  if (pLastItem) {
    memcpy(pLastItem, pRItem, itemSize(pRItem));
  } else {
    throw std::string("Unable to allocate storage for prior item");
  }
  if (pItem->type() != PHYSICS_EVENT) {
    writeNonPhysicsItem(pItem);
  } else {
    writePhysicsItem(pItem);
  }

  delete pItem;
}
/**
 * writeNonPhysicsItem
 *   Takes a non Physics item, maps its type into the
 *   correct barrier type.  Since data source information is not
 *   preserved at this time, a source Id is assigned to the fragments.
 *   emitted.
 *  
 * @note The ring item type is what we will put in as the barrier type.
 * @param pFrag - Pointer to the fragment to emit.
 */
static void writeNonPhysicsItem(CRingItem* pItem)
{

  // Fill in the header:

  pRingItem pRItem = pItem->getItemPointer();

  // Provide initial values
  EVB::FragmentHeader hdr;
  hdr.s_timestamp = NULL_TIMESTAMP;
  hdr.s_sourceId  = sourceId;
  hdr.s_size      = itemSize(pRItem);
  uint32_t type=itemType(pRItem);

  // at the moment we assume that only state change items are barriers
  if (    type == BEGIN_RUN || type == END_RUN 
          || type == PAUSE_RUN || type==RESUME_RUN ) { 
    hdr.s_barrier   = pRItem->s_header.s_type;
  } else {
    // all non-state change items are not barriers
    hdr.s_barrier   = 0;
  }

  // if the ring item has a body header, override the 
  // values in the fragment header with the values 
  // in the body header 
  if (pItem->hasBodyHeader()) {
    hdr.s_timestamp = pItem->getEventTimestamp();
    hdr.s_sourceId  = pItem->getSourceId();
    hdr.s_barrier   = pItem->getBarrierType();
  }

  EVB::pFragment p = allocateFragment(&hdr);
  memcpy(p->s_pBody, pRItem, pRItem->s_header.s_size);
  CFragIO::writeFragment(STDOUT_FILENO, p);

  freeFragment(p);

  
}
/**
 * writePhysicsEvent
 *
 *  Write a physics event as its component fragments.  This works by recognizing
 *  that each physics event is a list of flattened fragments.
 *
 * @param pItem - Pointer to the event.  The caller has ensured that the item is,
 *                in fact, a PHYSICS_EVENT item.
 *
 */
static void
writePhysicsItem(CRingItem* pItem)
{
  uint32_t residualSize = pItem->getBodySize();
  uint8_t* pBody        = reinterpret_cast<uint8_t*>(pItem->getBodyPointer());

  // Skip the leading event size glom filled in.

  pBody                += sizeof(uint32_t);
  residualSize         -= sizeof(uint32_t);

  while (residualSize) {
    EVB::pFlatFragment pFrag = reinterpret_cast<EVB::pFlatFragment>(pBody);
    EVB::Fragment      frag;
    frag.s_header = pFrag->s_header;
    frag.s_pBody  = pFrag->s_body;
    CFragIO::writeFragment(STDOUT_FILENO, &frag);

    uint32_t totalSize = sizeof(EVB::FragmentHeader) + pFrag->s_header.s_size;
    pBody += totalSize;
    residualSize -= totalSize;
  }
}
