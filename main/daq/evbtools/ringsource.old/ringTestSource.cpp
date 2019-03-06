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
 * @file ringTestsource.cpp
 * @brief Test data source for the event builder's ring buffer data
 *        source.
 *
 *  The ring buffer test data source provides test data as well as the 
 *  opportunity to create any of several error conditions and special case
 *  conditions.
 *
 *  Command line options are:
 *   --ring - Name (not URL) of ring we're putting data in as producers must
 *            be local to their rings.
 *   --size - (optional) Size of the payload of each physics event.  Use 
 *            --help to see what the defautl value is.
 *
 */

#include "ringtestoptions.h"
#include "TestSource.h"

#include <string>
#include <Exception.h>
#include <stdexcept>
#include <iostream>

#include <stdlib.h>


/**
 * main 
 *
 *  C++ program entry point.  Our responsibility is to:
 *  - Parse the command line parameters.
 *  - set up a last chance exception handler.
 *  - Create and start the data source object.
 */
int main(int argc, char** argv)
{
  struct gengetopt_args_info  args;
  cmdline_parser(argc, argv, &args);
  std::string ringName  = args.ring_arg;
  int         eventSize = args.size_arg;

  bool    genBufferHeaders = args.bodyheaders_flag;


  cmdline_parser(argc, argv, &args);

  try {
    TestSource source(ringName, eventSize,genBufferHeaders);
    source.setTimestampIncrement(args.increment_arg);
    source.setDelay(args.delay_arg);
    source();
		       
  }
  catch (std::string msg) {

    std::cerr << "String exception caught: " << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (const char* msg) {
    std::cerr << "const char* exception caught: " << msg << std::endl;
    exit(EXIT_FAILURE);
  }
  catch (CException& e) {
    std::cerr << "NSCL DAQ Exception caught: " << e.ReasonText() << std::endl;
    exit(EXIT_FAILURE);
  }
  catch(std::exception& e) {
    std::cerr << "std::exception caught: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  catch(...) {
    std:: cerr << "ERROR Unanticipated exceptioon type caught\n";
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);



}
void* gpTCLApplication(0);