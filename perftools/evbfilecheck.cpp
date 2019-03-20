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

/** @file:  evbfilecheck.cpp
 *  @brief: Check correctness of eventbuilder files
 */

/**
 *  This program makes _big_ assumptions.  The assumption is that:
 *  The timestamp ticks increment by 1 for each event,
 *  The dt for glom is 1.
 *  There are two data sources,
 *  All data sources are always in coincidence  except at the end where
 *  one source may run longer than the others and produce singles.
 *
 *  This program,
 *    - Reports the number of events that don't match that description
 *      which are followed by an event that does follow that description.
 *    -  Writes to file the events  that are like that.
 
*/
 
#include <CDataSource.h>              // Abstract source of ring items.
#include <CDataSourceFactory.h>       // Turn a URI into a concrete data source
#include <CRingItem.h>                // Base class for ring items.
#include <CPhysicsEventItem.h>        // Physics item.
#include <DataFormat.h>                // Ring item data formats.
#include <Exception.h>                // Base class for exception handling.
#include <CRingItemFactory.h>


// standard run time headers:

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>

static void
processRingItem(CRingItem& item);   // Forward definition, see below.


// This is the list of items that are 'not good'.

static std::vector<CPhysicsEventItem> badItems;
static std::ofstream badfile("badItems.txt");

/**
 * Usage:
 *    This outputs an error message that shows how the program should be used
 *     and exits using std::exit().
 *
 * @param o   - references the stream on which the error message is printed.
 * @param msg - Error message that precedes the usage information.
 */
static void
usage(std::ostream& o, const char* msg)
{
    o << msg << std::endl;
    o << "Usage:\n";
    o << "  readrings uri\n";
    o << "      uri - the file: or tcp: URI that describes where data comes from\n";
    std::exit(EXIT_FAILURE);
}


/**
 * The main program:
 *    - Ensures we have a URI parameter (and only a URI parameter).
 *    - Constructs a data source.
 *    - Reads items from the data source until the data source is exhausted.
 *
 *  @note Online ringbuffers are never exhausted.  The program will just block
 *        when the ringbuffer is empty until the ring has new data.
 *  @note Because of the note above, this function never exits for online sources.
 */

int
main(int argc, char** argv)
{
    // Make sure we have enough command line parameters.
    
    if (argc != 2) {
        usage(std::cerr, "Not enough command line parameters");
    }
    // Create the data source.   Data sources allow us to specify ring item
    // types that will be skipped.  They also allow us to specify types
    // that we may only want to sample (e.g. for online ring items).
    
    std::vector<std::uint16_t> sample;     // Insert the sampled types here.
    std::vector<std::uint16_t> exclude;    // Insert the skippable types here.
    CDataSource* pDataSource;
    try {
        pDataSource =
        CDataSourceFactory::makeSource(argv[1], sample, exclude);
    }
    catch (CException& e) {
        usage(std::cerr, "Failed to open ring source");
    }
    // The loop below consumes items from the ring buffer until
    // all are used up.  The use of an std::unique_ptr ensures that the
    // dynamically created ring items we get from the data source are
    // automatically deleted when we exit the block in which it's created.
    
    CRingItem*  pItem;
    while ((pItem = pDataSource->getItem() )) {
        std::unique_ptr<CRingItem> item(pItem);     // Ensures deletion.
        processRingItem(*item);
    }
    // We can only fall through here for file data sources... normal exit
    
    std::exit(EXIT_SUCCESS);
}

bool goodItem(CPhysicsEventItem& item)
{
    return true;
}
void dumpBadItems()
{
    
}
void checkBadItemsAtEndRun()
{
    
}

/**
 * processRingItem.
 *    Modify this to put whatever ring item processing you want.
 *    In this case, we just output a message indicating when we have a physics
 *    event.  You  might replace this with code that decodes the body of the
 *    ring item and, e.g., generates appropriate root trees.
 *
 *  @param item - references the ring item we got.
 */
static void
processRingItem(CRingItem& item)
{
    if (item.type() == PHYSICS_EVENT) {
        CPhysicsEventItem* pItem =
            dynamic_cast<CPhysicsEventItem*>(CRingItemFactory::createRingItem(item));
        if (!goodItem(*pItem)) {
            badItems.push_back(*pItem);
        } else {
            if (badItems.size()) {
                dumpBadItems();
                badItems.clear();
            }
        }
    }
    // At the end of the run, we must ensure that all 'bad' items only have
    // contributions from one event source and that this source is the same for
    // all 'bad items'.
    
    if (item.type() == END_RUN) {
        checkBadItemsAtEndRun();
    }
    
}