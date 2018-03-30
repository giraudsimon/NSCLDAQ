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

/** @file:  CMerge.cpp
 *  @brief: Implement the merge sort.
 *  @note: since there's no timing involved this is much simpler than the
 *         event orderer.
 */

#include "CMerge.h"
#include <CDataSource.h>
#include <CRingItem.h>
#include <stdio.h>
#include <fragment.h>
#include <algorithm>
#include <ErrnoException.h>
#include <DataFormat.h>
#include <iostream>

/**
 * constructor
 *   - Save the output file pointer,
 *   - Copy in the data source pointer vector.
 *   - Allocate the source info  array.
 */
CMerge::CMerge(FILE* output, std::vector<CDataSource*> sources) :
    m_pOutput(output), m_dataSources(sources), m_sources(0)
{
    m_sources = new sourceInfo[m_dataSources.size()];
    for (int i =0; i < m_dataSources.size(); i++) {
        m_sources[i].s_pItem = nullptr;
        m_sources[i].s_thisStamp = NULL_TIMESTAMP;
        m_sources[i].s_lastStamp = 0;
    }
}
/**
 * destructor
 *    free the data sources.  -- if there are any ring items left they are
 *                               freed too:
 */
CMerge::~CMerge()
{
    // Kill off any ring items:
    
    for (int i = 0; i < m_dataSources.size(); i++) {
        delete m_sources[i].s_pItem;
    }
    // Kill of the array of structs too:
    
    delete []m_sources;
    
    // delete the data sources;
    
    for (int i = 0; i < m_dataSources.size(); i++) {
        delete m_dataSources[i];
    }
}
/**
 * operator()
 *    Do the sort/merge.
 *    This means processing the begin barrier,
 *    Processing data until we hit the end
 *    Processing the end barrier.
 */
void
CMerge::operator()()
{
    Begin();
    while (!atEnd()) outputOldest();
    End();                           // That was easy.
}
/**
 * Begin
 *    Process the begin barrier.  What we do:
 *    We load the data sources with data.
 *    We output all Begin run fragments and keep track of  when we don't
 *    have  one.
 *  Note we assume there's always at least one fragment because unglom would not
 *  make a file without seeing one.
 */
void
CMerge::Begin()
{
    // Load the source infos:
    
    for (int i =0; i < m_dataSources.size(); i++) {
        readFragment(i);
    }
    // Output all the begins:
    
    int notBegins(0);            // Counter for when we don't see begins:
    
    for (int i =0; i < m_dataSources.size(); i++) {
        if (m_sources[i].s_pItem->type() == BEGIN_RUN) {
            outputFragment(i);                // Loads next item too.
        } else {
            notBegins++;
        }
    }
    if (notBegins) {
        std::cerr << "Warning " << notBegins << " data sources are missing begin run items\n";
    }
}

/**
 * outputOldest
 *    Locates the oldest item and outputs it:
 *    - State Change items are never oldest.
 *    - Sources with nulls for their items are never oldest.
 *    - Otherwise the oldest is determined by the smallest s_thisStamp.
 *  @note:
 *     readFragment takes care of assigning that for NULL_TIMESTAMP items so
 *     we don't have to worry about that case.
 *  @note:
 *     Data sources with an end run or at end of data will not be output.
 *     See atEnd and End below.
 */
void
CMerge::outputOldest()
{
    // Null timestamp is maxint for the size of the timestamp.
    
    std::uint64_t oldest        = NULL_TIMESTAMP;
    unsigned      oldestSource = 0;
    
    for (int i =0; i < m_dataSources.size(); i++) {
        if (m_sources[i].s_pItem && (m_sources[i].s_pItem->type() != END_RUN)) {
            if (m_sources[i].s_thisStamp < oldest) {
                oldest = m_sources[i].s_thisStamp;
                oldestSource = i;
            }
        }
    }
    
    // Now output the fragment from that queue:
    
    outputFragment(oldestSource);
}
/**
 * End
 *    At this point all of the sources either have an end item which is
 *    assumed to be the last item in the source or they have null items indicating
 *    we've hit the end of file whithout finding an end.
 *    -  Output all the ends.
 *    -  Report as a warning, the number of sources that don't have end items.
 *    @note atEnd will ensure that we have all ends or nulls and must have returned
 *          true before calling this.
 */
void
CMerge::End()
{
    unsigned endMissing(0);
    for (int i = 0; i < m_dataSources.size(); i++) {
        if (m_sources[i].s_pItem) {
            outputFragment(i);                 // We know this is an end.
        } else {
            endMissing++;
        }
    }
}
/**
 * atEnd
 *    Determines if we are at the end of the run.  We're there if all sources either
 *    have a null pointer for a ring item or an end run item.
 */
bool
CMerge::atEnd()
{
    for (int i = 0; i < m_dataSources.size(); i++) {
        if (m_sources[i].s_pItem && (m_sources[i].s_pItem->type() != END_RUN)) {
            return false;
        }
    }
    return true;
}
/**
 * outputFragment
 *    Outputs a fragment from a specific data source.  The fragment header
 *    is constructed from the ring item body header (all items are assumed to have
 *    body headers).  Once output readFragment is invoked to read the next
 *    fragment from the queue.  Note.  The timestamp is gotten from
 *    s_thisStamp as that's already corrected for NULL_TIMESTAMP items.
 *
 * @param sourceIndex - index in to m_sources  of the source we're otuputting.
 */
void
CMerge::outputFragment(unsigned sourceIndex)
{
    EVB::FragmentHeader header;
    pRingItemHeader pItem =
        reinterpret_cast<pRingItemHeader>(m_sources[sourceIndex].s_pItem->getItemPointer());
    header.s_timestamp = m_sources[sourceIndex].s_thisStamp;
    header.s_sourceId  = m_sources[sourceIndex].s_pItem->getSourceId();
    header.s_size      = pItem->s_size;
    header.s_barrier   = m_sources[sourceIndex].s_pItem->getBarrierType();
    
    // Output the fragment header and the ring item to the pipe:
    
    size_t nWritten    = fwrite(&header, sizeof(header), 1, m_pOutput);
    nWritten          += fwrite(pItem, pItem->s_size, 1, m_pOutput);
    
    if (nWritten != 2) {
        throw CErrnoException("Unable to write an item to the glom pipe");
    }
    readFragment(sourceIndex);              // Restock the source.
}
/**
 * readFragment
 *    Read a new Ring item from file.
 *    - Prior to doing the read s_pItem is deleted.
 *    - If the read fails, s_pItem is set to null, otherwise, it points to the
 *      item read.
 *    - If the item read has a NULL_TIMESTAMP, s_thisStamp <- s_lastStamp
 *      otherwise, s_thisStamp comes from the item.
 *
 *      @param sourceIndex - index of the source being read in both m_dataSources
 *                           and m_sources.
 *      
 */
void
CMerge::readFragment(unsigned sourceIndex)
{
    delete m_sources[sourceIndex].s_pItem;            // Kill off the old one.
    m_sources[sourceIndex].s_pItem = m_dataSources[sourceIndex]->getItem();
    
    if (m_sources[sourceIndex].s_pItem) {
        m_sources[sourceIndex].s_thisStamp = m_sources[sourceIndex].s_pItem->getEventTimestamp();
        if (m_sources[sourceIndex].s_thisStamp == NULL_TIMESTAMP) {
            m_sources[sourceIndex].s_thisStamp = m_sources[sourceIndex].s_lastStamp;
        } else {
            m_sources[sourceIndex].s_lastStamp  = m_sources[sourceIndex].s_thisStamp;
        }
    }
}