/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "loggeroptions.h"
#include "CLogRun.h"

#include <iostream>
#include <string>

#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


/**
 * This program is an event logger for NSCL buffers that
 * arrive on stdin.  It is expected that ths will run in  a pipeline like:
 * ringselect --exitonend | compatibilitybuffer --buffersize=8192 | compatibilitylogger --buffersize=8192
 *
 * Therefore we're only going to worry about recording a single run and:
 * - If we come in in the middle we give up since there's no way to assign a good
 *   run number in ringdaq.
 * - We'll exit on end of run or stdin end of file.
 *  
 * All of this really makes the program quite simple...were it not for the issue of
 * needing to break the run down into 2Gbyte segments.
 *
 */

int main(int argc, char** argv)
{
  struct gengetopt_args_info options;
  
  if (cmdline_parser(argc, argv, &options)) { // This complains on error too.
    std::cerr << "Command option parse failed";
    exit(EXIT_FAILURE);
  }

  // Create the I've started file


  const char* startedFile = "./.started";
  int fd = open(startedFile, O_WRONLY | O_CREAT,
		S_IRWXU );
  if (fd == -1) {
    perror("Could not open the .started file");
    exit(EXIT_FAILURE);
  }
  close(fd);

  // Log the run

  CLogRun runlogger(options.buffersize_arg);
  runlogger();

  // Create the I'm done file.

  const char* exitedFile     = "./.exited";
  fd = open(exitedFile, O_WRONLY | O_CREAT,
		S_IRWXU);
  if (fd == -1) {
    perror("Could not open .exited file");
    exit(EXIT_FAILURE);
  }
  close(fd);
  
  // exit.

  exit(EXIT_SUCCESS);
}
