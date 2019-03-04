/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI
*/
#include <config.h>
#include "sclclientMain.h"
#include <Exception.h>
#include <string>
#include <iostream>
#include <stdlib.h>

using namespace std;



int 
main(int argc, char** argv) 
{
  SclClientMain app;
  try {
    return app(argc, argv);
  }
  catch (CException& e) {
    cerr << "Caught an exception : " << e.ReasonText() << endl;
    return EXIT_FAILURE;
  }
  catch (string msg) {
    cerr << "Caught an exception: " << msg << endl;
    return EXIT_FAILURE;
  }
  catch (const char* msg) {
    cerr << "Caught an exception: " << msg << endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    cerr << "Caught an unexpected exception type\n";
    return EXIT_FAILURE;
  }
}
