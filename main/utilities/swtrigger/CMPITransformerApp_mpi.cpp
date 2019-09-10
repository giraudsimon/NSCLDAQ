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

/** @file:  CMPITransformerApp_mpi.cpp
 *  @brief: Implement the MPI based parallel strategy for the data transformer app.
 */
#include "CMPITransformerApp_mpi.h"
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

#include "transformer.h"

#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>


/**
 * constructor
 *   - init MPI and make sure our size is appropriate.
 *
 * @param argc, argv - unprocessed command line parameters (MPI_Init needs).
 * @param args       - Processed args.
 */
CMPITransformerApp::CMPITransformerApp(
    int argc, char** argv, gengetopt_args_info& args
) :
    CTransformerApp(args)
{
    // We need at least 5 processes to operate (see
    // the header for the rank assigments).
    
    MPI_Init(&argc, &argv);
    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    if (nProcs < 4) {
        std::cerr << "This program needs at least 4 processes to run\n";
        throw std::invalid_argument("Too few processes");
    }
    // If the # workers consistent with n procs then warn he user
    // that the n procs overrides... only warn in rank 1:
    
    int workerCount = args.workers_arg;
    nProcs -= 3;
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
// Destructor is currently null:

CMPITransformerApp::~CMPITransformerApp() {}

/**
 * operator()
 *    Here we need to figure out, based on o ur rank, which processing
 *    element we are and run it until it exits at which point we finalize
 *    MPI and return.
 */
int
CMPITransformerApp::operator()()
{
    CProcessingElement* e = createProcessingElement();  // Figure out what we're running
    
    (*e)();
    delete e;
    MPI_Finalize();
    return EXIT_SUCCESS;
}
/**
 * createProcessingElement
 *     Based on rank, create and return the appropriate processing element
 *     object.
 */
CProcessingElement*
CMPITransformerApp::createProcessingElement()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    CProcessingElement* pResult(0);
    
    switch (rank) {
    case 0:
        pResult = createDataSource();
        break;
    case 1:
        pResult = createSorter();
        break;
    case 2:
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
CMPITransformerApp::createDataSource()
{
    return new CRingItemMPIDataSource(m_params.source_arg, m_params.clump_size_arg);
}

/**
 * createWorker
 *    Called to create a worker process.  This is called for ranks >
 *    3.  We create the a fanout client transport for rank 0  and
 *    associate it with a worker object.
 *    Note that the extender object needs to be gotten from the supplied
 *    library.
 */
CProcessingElement*
CMPITransformerApp::createWorker()
{
    // We need the classifier object the user provided:
    
    ExtenderFactory fact = getExtenderFactory();
    CBuiltRingItemExtender::CRingItemExtender* pExtender = (*fact)();
    
    // We need an MPI fanout client
    // a sink (to the sorter) a client id (our rank).
    // and we have the classifier.
    
    CFanoutClientTransport* pFanoutClient = new CMPIFanoutClientTransport(0);
    CTransport*             pSendingTransport = new CMPITransport(1);
    CSender*                pSender       = new CSender(*pSendingTransport);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // For my ID.
    
    CProcessingElement* pResult =
        new CBuiltRingItemExtender(*pFanoutClient, *pSender, rank-2, pExtender);
    
    return pResult;
}
/**
 * createSorter
 *    Called for rank 1 - creates a recipient that sorts data and pushes
 *    the sorted data to the sink.
 */
CProcessingElement*
CMPITransformerApp::createSorter()
{
    // Compute the number of workers:
    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    int nWorkers = nProcs - 3;        // Three non worker procs.
    
    // Make a receiver transport and a sender transport that sends
    // data to rank 2 (the sink).  Bind these into CReceiver and CSender objects:
    
    CMPITransport* pReceiverTransport = new CMPITransport();   // only receives.
    CMPITransport* pSenderTransport   = new CMPITransport(2);  // Sends to 2.
    
    CReceiver* pReceiver = new CReceiver(*pReceiverTransport);
    CSender*   pSender   = new CSender(*pSenderTransport);
    
    
    // Note the window parameter is obsolete.
    
    CProcessingElement* pResult = new CRingItemSorter(*pReceiver, *pSender, 0, nWorkers);
    return pResult;
}
/**
 * createSink
 *    Creates the data sink process.
 *    -  Create a transport to receive data from the sorter.
 *    -  Manufactures a sink from the ring tranport factory and the sink_arg
 *       specification in the parameters.
 *    -  Binds the transports into senders and receivers.
 *    -  Constructs a CRingBlockDataSink to use at the transport and returns that.
 */
CProcessingElement*
CMPITransformerApp::createSink()
{
    
    CMPITransport*  pReceiverTransport = new CMPITransport();
    CTransport*     pSenderTransport = CRingItemTransportFactory::createTransport(
        m_params.sink_arg, CRingBuffer::producer
    );
    
    CReceiver* pReceiver = new CReceiver(*pReceiverTransport);
    CSender*   pSender   = new CSender(*pSenderTransport);
    
    CProcessingElement* pResult = new CRingBlockDataSink(*pReceiver, *pSender);
    return pResult;
}