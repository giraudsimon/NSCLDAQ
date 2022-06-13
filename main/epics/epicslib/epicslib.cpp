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

/** @file:  epicslib.cpp
 *  @brief: implement epicslib.
 */
#include "epicslib.h"
#include <cadef.h>
#include <stdio.h>
#include <string.h>
#include <os.h>

/**
 * checkChanStatus
 *    Given a status if it's not success, throw an std::string.
 *
 *   @param status - Status value from Epics CA.
 *   @param chan   - Name of the relevant channel
 *   @param format - sprintf format string. It must have
 *                   two slots in order for the channel name and
 *                   detailed message string (ca_message return value).
 *   @throws std::string - the message if not normal
 */
void
epicslib::checkChanStatus(int status, const char* chan, const char* format)
{
    std::string result;
    char       resultBuf[1024];
    if (status != ECA_NORMAL) {
        sprintf(resultBuf, format, chan, ca_message(status));
        result = resultBuf;
        throw result;
    }

    
}

/**
 * startRepeater
 *    Starts the epics repeater as a subprocess to us. The
 *    repeater must be in the path for success.
 *    Note that if unable to do that we fail silently because
 *    all that means is we won't have a centralized repeater
 *    process.
 * @param argc, argv - parameters passed to the program
 * #   these are passed to the repeater as well.
 */
void
epicslib::startRepeater(int argc, char** argv)
{
  int pid = fork();
  if(pid < 0) {
    perror("Failed to fork in startRepeater()");
    exit(errno);
  }
  if (pid == 0) {		// child process
    if(daemon(0,0) < 0) {
      perror("Child could not setsid");
      exit(errno);
    }
    argv[0] = const_cast<char*>("caRepeater");
    int stat = execlp("caRepeater", 
		      "caRepeater", NULL);
    perror("Child: execlp failed!!");
    exit(errno);
  }
  Os::usleep(1000);
}

