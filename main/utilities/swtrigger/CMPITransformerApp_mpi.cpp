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



#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>


/////////////// Define/implement our strategy object //////////
class CMPITransformerStrategy : public CMPIAppStrategy
{
private:
    ExtenderFactory m_factory;
public:
    CMPITransformerStrategy(
        int argc, char** argv, gengetopt_args_info& args, ExtenderFactory fact
    );
    virtual ~CMPITransformerStrategy() {}
    
    virtual CProcessingElement* createApplicationWorker(
        CFanoutClientTransport& source, CSender& sink, int id
    );

    
};
CMPITransformerStrategy::CMPITransformerStrategy(
    int argc, char** argv, gengetopt_args_info& args, ExtenderFactory fact
) :
    CMPIAppStrategy(argc, argv, args),
    m_factory(fact)
{}
CProcessingElement*
CMPITransformerStrategy::createApplicationWorker(
     CFanoutClientTransport& source, CSender& sink, int id
)
{
    CBuiltRingItemExtender::CRingItemExtender* pExtender = (*m_factory)();
    return new CBuiltRingItemExtender(source, sink, id, pExtender);
}

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
    CTransformerApp(args), m_strategy(nullptr)
{
    auto factory = getExtenderFactory();
    m_strategy   = new CMPITransformerStrategy(argc, argv, args, factory);
}
// Destructor is currently null:

CMPITransformerApp::~CMPITransformerApp()
{
    delete m_strategy;
}

/**
 * operator()
 *    Here we need to figure out, based on o ur rank, which processing
 *    element we are and run it until it exits at which point we finalize
 *    MPI and return.
 */
int
CMPITransformerApp::operator()()
{
    return (*m_strategy)();
}
