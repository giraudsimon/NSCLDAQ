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

/** @file: Main.cpp
 *  @brief: Program entry point
 */

#include "evts2frags.h"
#include "CEvents2Fragments.h"
#include "CFragmentMaker.h"
#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    // Process the command line parameters:
    
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);
    
    // Pull the parameters out of the args struct:
    
    int readBlocksize = args.read_blocksize_arg;
    int defaultSid    = args.default_sid_arg;
    int writeBlocksize = args.write_blocksize_arg;
    int writeTimeout   = args.write_timeout_arg;

    // Ok, create the objects that the app class needs:
    
    CRingFileBlockReader reader(STDIN_FILENO);
    io::CBufferedOutput  writer(STDOUT_FILENO, writeBlocksize);
    CFragmentMaker       fragMaker(defaultSid);
    
    // Now create and run the application class in a try catc block:
    
    try {
        // We need stdin to be non-blocking so we don't block on a partial
        // read at end of run.
        
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        flags |= O_NONBLOCK;
        //fcntl(STDIN_FILENO, F_SETFL, flags);
        
        CEvents2Fragments app(readBlocksize, reader, fragMaker, writer);
        app();
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (std::string msg) {
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (...) {
        std::cerr << "Unanticipated exception type caught\n";
        throw;   // Hopefully there's enough info from the catch all handler.
    }
    return EXIT_SUCCESS;
}