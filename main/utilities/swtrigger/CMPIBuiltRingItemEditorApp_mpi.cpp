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

/** @file:  CMPIBuiltRingItemEditorApp_mpi.cpp
 *  @brief: Implement the MPI version of the app.
 */

#include "CMPIBuiltRingItemEditorApp_mpi.h"

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

#include "eventeditor.h"

#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>

/**
 * constructor
 *   Note that if the COMM_WORLD communicator has a size that's
 *   inconistent with the nmber of workesr we warn that the
 *   comm size overrides.  Furthermore if there are not enough
 *   processes to even have one worker we need to abort.
 *
 *   @param argc - number of command line parametesr.
 *   @param argv - The command line parameters
 *   @param args - The parsed argument struct.
 */
CMPIBuiltRingItemEditorApp::CMPIBuiltRingItemEditorApp(
    int argc, char** argv, gengetopt_args_info& args
) :
    CBuiltRingItemEditorApp(args)
{
    // We need at least 5 processes to operate (see
    // the header for the rank assigments).
    
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Only rank 0 emits errors.

    
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    if ((nProcs < 4) && (rank == 0)) {
        std::cerr << "This program needs at least 4 processes to run\n";
        MPI_Finalize();          // We're done.
        throw std::invalid_argument("Too few processes");
    
    }
    // If the # workers consistent with n procs then warn he user
    // that the n procs overrides... only warn in rank 1:
    
    int workerCount = args.workers_arg;
    nProcs -= 3;
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
 *    Nothing to do.
 */
CMPIBuiltRingItemEditorApp::~CMPIBuiltRingItemEditorApp()
{}

/**
 * operator()
 *    This is a matter of selecting the right processing element
 *    to run and running it:
 */
void
CMPIBuiltRingItemEditorApp::operator()()
{
    CProcessingElement* e = createProcessingElement();
    
    (*e)();
    delete e;
    MPI_Finalize();
    
}
/**
 * createProcessingElement
 *    Creates and returns the appropriate processing
 *    element for our rank
 *
 *   * rank 0 is the data source.
 *   * rank 1 is the sorter.
 *   * rank 2 is the outputter
 *   * All other ranks are workers.
 *
 * @return CProcessingElement*  - pointer to new'd processing
 *         element appropriate to our process rank.
 */
CProcessingElement*
CMPIBuiltRingItemEditorApp::createProcessingElement()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    CProcessingElement* pResult;
    
    switch(rank) {
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
 *    Creates the ultimate datga source.  Note that
 *    args.source_arg specifies the URI of the source and
 *    args.clump_size_arg the work item clump size.
 *
 *  @return CProcessingElement* - the processing element of a ring data src.
 *  
 */
CProcessingElement*
CMPIBuiltRingItemEditorApp::createDataSource()
{
    return new CRingItemMPIDataSource(
        m_args.source_arg, m_args.clump_size_arg
    );
}
/**
 * createSorter
 *    Creates a processing element that sorts incoming items
 *    by timestamp.
 *    
 *  @return CProcessingElement*  - Pointer to a newly created processing
 *          element.
 */
CProcessingElement*
CMPIBuiltRingItemEditorApp::createSorter()
{
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
 *    Create the ultimate sink.
 *    m_args.sink_arg is the URI of the data sink.
 *
 * @return CProcessingElement* pointer to the processing element.
 */
CProcessingElement*
CMPIBuiltRingItemEditorApp::createSink()
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
/**
 * createWorker
 *    -  Gets the editor factory and creates a user editor object.
 *    -  Creates the worker, configures its communication and
 *
 *  @return CProcessingElement* - pointer to the worker element,.
 */
CProcessingElement*
CMPIBuiltRingItemEditorApp::createWorker()
{
    EditorFactory fact = getEditorFactory();
    CBuiltRingItemEditor::BodyEditor* pEditor = (*fact)();
    
    // We need an MPI fanout client
    // a sink (to the sorter) a client id (our rank).
    // and we have the classifier.
    
    CFanoutClientTransport* pFanoutClient =
        new CMPIFanoutClientTransport(0);
    CTransport*             pSendingTransport = new CMPITransport(1);
    CSender*                pSender       =
        new CSender(*pSendingTransport);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // For my ID.
    
    CProcessingElement* pResult =
        new CBuiltRingItemEditor(
            *pFanoutClient, *pSender, rank-2, pEditor
        );
    
    return pResult;
}