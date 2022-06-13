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

/** @file:  CMPITransformerApp_mpi.h
 *  @brief: Defines the MPI strategy application class for the transformer.
 */
#ifndef CMPITRANSFORMERAPP_MPI_H
#define CMPITRANSFORMERAPP_MPI_H

#include "CTransformerApp.h"

class CMPIAppStrategy;


class CMPITransformerApp : public CTransformerApp
{
private:
    CMPIAppStrategy*  m_strategy;
public:
    CMPITransformerApp(int argc, char** argv, gengetopt_args_info& args);
    virtual ~CMPITransformerApp();
    
    virtual int operator()();

};



#endif