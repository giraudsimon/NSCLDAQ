/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*!
  \file Main.cpp
   This file contains the unbound functions and the program entry point.
   The inventory of functions includes:
   - main          - Program entry point.
   - EpicsInit     - Perform whatever initialization EPICS requires.
   - startRepeater - Workaround to properly start/stop the caRepeater
                     process when it is not started by the system startup
                     scripts.
*/

#include <config.h>
#include "cmdline.h"
#include "CApplication.h"
#include <cadef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <CopyrightNotice.h>
#include <iostream>
#include <string>
#include <Exception.h>
#include <stdlib.h>
#include <os.h>


using namespace std;



/*!
   This function kludges around a problem with epics.  It's supposed
   to start up caRepeater, but evidently:
   - It does that by forking and making caRepeater the parent process.
   - the resulting caRepeater identifies to ps as us.
   - After our process exits, the resulting caRepeater doesn't work quite
     right but hangs around.


*/
void
startRepeater(int argc, char** argv)
{
  string repeatername(EPICS_BIN);
  repeatername += "/caRepeater";

  int pid = fork();
  if(pid < 0) {			// Fork failed.
    perror("Failed to fork in startRepeater()");
    exit(errno);
  }
  else if (pid == 0) {		// child process
    if(daemon(0,0) < 0) {
      perror("Child could not setsid");
      exit(errno);
    }
    argv[0]  = const_cast<char*>("caRepeater");
    int stat = execlp(repeatername.c_str(),
		      repeatername.c_str(), NULL);
    perror("Child: execlp failed!!");
    exit(errno);
  }
  else {			// Fork parent process..
    Os::usleep(1000);			// Let it startup.
  }
}

/*!
  Initializes access to EPICS
  \throw fatal exception caught by epics
        on error.
*/
void EpicsInit(int argc, char** argv)
{

  startRepeater(argc, argv);
  int status = ca_task_initialize();
  SEVCHK(status, "Initialization failed");
  
}


/*!
  Actual program entry point.  We parse the commands and, if that's
  successful, creat an application object and invoke it.
*/
int
main(int argc, char** argv)
{
  try {
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args); // exits on error.

    // Ensure the user supplied a filename.

    if(args.inputs_num != 1) {
      cerr << "Missing channel input file.\n";
      cmdline_parser_print_help();
      exit(-1);
    }

    CopyrightNotice::Notice(cout, argv[0], 
			    "1.0", "2004");
    CopyrightNotice::AuthorCredit(cout, argv[0],
				  "Ron Fox", NULL);
    EpicsInit(argc, argv);
    CApplication app;
    int status =  app(args);
    
    // In the future, maybe app can exit correctly... so...

    if(status != 0) {
      cerr << "Main application object exiting due to error\n";
    }
    return status;
  }
  catch (string message) {
    cerr << "Main caught a string exception: " << message << endl;
    cerr << "Exiting due to string exception " << endl;
  }
  catch (CException& failure) {
    cerr << "Main caught an NSCL Exception object: " << failure.ReasonText()
         << endl;
    cerr << "Exiting due to an NSCL Exception object\n";
  }
  catch (char* msg) {
    cerr << "Main caught a char* exception: " << msg << endl;
    cerr << "Exiting due to a char* exception " << endl;
  }
  catch (...) {
    cerr << "Main caught an unanticipated exception type ... exiting \n";
  }
  exit(-1);			// Exit due to exception.
}
   
