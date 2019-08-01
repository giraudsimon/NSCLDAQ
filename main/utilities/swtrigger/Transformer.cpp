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

/** @file:  Transformer.cpp
 *  @brief: Main program for the transformer program.
 */

#include "transformer.h"
#include <stdlib.h>
#include <string>
#include <iostream>

/**
 * The main program, like all of our main programs just parses
 * arguments, selects which application class to instantiate,
 * instantiates one and runs it.
 *
 * @param argc - number of command words.
 * @param argv - array of pointers to command words.
 * @note The gengetopt file transformer.ggo is used to drive parameter
 *       processing.
 */
int main(int argc, char** argv)
{
    gengetopt_args_info params;
    cmdline_parser(argc, argv, &params);
    std::string parallelizationStrategy = params.parallel_strategy_arg;
    
    if (parallelizationStrategy == "threaded") {
        std::cerr << "Threaded parallelization app still has to be written\n";
    } else if (parallelizationStrategy == "mpi") {
#ifdef HAVE_MPI
        std::cerr << "MPI parallelization app still has to be written\n";
#else
        std::cerr << "This version of NSCLDAQ is not MPI enabled\n";
        return EXIT_FAILURE;
#endif
    }
    
    return EXIT_SUCCESS;
}