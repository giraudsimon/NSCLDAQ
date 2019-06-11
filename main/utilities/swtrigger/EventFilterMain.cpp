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

/** @file:  EventFilterMain.cpp
 *  @brief: Main program for the event filter.
 */

#include "eventfilterflags.h"
#include <stdlib.h>

/**
 *  Entry point
 *     Just process the command parameters, instatiate the app and let the
 *     app do its work.
 *
 */

int main (int argc, char** argv)
{
    gengetopt_args_info parsedArgs;
    cmdline_parser(argc, argv, &parsedArgs);
}