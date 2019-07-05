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

/** @file:  CMPIClassifierApp.cpp
 *  @brief: Implement the MPI classifier application setup code.
 */
#include "CMPIClassifierApp_mpi.h"
#include "CMPITransport_mpi.h"
#include "CMPIFanoutTransport_mpi.h"
#include "CMPIFanoutClientTransport_mpi.h"

#include "CRingItemTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CRingItemSorter.h"
#include "CDataSinkElement.h"
#include "CRingItemTransportFactory.h"
#include "CRingBlockDataSink.h"

#include "swtriggerflags.h"


#include <mpi.h>
#include <stdexcept>

/**
 * constructor
 *    We're just going to save the parameters (shallow copy)
 *    and initialize MPI.  operator() does the lion's share of the work
 *  @param argv - comand line parameters.
 *  @param argc  - command line parameter count.
 *  @param args  - processed args thanks t gengetopt.
 */

CMPIClassifierApp::CMPIClassifierApp(
    int argc, char** argv, gengetopt_args_info& args
) :  CClassifierApp(args)
{
    MPI_Init(&argc, &argv);
    
    // We need at least 5 processes to operate (see
    // the header for the rank assigments).
    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    if (nProcs < 5) {
        std::cerr << "This program needs at least 5 processes to run\n";
        throw std::invalid_argument("Too few processes");
    }
    
}
/**
 * destructor
 */
CMPIClassifierApp::~CMPIClassifierApp() {}

/**
 * operator()
 *    Called to run the operation. For MPI, each process run one operation.
 *    Our job, therefore, is to select which operation to run,
 *    set up its transports, instantiate it and run it.
 */

int
CMPIClassifierApp::operator()()
{
    
}