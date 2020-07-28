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

/** @file:  CMPIFullEditorApp_mpi.h
 *  @brief: Defines the full editor application class for MPI transport.
 */
#ifndef CMPIFULLEDITORAPP_MPI_H
#define CMPIFULLEDITORAPP_MPI_H

#include "CFullEventEditorApp.h"
#include "swtriggerflags.h"

class CProcessingElement;
class CMPIAppStrategy;

/**
 * @class CMPIFullEditorApp
 *    This class provides an application class for the full event editor
 *    using MPI transport.  In MPI transport, each processin element is a
 *    process with a distinct address space.   MPI uses cluster communication
 *    services as configured by the cluster managers to communicate
 *    betweeen processes.
 *      Each process has a rank which is an integer value. The ranks are
 *    used to identify communication targets.  In our case, the rank also
 *    determines the function of the process within the application:
 *
 *    *  Rank 0 - Is the data source fanout to the workers.
 *    *  Rank 1 - Is the time-stamp resorter.
 *    *  Rank 2 - Is the event sink to the output file (typically we can't
 *                output to a ringbuffer in a cluster).
 *    *  All other ranks are workers.
 *
 * This implies that the value for -np on mpirun  must be at least 4 to allow
 * for one worker.  Note that the value for --workers is a register of intent
 * rather than actually used:
 *   *  IF -np is < 4, the application aborts.
 *   *  If -np is --nworkers + 3 all is good.
 *   *  If -np is not --nworkers + 3 but is > 4 a message is output indicating
 *      that there's an inconsistency between -np and --workers and that the
 *      number of workers implied by -np is overiding the value of --workers.
 *      and the application continues with -np - 3 workers.
 */
class CMPIFullEventEditorApp : public CFullEventEditorApp
{
private:
    CMPIAppStrategy* m_strategy;
public:
    CMPIFullEventEditorApp(int argc, char** argv, gengetopt_args_info& args);
    virtual ~CMPIFullEventEditorApp();
    
    virtual int operator()();
    
};

#endif