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
#include <config.h>
#include "RingSelectorMain.h"
#include <Exception.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <signal.h>
#include <os.h>
#include <stdio.h>

int main(int argc, char** argv) 
{
  if (Os::blockSignal(SIGPIPE)) {
    perror("failed to block SIGPIPE");
  }
  try {
    RingSelectorMain app;
    return app(argc, argv);
  }
  catch (CException& e) {
    std::cerr << "Exception: " << e.ReasonText() << std::endl;
    return(EXIT_FAILURE);
  }
  catch (std::string msg) {
    std::cerr << "Exception: " << msg << std::endl;
    return(EXIT_FAILURE);
  }
  catch (const char* msg) {
    std::cerr << "Exception: " << msg << std::endl;
    return(EXIT_FAILURE);
  }

  
}

