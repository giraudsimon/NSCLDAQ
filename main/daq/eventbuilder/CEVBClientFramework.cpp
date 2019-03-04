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
 ** This file contains implementations of all of the event builder
 ** client application framework API methods.
 */

#include <EVBFramework.h>
#include <CEVBFrameworkApp.h>

/**
 * Main - Start the event builder client framework.  We just need to 
 * get the singleton instance of the framework application and call its
 * function call operator.  We let it deal with the command line 
 * parameters.
 *
 * @param argc - Number of command line words.
 * @param argv - array of pointers to the command line words.
 * @return int
 * @retval EXIT_SUCCES or EXIT_FAILURE as required.
 */
int
CEVBClientFramework::main(int argc, char** argv)
{
  CEVBFrameworkApp* pAppInstance = CEVBFrameworkApp::getInstance();
  pAppInstance->main(argc, argv);
}

