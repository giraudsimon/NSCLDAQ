/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CUnglomDecoder.cpp 
 *  @brief: Implement the CUnglomDecoder class.
 */

#include "CUnglom.h"

/**
 *  The ring item factory is sort of like a smart upcast - given a ring item
 *  it creates the specific ring item object dynamically. This will be used
 *  to turn a CRingItem into a CPhysicsEventItem object..
*/
#include <CRingItemFactory.h>
#include <CRingItem.h>
#include <CPhysicsEventItem.h>

#include <DataFormat.h>           // Defines the ring item types.


// Fragment index provides iterator functionality on the fragments of a
// built event.

#include "FragmentIndex.h"         


#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sstream>

/**
 *   constructor
 *       We just need to initialize the end handler pointer to null so that
 *       there's no intial end handler.  std::map constructs to empty so there's
 *       no need to do anything with it.
 */
CUnglomDecoder::CUnglomDecoder() 
{
    
}

/**
 * operator()
 *    Responsible for decoding a single ring item.
 *    - Extract the type.
 *    - If the type is a PHYSICS_EVENT convert the item to a PhysicsEventItem
 *      and dispatch to decodePhysicsEvent
 *    - If the type is anything else, dispatch to decodeOtherItems.
 *
 * @param pItem - pointer to the ring item.
 */
void
CUnglomDecoder::operator()(CRingItem* pItem)
{
    std::uint32_t itemType = pItem->type();
    CRingItem* pActualItem = CRingItemFactory::createRingItem(*pItem);
    
    if (itemType == PHYSICS_EVENT) {
        CPhysicsEventItem* pPhysics =
            dynamic_cast<CPhysicsEventItem*>(pActualItem);
        if (!pPhysics) {
            std::cerr << "Error item type was PHYSICS_EVENT but factory could not convert it";
            return;
        }
        decodePhysicsEvent(pPhysics);
    } else {
        decodeOtherItems(pActualItem);
    }
    delete pActualItem;
}
/**
 * decodePhysicsEvent
 *    - extract the body header items just to show how that can be done.
 *    - Iterate over the fragments; for each fragment invoke
 *      any handler for its data source id.
 *    - If there's an end of event handler, invoke it after all fragments
 *      have been handed.
 *
 *  Note:  A warning message is emitted, and the fragment is not processed
 *         if it has no body header.
 *
 *   @param pItem - pointer to the physics even item  object.
 */
void
CUnglomDecoder::decodePhysicsEvent(CPhysicsEventItem* pItem)
{
    if (! pItem->hasBodyHeader()) {
        std::cerr << "Warning - an event has no body header - won't be processed\n";
        return;
    }
    
    // Pull out the body header information:
    
    std::uint64_t timestamp = pItem->getEventTimestamp();
    std::uint32_t srcid     = pItem->getSourceId();   // the source id for next stage building.
    std::uint32_t btype     = pItem->getBarrierType();
    
    
    // Build the fragment iterator and ask it how many fragments the event has:
    
    FragmentIndex iterator(reinterpret_cast<std::uint16_t*>(pItem->getBodyPointer()));
    
    // For each fragment write the ring item that made it to its fragment files.
    // If there is no fragment file, create one and add its info to the source
    // map.
    //   If there's a timestamp out of order report it as well.
    
    for (auto p = iterator.begin(); p != iterator.end(); p++)
    {
        FragmentInfo info = *p;
        
        // If necessary make a new output file/etc.
        
        if (m_sourceMap.count(info.s_sourceId) == 0) {
            makeNewInfoItem(info.s_sourceId);
        }
        // Fetch the size of the ring item:
        
        pRingItem pItem =
            reinterpret_cast<pRingItem>(info.s_itemhdr);
        size_t    nBytes= itemSize(pItem);
        write(m_sourceMap[info.s_sourceId].s_nFd, pItem, nBytes);
        
        // See if the timestamp and update last timestamp.
        
        checkTimestamp(info);
        
    }
    
    
}
/**
 * Handle non physics (non built)
 * events.
 */
void
CUnglomDecoder::decodeOtherItems(CRingItem* pItem)
{
    // For now just elide any items that don't have body headers.
    
    if (pItem->hasBodyHeader()) {
        std::uint32_t sid = pItem->getSourceId();
        if (m_sourceMap.count(sid) == 0) {
            makeNewInfoItem(sid);
        }
        size_t nBytes = itemSize(pItem->getItemPointer());
        write(m_sourceMap[sid].s_nFd, pItem->getItemPointer(), nBytes);
        
        // Don't update the timestmap.
    }
}
/**
 *  Called on end input file:
 */
void
CUnglomDecoder::onEndFile()
{
    // Close all of the files that are open.
    
    for (auto p = m_sourceMap.begin(); p!= m_sourceMap.end(); p++) {
        sourceInfo sinfo = p->second;
        close(sinfo.s_nFd);
        
    }
    m_sourceMap.clear();              // Empty the map to be idempotent.
    
}
/**
 * makeNewInfoItem
 *     Creates a new information item for an sid:
 *     - Opens a file for it named sid-%x  where sid is the source  id.
 *     - Fills in the template struct with a zero timestamp and the fd
 *       of the newly opened file.
 *     - adds the struct to the map.
 *  @param sid - id of the source to make an info item for.
 */
void
CUnglomDecoder::makeNewInfoItem(std::uint32_t sid)
{
    std::stringstream fnameStream;
    fnameStream << "sid-" << std::hex << sid;
    const char* filename = fnameStream.str().c_str();
    
    int fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        perror("Failed to open a new segment file");
        std::exit(EXIT_FAILURE);
    }
    sourceInfo sinfo;
    sinfo.s_lastTimestamp = 0;
    sinfo.s_nFd           = fd;
    m_sourceMap[sid] = sinfo;
}

/**
 * checkTimestamp
 *    Checks that a timestamp we've received is >= than the last one for
 *    that sourceid.
 *
 *    -  If that's not the case yells on stderr.
 *    -  Update sht most recently received timestamp.
 */
void
CUnglomDecoder::checkTimestamp(const FragmentInfo& finfo)
{
    std::uint32_t sid = finfo.s_sourceId;
    std::uint64_t ts  = finfo.s_timestamp;
    sourceInfo& sinfo(m_sourceMap[sid]);
    
    if (ts < sinfo.s_lastTimestamp) {
        std::cerr << "Sid" << sid << " timestamp went backwards from : "
                  << sinfo.s_lastTimestamp << " to " << ts << std::endl;
    }
    sinfo.s_lastTimestamp = ts;
}
