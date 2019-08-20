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

/** @file:  EventEditor.cpp
 *  @brief: Main program for the event editor.
 */


#include "eventeditor.h"
#include "CBuiltRingItemEditorApp.h"

#include <stdlib.h>
/**
 * main
 *    Entry point.
 *    - Parse the parameters.
 *    - Choose the application based on the --parallel-strategy flag.
 *    - Instantiate and run the appropriate appliction.
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info parsed;
    cmdline_parser(argc, argv, &parsed);
    CBuiltRingItemEditorApp* pApp;
    
    // This code selects the app to use:
    
    
    // Run the app:
    
    (*pApp)();
    
    exit(EXIT_SUCCESS);
}