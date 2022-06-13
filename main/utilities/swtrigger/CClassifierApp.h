/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file  CClassifierApp.h
* @brief ABC for classifier applications.
*/
#ifndef CCLASSIFIERAPP_H
#define CCLASSIFIERAPP_H

#include "swtriggerflags.h"
#include "CRingItemMarkingWorker.h"

/**
 * @class CClassifierApp
 *     The family of classifier applications embody both communication and
 *     parallelization for the application. The idea is that the --parallel-stratgey
 *     flag selects a specific communication and parallelization mechanism which,
 *     in turn, can be used to instantiate the correct CClassifierApp subclass
 *     by the SoftwareTrigger main program.
 * @note In addition to providing an abstract base class for the hierarchy, the
 *       class encapsulates the parsed program options.
 */
class CClassifierApp
{
protected:
    gengetopt_args_info  m_params;
public:
    CClassifierApp(gengetopt_args_info& args) :
        m_params(args) {}
    virtual ~CClassifierApp() {}                     // for destructor chaining.
    
    virtual int operator()() = 0;                    // Run the application
public:
    typedef CRingMarkingWorker::Classifier* (*ClassifierFactory)();
    ClassifierFactory getClassifierFactory(); 
};

#endif