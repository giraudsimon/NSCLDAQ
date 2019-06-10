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
* @file   SoftwareTrigger.cpp
* @brief  Main program for the software trigger framwork.
*/

#include "swtriggerflags.h"
#include "CZMQThreadedClassifierApp.h"
#include <stdlib.h>

/**
 *  The main just processes the program files, instantiates the
 *  application class and runs it.
 *
 * @param argc - number of command line parameters (includes invocation string).
 * @param argv - Command line parameters.
 */
int main(int argc, char** argv)
{
    gengetopt_args_info params;
    cmdline_parser(argc, argv, &params);
    CZMQThreadedClassifierApp app(params);
    exit(app());
}