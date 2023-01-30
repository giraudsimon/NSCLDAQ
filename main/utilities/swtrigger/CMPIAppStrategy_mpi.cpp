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

/** @file:  CMPIAppStrategy_mpi.cpp
 *  @brief: Implement the implemented methods of the MPI app strategy.
 */
#include "CMPIAppStrategy_mpi.h"

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
#include <iostream>

/**
 * constructor
 *   
 *  -   Construct a reference to the parsed parameters,
 *  -   Initialize MPI,
 *  -   Check that the size of the application meets minimum
 *      requirements.
 *  -   If the appliction size is inconsistent with th e requested
 *      workers emit a message to that effect.
 *  
 *  @param argv - comand line parameters.
 *  @param argc  - command line parameter count.
 *  @param args  - processed args thanks t gengetopt.
 */
CMPIAppStrategy::CMPIAppStrategy(int argc, char** argv, gengetopt_args_info& args) :
    m_args(args)
{
    MPI_Init(&argc, &argv);
    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    
    // We need at least 4 processes to operate (see
    // the header for the rank assigments).
    
    if (nProcs < 4) {
        std::cerr << "This program needs at least 4 processes to run\n";
        throw std::invalid_argument("Too few processes");
    }
    
    // If the # workers consistent with n procs then warn he user
    // that the n procs overrides... only warn in rank 1:
    
    int workerCount = m_args.workers_arg;
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
    
     // Note that this change in worker count is automatically
     // handled at run time by the selection of which ranks perform
     // which jobs.
}
/**
 * at present we have nothing to destroy:
 */
CMPIAppStrategy::~CMPIAppStrategy() {}

/**
 * operator()
 *    The MPI applications we build create concrete instances
 *    of processing elements based on the process rank
 *    and just run them. createProcessingElement does the
 *    element selection/creation and communication set up for
 *    each of the elements.  We just run the object,
 *    delete it when done and finalize MPI.
 */
int
CMPIAppStrategy::operator()()
{
    CProcessingElement* e = createProcessingElement();  // Figure out what we're running
    
    // Now run our piece of the app.
    
    (*e)();
    delete e;
    MPI_Finalize();
    return EXIT_SUCCESS;
}
/**
 * createProcessingElement
 *    This method creates the processing element executed by this
 *    MPI rank.   The ranks are assigned to elements as follows:
 *
 *    -   0 is a data source.  The actual data source
 *        depend on command line options.
 *    -   1 is the data sorter. It gets the data as transformed by the
 *        worker processes and reorders them by timestamp so that the
 *        resulting data are back in timestamp order.
 *    -   2 is the data sink.  It opens a data sink depending on
 *        the command line parameters, accepts sorted data from the
 *        sorter and outputs it to the sink.
 *    -  all others are worker processes towhich the data source
 *       fans out work elements.
 * @return CProcessingElement* - pointer to a dynamically
 *       created processing element to execute in this process.
 */
CProcessingElement*
CMPIAppStrategy::createProcessingElement()
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
 *    Creates the data source (rank 0) processing element.
 *    The source of the data are determined by the URI in the
 *    source_arg field of the parsed arguments.  The work item
 *    size (number of ring items in a work item) is determined by
 *    the clump_size_arg of the same struct.
 *  @return CProcessingElement*
*/
CProcessingElement*
CMPIAppStrategy::createDataSource()
{
    return new CRingItemMPIDataSource(m_args.source_arg, m_args.clump_size_arg);
}
/**
 * createWorker
 *   While the worker is actually very application specific,
 *   the communication harness to the worker is not.
 *   -  We create a client transport to the data source fanout.
 *   -  We create a transport to what will ultimately be the
 *      sorter.
 *   -  We compute the id of the worker from our actual rank.
 *   We then ask createApplicationWorker to gift us with a worker
 *   tied into these things. Note that createApplicationWorker is
 *   pure virtual and must be supplied by a concrete subclass of
 *   this class.
 *   
 * @return CProcessingElement*
 */
CProcessingElement*
CMPIAppStrategy::createWorker()
{
// We need an MPI fanout client
    // a sink (to the sorter) a client id (our rank).
    // and we have the classifier.
    
    CFanoutClientTransport* pFanoutClient =
        new CMPIFanoutClientTransport(0);
    CTransport*  pSendingTransport = new CMPITransport(1);
    CSender*  pSender       = new CSender(*pSendingTransport);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // For my ID.
    
    return createApplicationWorker(*pFanoutClient, *pSender, rank-2);    
}
/**
 * createSorter
 *    Creates a timestamp re-sorter and hooks it into the
 *    data flow.  We'll get data from the array of workers,
 *    sort it and output it in a pipeline to the
 *    data sink.  We execute in rank 1 of the program.
 * @return CProcessingElement*
 */
CProcessingElement*
CMPIAppStrategy::createSorter()
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
 *    Creates a sink to the destination specified in m_args.sink_arg.
 *    The data for our sink is the other end of the sorter output
 *    pipeline.
 *
 * CProcessingElement*
 */
CProcessingElement*
CMPIAppStrategy::createSink()
{
    CMPITransport*  pReceiverTransport = new CMPITransport();
    CTransport*     pSenderTransport = CRingItemTransportFactory::createTransport(
        m_args.sink_arg, CRingBuffer::producer
    );
    
    CReceiver* pReceiver = new CReceiver(*pReceiverTransport);
    CSender*   pSender   = new CSender(*pSenderTransport);
    
    CProcessingElement* pResult = new CRingBlockDataSink(*pReceiver, *pSender);
    return pResult;
}
