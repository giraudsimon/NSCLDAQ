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

/** @file:  CMPIAppStrategy_mpi.h
 *  @brief: Provide base class for MPI source->workers->sort->output apps.
 */

#ifndef CMPIAPPSTRATEGY_H
#define CMPIAPPSTRATEGY_H
#include "swtriggerflags.h"

class CFanoutClientTransport;
class CSender;
class CProcessingElement;

/**
 * @class CMPIStrategy
 *     MPI versions of our standard application framework all
 *     look sort of the same:
 *     - Rank 0 creates a data source of some sort depending onthe
 *       parameters.
 *     - Rank 1 creates a sorting object.
 *     - Rank 2 creats a sink object.
 *     - All the other ranks create some worker object.
 *
 *     This class encapsulates that in a strategy pattern really only
 *     a single pure virtual method... a method to produce the
 *     actual worker object given the transports needed to
 *     hook it to the rest of the application.
 */
class CMPIAppStrategy
{
private:
    gengetopt_args_info& m_args;
public:
    CMPIAppStrategy(int argc, char** argv, gengetopt_args_info& args);
    virtual ~CMPIAppStrategy();
    
    virtual int operator()();             // delegated run operator.
protected:
    virtual CProcessingElement* createProcessingElement();
    virtual CProcessingElement* createDataSource();        // Rank 0
    virtual CProcessingElement* createWorker();            // Rank >=3
    virtual CProcessingElement* createSorter();            // Rank 1
    virtual CProcessingElement* createSink();              // Rank 2
    
    virtual CProcessingElement* createApplicationWorker(
        CFanoutClientTransport& source, CSender& sink, int id
    ) = 0;
};

#endif