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

#include "swtriggerflags.h"
class CFullEventEditorApp;
#include "CFullEventEditor.h"
#include "CZMQFullEventEditorApp.h"

#ifdef HAVE_MPI
#include "CMPIFullEventEditorApp_mpi.h"
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
    CFullEventEditorApp* pApp(0);
    
    std::string strategy = parsed.parallel_strategy_arg;
        
    try {
        if (strategy == "threaded") {
            pApp = new CZMQFullEventEditorApp(parsed);
        } else if (strategy == "mpi") {
#ifdef HAVE_MPI
            pApp = new CMPIFullEventEditorApp(argc, argv, parsed);
#else
            throw std::invalid_argument("MPI parallelization not supported");
#endif
        } else {
            throw std::invalid_argument("Invalid --parallel-strategy value");
        }
        if (pApp) {
            exit((*pApp)());
        } else {
            throw std::logic_error("Application object not crated.");
        }
    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    delete pApp;
    exit(EXIT_SUCCESS);
}