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
* @file   CZMQThreadedClassifierApp.cpp
* @brief Implements the CZMQThreadedClassifierAPp class.
*/
#include "CZMQThreadedClassifierApp.h"
#include <stdlib.h>

/**
 * constructor
 *   @param args -the parsed arguments.
 */
CZMQThreadedClassifierApp::CZMQThreadedClassifierApp(gengetopt_args_info& args) :
    CClassifierApp(args) {}
    
/**
 * destructor
 */
CZMQThreadedClassifierApp::~CZMQThreadedClassifierApp()
{}

/**
 * operator()
 *    Runs the application.
 * @return - the status to return on application exit.
 */
int
CZMQThreadedClassifierApp::operator()()
{
    return EXIT_SUCCESS;
}