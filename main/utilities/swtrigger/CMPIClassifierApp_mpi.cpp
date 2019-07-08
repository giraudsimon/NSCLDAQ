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
#include "CRingItemMPIDataSource_mpi.h"
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
#include <stdlib.h>

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
    // We need at least 5 processes to operate (see
    // the header for the rank assigments).
    
    MPI_Init(&argc, &argv);
    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    if (nProcs < 5) {
        std::cerr << "This program needs at least 5 processes to run\n";
        throw std::invalid_argument("Too few processes");
    }
    // If the # workers consistent with n procs then warn he user
    // that the n procs overrides... only warn in rank 1:
    
    int workerCount = args.workers_arg;
    nProcs -= 4;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if ((nProcs != workerCount) && (rank == 0)) {
        std::cerr << "**WARNING** number of MPI processes is not consistent with ";
        std::cerr << " the number of workers \n" << " requested in --workers\n";
        std::cerr << " --workers = " << workerCount;
        std::cerr << " MPI processes supports " << nProcs << " workers\n";
        std::cerr << nProcs << " workers will be used\n";
        
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
    CProcessingElement* e = createProcessingElement();  // Figure out what we're running
    
    // Do we neeed a delay here?
    
    (*e)();
    delete e;
    MPI_Finalize();
    return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////
// createProcessingElement

CProcessingElement*
CMPIClassifierApp::createProcessingElement()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    CProcessingElement* pResult(0);
    
    switch (rank) {
    case 0:
        pResult = createDataSource();
        break;
    case 1:
        pResult = createFanout();
        break;
    case 2:
        pResult = createSorter();
        break;
    case 3:
        pResult = createSink();
        break;
    default:
        pResult = createWorker();
        break;
    }
    
    return pResult;
}
/**
 * createDataSource
 *    Called for rank 0 - this sets up the initial data source
 *    that sends blocks of ring items to workers.
 */
CProcessingElement*
CMPIClassifierApp::createDataSource()
{
    return new CRingItemMPIDataSource(m_params.source_arg, m_params.clump_size_arg);
}