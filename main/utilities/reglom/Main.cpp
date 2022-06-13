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

/** @file:  Main.cpp
 *  @brief: Driver function for unglom and timecheck:
 */

// CDataSource Factory creates a data source for file or online from a URI.
// CDataSource is the base class for data sources.
//  docs.nscl.msu.edu/daq/newsite/nscldaq-11.2/index.html has detailed
// documentation of all the classes we're going to use.

#include <CDataSourceFactory.h>
#include <CDataSource.h>
#include <DataFormat.h>                    // Defines ring item types inter alia.
#include <CRingItem.h>                     // Base class for all ring items.


#include "CRingItemDecoder.h"              // Sample code.

    
// Includes that are standard c++ things:

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <errno.h>

/* Note that not all errno.h's define ESUCCESS so:  */

#ifndef ESUCCESS
#define ESUCCESS 0
#endif
static char* program;
/**
 *  usage:
 *     Reports usage information for the program to stderr
 *     We accept a single parameter.  The URI of the data source.
 */
void
usage()
{
    std::cerr << "Usage\n";
    std::cerr << "    " << program << "  data-source-uri\n";
    std::cerr << "Where:\n";
    std::cerr << "   data-source-uri is the URI for a file or ringbuffer that\n";
    std::cerr << "                   data will be read from\n";
}

/**
 * main
 *    Entry point for the program -- the usual command parameters.
 */



int
Main(int argc, char**argv, CRingItemDecoder& decoder)
{
    program = argv[0];
    // We need to have a URI and only a URI:
    
    if (argc != 2) {
        usage();
        std::exit(EXIT_FAILURE);
    }
    
    /**
     *  Get the data source URI and use it to create a data source.
     *  our error handling is going to be a bit primitive.  We could catch the
     *  specific exception types and report something a bit more detailed on failure
     */
    
    std::string uri(argv[1]);
    std::vector<uint16_t> sample = {PHYSICS_EVENT};   // means nothing from file.
    std::vector<uint16_t> exclude;                    // get all ring item types:
    
    CDataSource* pSource;
    try {
        pSource = CDataSourceFactory::makeSource(uri, sample, exclude);
    }
    catch (...) {
        std::cerr << "Failed to open the data source.  Check that your URI is valid and exists\n";
        std::exit(EXIT_FAILURE);
    }
    
    CDataSource& source(*pSource);
    


    // Now we're ready to accept ring items from the data source.
    // Note that CDataSource::getItem will return a pointer to a
    // dynamically allocated ring item received from the source.
    // -  An int exception is thrown for most errors.
    // -  End of source (e.g. file) is indicated by a null pointer
    //    and errno of ESUCCESS;  Note that ESUCCESS  is not POSIX but
    //    is defined under Linux (it's 0).
    try {
        CRingItem* pItem;
        while (pItem = source.getItem()) {
            decoder(pItem);
            delete pItem;
        }
        // If errno is ESUCCESS we're done, otherwise, just throw the errno
        //  
        if (errno != ESUCCESS) throw errno;
        decoder.onEndFile();        
        std::exit(EXIT_SUCCESS);
    }
    catch (int errcode) {
        
        std::cerr << "Ring item read failed: " << std::strerror(errcode) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (std::string msg) {
        std::cout << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // Code should not fall through to here so:
    
    std::cerr <<"BUG - control fell to the bottom of the main\n";
    std::exit(EXIT_FAILURE);
}
    
    
