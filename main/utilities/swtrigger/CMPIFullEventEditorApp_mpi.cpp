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

/** @file:  CMPIFullEventEditorApp_mpi.cpp
 *  @brief: Implement the application using MPI transport and process model.
 */

#inlcude "CMPIFullEventEditorApp_mpi.h"
#include "CProcessingElement.h"

#include "CFullEventEditor.h"

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

#include "fulleventeditor.h"

#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>

/**
 * constructpor
 *    Constructs the app by constructing the base class.
 *    We also initialize MPI and figure out how many workers we'll have
 *    compared with what was requested on the command line.
 *
 * @param argc, argv - From main - the raw, unprocessed command line parameters.
 * @param args       - the gengetopt processed arguments.
 */
CMPIFullEditorApp::CMPIFullEventEditorApp(
    int argc, char** argv, gengetopt_args_info& args
) :
    CFullEventEditorApp(args)
{
    MPI_Init(argc, argv);
    size_t nWorkers = getWorkerCount();         // Error messagesexit if appropriate.
}
/**
 * destructor
 *    Currently there's no need for local destruction.
 */
CMPIFullEventEditorApp::~CMPIFullEventEditorApp()
{
    
}
/**
 * operator()
 *   Use createProcessingElement to get the processing element appropriate to our
 *   rank/role -- then run it.
 */
int
CMPIFullEventEditorApp::operator()()
{
    CProcessingElement* e = createProcessingElement();
    (*e)();
    delete e;
    
    MPI_Finalize();                        // Includes the end barrier.
    return EXIT_SUCCESS;
}
/**
 * createProcessingElement
 *    Based on my rank in the application, create and return the correct
 *    processing element.
 * @return CProcessingElement* pointer to a dynamically (new) allocated
 *         processing element to run.
 */
CProcessingElement*
CMPIFullEventEditorApp::createProcessingElement()
{
    // Figure out who I am:
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    CProcessingElement* pResult(0);
    
    // Based on my rank, create the proper processing element:
    
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
        default:                          // all others are workers:
            pResult = createWorker();
            break;
    }
    
    return pResult;
}
/**
 * createDataSource
 *    Create a fanout data source processing element.  Note that the
 *    base class's m_params has the source URI.
 */
CProcessingElement*
CMPIFullEventEditorApp::createDataSource()
{
    return new CRingItemMPIDataSource(
        m_params.source_arg, m_params.clump_size_arg
    );
}
/**
 * createWorker
 *    - Create a user editor.
 *    - Create in and out transports and wrpa that into a CFullEventEditor object
 *      which we return to the user.
 */
CProcessingElement*
CMPIFullEventEditorApp::createWorker()
{
    CFullEventEditor::Editor* pEditor = createUserEditor();  // Base class knows how.
    
    // Our in/out transports:
    
    CFanoutClientTransport* pFanoutClient = new CMPIFanoutClientTransport(0);
    CTransport*             pSendingTransport = new CMPITransport(1);
    CSender*                pSender       = new CSender(*pSendingTransport);
    
    // Who we are as that's our fanout id:
    
    int rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    CProcessingElement* pResult =
        new CFullEventEditor(pFanoutClient, *pSender, rank-2, pEditor))
    return pResult;
}
/**
 * createSorter
 *    Create the timestamp re-sorting processor.
 */
CProcessingElement*
CMPIFullEventEditorApp::createSorter()
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
 *    Create the data sink.  The ring item data sink itself can come from
 *    the base class's createRingSink.
 */
CProcessingElement*
CMPIFullEventEditorApp::createSInk()
{
    
    CMPITransport*  pReceiverTransport = new CMPITransport();
    CTransport*     pSenderTransport = createRingSink();
    );
    
    CReceiver* pReceiver = new CReceiver(*pReceiverTransport);
    CSender*   pSender   = new CSender(*pSenderTransport);
    
    CProcessingElement* pResult = new CRingBlockDataSink(*pReceiver, *pSender);
    return pResult;    
}
/**
 * getWorkerCount
 *    Figure out the number of workers.
 *    - throws an invalid_argument exception if the computation size is not at
 *      least 4 (one worker).
 *    - If we are rank 0 and the number of workers is not consistent with
 *      --workers outputs an error to std::cerr.
 *
 *  @returns the actual number of workers.
 */
size_t
CMPIFullEventEditorApp::getWorkerCount()
{
    // Figure out the computation size:
    
    int nProcesses;
    int myrank;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    
    // We need at least four:
    
    if (nProcesses < 4) {
        if(myrank == 0) {
            std::cerr << "You must have -np at least 4 to support at least one worker\n";
        }
        throw std::invalid_argument("Too few processes - need mpirun -np at least 4.")
    }
    
    size_t nWorkers = nProcesses - 3;  // Source, sorter and sink are three.
    
    if ((nWorkers != m_params.workers_arg) && (myrank == 0)) {
        std::cerr << "The number of workers computed from -np (-np - 3) differs from --workers\n";
        std::cerr << "-np implies " << nWorkers << " while --workers requested "
            << nWorkers << " workers " << std::endl;
        std::cerr << "The run will continue using " << nWorkers << " worker processes\n";
    }
    return nWorkers;
}