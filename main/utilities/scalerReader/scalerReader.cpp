/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Giordano Cerizza 
	     NSCL
	     Michigan State University
	     East Lansing, MI
*/
#include <Exception.h>             
#include <iostream>

#include "scalerReaderMain.h"

int 
main(int argc, char** argv) 
{
  scalerReaderMain app;
  try {
    return app(argc, argv);
  }
  catch (CException& e) {
    std::cerr << "Caught an exception : " << e.ReasonText() << std::endl;
    return EXIT_FAILURE;
  }
  catch (std::string msg) {
    std::cerr << "Caught an exception: " << msg << std::endl;
    return EXIT_FAILURE;
  }
  catch (const char* msg) {
    std::cerr << "Caught an exception: " << msg << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Caught an unexpected exception type\n";
    return EXIT_FAILURE;
  }
}
