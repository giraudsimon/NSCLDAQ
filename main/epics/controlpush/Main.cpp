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
#include <epicslib.h>


using namespace std;




/*!
  Initializes access to EPICS
  \throw fatal exception caught by epics
        on error.
*/
void EpicsInit(int argc, char** argv)
{

  epicslib::startRepeater(argc, argv);
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
   
