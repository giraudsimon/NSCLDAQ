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

#include "CMPIFullEventEditorApp_mpi.h"
#include "CMPIAppStrategy_mpi.h"
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

#include "swtriggerflags.h"

#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>

///////////////////// Strategy pattern //////////////////////


class CMPIFullEditorStrategy : public CMPIAppStrategy
{
private:
    CMPIFullEventEditorApp* m_pApp;
public:
    CMPIFullEditorStrategy(
        int argc, char** argv, gengetopt_args_info& args,
        CMPIFullEventEditorApp* app
    );
    virtual CProcessingElement* createApplicationWorker(
        CFanoutClientTransport& source, CSender& sink, int id
    );
};
CMPIFullEditorStrategy::CMPIFullEditorStrategy(
    int argc, char** argv, gengetopt_args_info& args,
    CMPIFullEventEditorApp* app
) :
    CMPIAppStrategy(argc, argv, args),
    m_pApp(app)
{}
CProcessingElement*
CMPIFullEditorStrategy::createApplicationWorker(
    CFanoutClientTransport& source, CSender& sink, int id
)
{
    
    CFullEventEditor::Editor* pEditor = m_pApp->createUserEditor();
    return new CFullEventEditor(source, sink, id, pEditor);
}
/**
 * constructpor
 *    Constructs the app by constructing the base class.
 *    We also initialize MPI and figure out how many workers we'll have
 *    compared with what was requested on the command line.
 *
 * @param argc, argv - From main - the raw, unprocessed command line parameters.
 * @param args       - the gengetopt processed arguments.
 */
CMPIFullEventEditorApp::CMPIFullEventEditorApp(
    int argc, char** argv, gengetopt_args_info& args
) :
    CFullEventEditorApp(args),
    m_strategy(nullptr)
{
    m_strategy = new CMPIFullEditorStrategy(argc, argv, args, this);
}
/**
 * destructor
 *    Currently there's no need for local destruction.
 */
CMPIFullEventEditorApp::~CMPIFullEventEditorApp()
{
    delete m_strategy;    
}
/**
 * operator()
 *   Use createProcessingElement to get the processing element appropriate to our
 *   rank/role -- then run it.
 */
int
CMPIFullEventEditorApp::operator()()
{
   return (*m_strategy)();
}
