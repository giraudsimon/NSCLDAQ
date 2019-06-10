/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file frag2ring.cpp
 * @brief Filter to turn event fragments into EVB_FRAGMENT ring buffer elements.
 *
 *  This is a filter. stdin accepts data that is in the output format of the
 *  event orderer.  Speicifcally stdin is a sequence of EVB:: FragmentHeader
 *  structs followed by the payload associated with those structs (see
 *  fragment.h
 *
 *  stdout is a series of EventBuilderFragment structs with s_body appropriately
 *  sized as defined by the payload size of the corresponding input fragment.
 *  furthermore, s_header.s_type is EVB_FRAGMENT
 *
 *  No command options are required/defined for this code.
 */

#include "CFragReader.h"
#include "CFragWriter.h"

#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <iomanip>



/**
  * Print the usage information to the screen.
  */
void printUsage() {

  std::cout << "This is the frag2ring conversion utility for converting " << std::endl;
  std::cout << "EVB::fragment structures into NSCLDAQ ring items.               \n" << std::endl;  
  std::cout << "frag2ring [options]                                               " << std::endl;  
  std::cout << "                                                                  " << std::endl;  
  std::cout << "Options:                                                          " << std::endl;  
  std::cout << "   -s,--strip  Strip fragment headers rather than converting" << std::endl;
  std::cout << "               them into EVB_FRAGMENT items, which is the default." << std::endl;
  std::cout << "   -h,--help   Print this message" << std::endl;
}

/**
 * main
 *
 *  For the most part we are just going to create a frag reader,
 *  a frag writer and then get fragments from the reader, pass them to the
 *  writer until one of them signals it can't work anymore.
 *  By convention: int exceptions signify normal completion,
 *  std::string messages signify abnormal completion with the string put out on
 *  stderr prior to exit with failure status.
 *
 * @parm argc - number of command line args 
 * @param argv - parsed for either --help,-h or --strip,-s  
 */
int main(int argc, const char** argv)
{
  bool stripHeaders = false;

  // Parse the cmdline args
  if (argc != 1) {
    for (int index=1; index<argc; ++index) {
      std::string param(argv[index]);
      
      if ((param == "--strip") || (param == "-s")) {
         stripHeaders = true; 
      } else if ((param == "--help") || (param == "-h" )) {
        printUsage();
        exit(0);
      }
      // otherwise just ignore the params
    }
  }

  CFragReader reader(STDIN_FILENO);
  CFragWriter writer(STDOUT_FILENO, stripHeaders);

  try {
    while(1) {
      void* pData = reader();
      writer(pData);
      free(pData);
    }
  }
  catch(std::string errorMessage) {
    std::cerr << errorMessage << std::endl;
    std::cerr.flush();
    exit(EXIT_FAILURE);
  }
  catch(int exitCode) {
    exit(exitCode);
  }

}

void* gpTCLApplication(0);




