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

/** @file:  writerings
 *  @brief: cookbook that shows how to write ringbuffers to file or to a ring.
 */


/**
 *
 *  This program build on the example of readrings.  It accepts a second parameter
 *  that specifies where ring items will be written.
 *  processRingItem will then take a data sink and only put physics items
 *  to this sink.  
 */

// header files:

// NSCLDAQ Headers:

#include <CDataSource.h>              // Abstract source of ring items.
#include <CDataSourceFactory.h>       // Turn a URI into a concrete data source
#include <CDataSink.h>                // Abstract sink of ring items.
#include <CDataSinkFactory.h>         //  Turn a URI into a concrete data sink.
#include <CRingItem.h>                // Base class for ring items.
#include <DataFormat.h>                // Ring item data formats.
#include <Exception.h>                // Base class for exception handling.

// standard run time headers:

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>

static void
processRingItem(CDataSink& sink, CRingItem& item);   // Forward definition, see below.

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
    o << "  readrings input-uri output-uri\n";
    o << "      input-uri - The file: or tcp: URI that describes where data comes from\n";
    o << "                   Note that the special value '-' makes the source get data from\n";
    o << "                   standard input.\n";
    o << "      output-uri - The file: or tcp: URI that describes where data will be written\n";
    o << "                   If the URI is a tcp: uri, the host part of the URI must either be\n";
    o << "                   empty or 'localhost\n";
    o << "                   Note that the special valu '-' makes the source put data to\n";
    o << "                   standard output\n";
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
    
    if (argc != 3) {
        usage(std::cerr, "Incorrect number of command line parameters");
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
        std::cerr << "Failed to open ring source: ";
        usage(std::cerr, e.ReasonText());
    }
    // Create a data sink that can be passed to the data processor.
    // this is wrappedn in an std::unique_ptr to ensure if an exception
    // stops the program it the sink is properly destructed (flushing and closing it).
    //
    
    CDataSink* pSink;
    try {
        CDataSinkFactory factory;
        pSink = factory.makeSink(argv[2]);
    }
    catch (CException& e) {
        std::cerr << "Failed to create data sink: ";
        usage(std::cerr, e.ReasonText());
    }
    std::unique_ptr<CDataSink> sink(pSink);
    
    // The loop below consumes items from the ring buffer until
    // all are used up.  The use of an std::unique_ptr ensures that the
    // dynamically created ring items we get from the data source are
    // automatically deleted when we exit the block in which it's created.
    
    CRingItem*  pItem;
    while ((pItem = pDataSource->getItem() )) {
        std::unique_ptr<CRingItem> item(pItem);     // Ensures deletion.
        processRingItem(*sink, *item);
    }
    // We can only fall through here for file data sources... normal exit
    
    std::exit(EXIT_SUCCESS);
}
/**
 * processRingItem.
 *    Modify this to put whatever ring item processing you want.
 *    In this case, we just output a message indicating when we have a physics
 *    event.  You  might replace this with code that decodes the body of the
 *    ring item and, e.g., generates appropriate root trees.
 *
 *  @param sink - references the data sink to which ring items are written.
 *  @param item - references the ring item we got.
 */
static void
processRingItem(CDataSink& sink, CRingItem& item)
{
    if (item.type() == PHYSICS_EVENT) {
        sink.putItem(item);
    }
}