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

/** @file:  ddasSort.cpp
 *  @brief: Main program to sort DDAS Hits.
 */

/**
 *  In order to get best performance from DDASReadout, sorting of the
 *  resulting hits has been pushed downstream.  This program reads
 *  ring items that consist of multiple hits from boards and outputs
 *  ring items that consist of single hits from all boards in an experiment
 *  sorted by time.  Ring item bodies we take as input look like:
 *
 *  +------------------------------+
 *  |  uint32_t # bytes of data    |
 *  +------------------------------+
 *  | uint32_t module type word    |
 *  +------------------------------+
 *  | hit1 | hit2 ....             |
 *               ...
 */

#include "ddasSortOptions.h"
#include "DDASSorter.h"

#include <CRingBuffer.h>
#include <CRemoteAccess.h>
#include <Exception.h>


#include <stdexcept>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <memory>
/**
 * main
 *    Entry point, this function processes command line arguments
 *    that are parsed by gengetopt and instantiates and invokes the
 *    application class with appropriate parameters.
 */

int main(int argc, char**argv)
{
    gengetopt_args_info parsedArgs;
    cmdline_parser(argc, argv, &parsedArgs);
    
    std::string sourceURI = parsedArgs.source_arg;
    std::string sinkRing  = parsedArgs.sink_arg;
    float       accumWindow = parsedArgs.window_arg;
    
    int status = EXIT_SUCCESS;
    std::string errorMessage;
    
    // Create the CRingBuffer objects for the source (which could specify
    // a remote ring) and sink (which must be a local ring).
    
    try {
        // Using unique pointers below ensures cleanup regardless how we
        // exit (e.g. including exceptions).
        
        std::unique_ptr<CRingBuffer> pSource(CRingAccess::daqConsumeFrom(sourceURI));
        std::unique_ptr<CRingBuffer> pSink(CRingBuffer::createAndProduce(sinkRing));
        
        DDASSorter sorter(*pSource, *pSink, accumWindow);
        sorter();
        
    }
    catch (CException& e) {
        errorMessage = "NSCLDAQ exception caught: ";
        errorMessage += e.ReasonText();
        status = EXIT_FAILURE;
    }
    catch (std::exception& e) {
        errorMessage = "C++ library exception caught: ";
        errorMessage += e.what();
        status = EXIT_FAILURE;
    }
    catch (std::string msg) {
        errorMessage = "std::string exception caught: ";
        errorMessage += msg;
        status = EXIT_FAILURE;
    }
    catch (const char* msg) {
        errorMessage = "const char* exception caught: ";
        errorMessage += msg;
        status = EXIT_FAILURE;
    }
    catch (...) {
        errorMessage = "Unanticpated exception type caught";
        throw;               // Maybe C++ lib will tell us the type.
    }
    
    
    if (status != EXIT_SUCCESS) {
        std::cerr << errorMessage;
    }
    exit(status);
}