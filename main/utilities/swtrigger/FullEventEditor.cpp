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

/** @file:  FullEventEditor.cpp
 *  @brief: Main program for the full event editor application.
 */

#include "fulleventeditor.h"
class CFullEventEditorApp;
#include "CFullEventEditor.h"

#ifdef HAVE_MPI
#endif


#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <errno.h>

typedef CFullEventEditor::Editor* (*createEditor)();



/**
 * main
 *    Entry point for the full event editor:
 *    -  Parse the arguments.
 *    -  Load the editor from the user's editor library.
 *    -  Based on the parallel strategy, instantiate  a ZMQ or an MPI
 *      (if enabled) App.
 *    - Run the app.
 */
int main(int argc, char**argv)
{
    gengetopt_args_info parsed;
    cmdline_parser(argc, argv, &parsed);
    CFullEventEditorApp* pApp;
    
    std::string strategy = parsed.parallel_strategy_arg;
        
    try {
        if (strategy == "zmq") {
            throw std::invalid_argument("ZMQ application not yet implemented");
        } else if (strategy == "mpi") {
#ifdef HAVE_MPI
            throw std::invalid_argument("MPI Application not yet implemented");
#else
            throw std::invalid_argument("MPI parallelization not supported");
#endif
        } else {
            throw std::invalid_argument("Invalid --parallel-strategy value");
        }
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}