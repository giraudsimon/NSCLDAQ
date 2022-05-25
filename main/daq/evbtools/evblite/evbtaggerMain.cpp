/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  evbtaggerMain.cpp
 *  @brief: Main program of evbtagger utility.
 */
#include "evbtagger.h"
#include <stddef.h>
#include <unistd.h>



/**
 * This program takes an input stream of ring items on STDIN_FILENO
 * tags them with fragment headers and blasts them out on STDOUT_FILENO
 * This allows the resulting stream of items to be directly used as input to
 * glom.  This pipeline with ringtostdout on theinput end and stdinto ring on
 * the output end constitutes evblite event builder for a single source of
 * timestamp sorted data (e.g. the output of a DDASSort process).
 *   See evbtagger.ggo for the usage.
 */


int main (int argc, char** argv)
{
    gengetopt_args_info parsedArgs;
    cmdline_parser(argc, argv, &parsedArgs);
    
    bool resetTimestampOnBeginRun = parsedArgs.resetts_flag != 0;
    size_t inputBufferSize = parsedArgs.buffersize_arg * 1024;  // In k.
    int source = STDIN_FILENO;
    int sink   = STDOUT_FILENO;
}