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



#include "CBufferConverter.h"
#include "parser.h"
#include <stdlib.h>
#include <iostream>
#include <stdint.h>


int
main (int argc, char**  argv)
{
  /* Parse the command line parameters: */
  /* and extract the buffer size in bytes */

  struct gengetopt_args_info argInfo;

  if(cmdline_parser(argc, argv, &argInfo)) {
    std::cerr << argv[0] <<  " Error parsing commandline\n";
    std::cerr << "Try : " << argv[0] << " --help\n";
    exit(EXIT_FAILURE);
  }
  uint32_t bufferSize = argInfo.buffersize_arg;


  CBufferConverter converter(bufferSize);
  int status =  converter();
  exit(status ? EXIT_FAILURE : EXIT_SUCCESS);

  
  
}
