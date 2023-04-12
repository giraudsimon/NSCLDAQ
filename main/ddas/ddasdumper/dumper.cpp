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
	     East Lansing, MI 48824-1321
*/

#include "BufdumpMain.h"
#include <Exception.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <TCLApplication.h>
CTCLApplication* gpTCLApplication(0);

int main(int argc, char** argv) 
{

  BufdumpMain app;
  try {
    exit(app(argc, argv));
  }
  catch (CException& e) {
    std::cerr << "dumper main caught an exception: " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (std::string msg) {
    std::cerr << "dumper main caught an exception: " << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (const char* msg) {
    std::cerr << "dumper main caught an exception " << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (...) {
    std::cerr << "dumper main caught an unexpected exception type \n";
    exit(EXIT_FAILURE);
  }


}
