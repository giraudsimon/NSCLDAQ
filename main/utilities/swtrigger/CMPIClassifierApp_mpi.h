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

/** @file:  CMPIClassifierApp_mpi.h
 *  @brief: Application setup class for MPI for Swtrigger --parallelization-strategy=mpi
 *  
 */
#ifndef CMPICLASSIFIERAPP_MPI_H
#define CMPICLASSIFIERAPP_MPI_H
#include "CClassifierApp.h"
#include "swtriggerflags.h"

class CProcessingElement;
class CMPIAppStrategy;

/**
 * @class CMPIClasifierApp
 *   This class provides setup code for the MPI parallelization strategy
 *   for the SoftwareTrigger application.  When MPI is detected, this
 *   code is built and enabled in the software trigger application along
 *   with MPI transport classes required to make all this work.
 */

class CMPIClassifierApp : public CClassifierApp
{
private:
    CMPIAppStrategy* m_strategy;
    
public:
    CMPIClassifierApp(int argc, char** argv, gengetopt_args_info& args);
    virtual ~CMPIClassifierApp();
    
    virtual int operator()();

};


#endif