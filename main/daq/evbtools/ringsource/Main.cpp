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

#include <EVBFramework.h>
#include "CRingSource.h"

/**
 * Below this comment you should add #include directives to at least define
 * all of the sub-classes of CEVBClientApp that you are using
 */

/**
 * Program entry point.
 *
 * @param argc - count of command line paramter words.
 * @param argv - Pointers to the actual words.
 *
 * @return int
 * @retval EXIT_SUCCESS - successful exit.
 * @retval EXIT_FAILURE - unsuccessful exit.
 */
int
main(int argc, char** argv)
{
  CRingSource src(argc, argv);


  /** Below you should create instances of your event builder client app clases
   * e.g:
   * CEVBMyClient myApp;
   */


  /** Don't touch anything below this line.
   */

  return CEVBClientFramework::main(argc, argv); // Start the framework.
}
