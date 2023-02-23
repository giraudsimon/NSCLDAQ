/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file   SoftwareTrigger.cpp
* @brief  Main program for the software trigger framwork.
*/

#include "swtriggerflags.h"
#include "CZMQThreadedClassifierApp.h"
#include <stdlib.h>
#include <string>
#include <iostream>

#ifdef HAVE_MPI
#include "CMPIClassifierApp_mpi.h"
#endif

/**
 *  The main just processes the program files, instantiates the
 *  application class and runs it.
 *
 * @param argc - number of command line parameters (includes invocation string).
 * @param argv - Command line parameters.
 */
int main(int argc, char** argv)
{
  gengetopt_args_info params;
  cmdline_parser(argc, argv, &params);
  std::string parallelizationStrategy = params.parallel_strategy_arg;
    
  if (parallelizationStrategy == "threaded") {
    CZMQThreadedClassifierApp app(params);
    exit(app());
  }  else if (parallelizationStrategy == "mpi") {
#ifdef HAVE_MPI
    CMPIClassifierApp app(argc, argv, params);
    int s = app();
    return(s);
#else
    std::cerr << "This version of SoftwareTrigger is not MPI enabled\n";
    return(EXIT_FAILURE);
#endif
  }
}
