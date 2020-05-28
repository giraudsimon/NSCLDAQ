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

#include "CMPIAppStrategy_mpi.h"

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

////////////////// Strategy object //////////////////////

class CMPIBuiltEditorStrategy : public CMPIAppStrategy
{
private:
    EditorFactory m_fact;
public:
    CMPIBuiltEditorStrategy(
        int argc, char** argv, gengetopt_args_info& args,
        EditorFactory fact
    );
    virtual CProcessingElement* createApplicationWorker(
        CFanoutClientTransport& source, CSender& sink, int id
    );
};

CMPIBuiltEditorStrategy::CMPIBuiltEditorStrategy(
    int argc, char** argv, gengetopt_args_info& args,
    EditorFactory fact
) :
    CMPIAppStrategy(argc, argv, args),
    m_fact(fact)
{}
CProcessingElement*
CMPIBuiltEditorStrategy::createApplicationWorker(
     CFanoutClientTransport& source, CSender& sink, int id

)
{
    Editor* pEditor = (*m_fact)();
    return new CBuiltRingItemEditor(
        source, sink, id, pEditor
    );
}

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
    CBuiltRingItemEditorApp(args),
    m_strategy(nullptr)
{
    auto factory = getEditorFactory();
    m_strategy  = new CMPIBuiltEditorStrategy(argc, argv, args, factory);
}
/**
 * destructor
 *    Nothing to do.
 */
CMPIBuiltRingItemEditorApp::~CMPIBuiltRingItemEditorApp()
{
    delete m_strategy;
}

/**
 * operator()
 *    This is a matter of selecting the right processing element
 *    to run and running it:
 */
void
CMPIBuiltRingItemEditorApp::operator()()
{
     (*m_strategy)();
    
}

