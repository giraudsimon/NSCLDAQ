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
#include "CMPIAppStrategy_mpi.h"


#include "swtriggerflags.h"


#include <mpi.h>
#include <stdexcept>
#include <stdlib.h>
#include <iostream>
///////////////////////////// our strategy class ////////////////
/**
 * @class CMPIClassifierStrategy
 *    Derived from CMPIAppStrategy understand how to create
 *    our application processor.
 */
class CMPIClassifierStrategy : public CMPIAppStrategy {
private:
    CClassifierApp::ClassifierFactory m_pFactory;
public:
    CMPIClassifierStrategy(
        int argc, char** argv, gengetopt_args_info& args,
        CClassifierApp::ClassifierFactory factory
    );
    virtual CProcessingElement* createApplicationWorker(
        CFanoutClientTransport& source, CSender& sink, int id
    );
};

/**
 * constructor:
 *    construct the base class and save our factory so we can
 *    create the worker.
 * @param factory  -- our classifier factory.
 */
CMPIClassifierStrategy::CMPIClassifierStrategy(
    int argc, char** argv, gengetopt_args_info& args,
    CClassifierApp::ClassifierFactory factory
) :
    CMPIAppStrategy(argc, argv, args),
    m_pFactory(factory)
{}

/**
 * createApplicationWorker
 *
 * @param source - source transport.
 * @param sink   - data sink.
 * @param id     - sink id.
 */
CProcessingElement*
CMPIClassifierStrategy::createApplicationWorker(
    CFanoutClientTransport& source, CSender& sink, int id
)
{
     CRingMarkingWorker::Classifier* pClassifier = (*m_pFactory)();
     return new CRingMarkingWorker(source, sink, id, pClassifier);
}
///////////////////// The application ////////////////////

/**
 * constructor
 *    Construct our base class and our strategy object.
 */
CMPIClassifierApp::CMPIClassifierApp(
        int argc, char** argv, gengetopt_args_info& args
) :
    CClassifierApp(args),
    m_strategy(nullptr)
{
      ClassifierFactory fact = getClassifierFactory();
      m_strategy = new CMPIClassifierStrategy(argc, argv, args, fact);
}

/**
 * destructor
 */
CMPIClassifierApp::~CMPIClassifierApp()
{
    delete m_strategy;
}
/**
 * operator()
 *   Run the strategy.
 */
int
CMPIClassifierApp::operator()()
{
    return (*m_strategy)();
}