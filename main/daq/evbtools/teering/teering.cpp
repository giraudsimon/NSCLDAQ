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
 * @file teering.cpp
 * @brief Main program for the teering program.
 */


#include "tcmdline.h"
#include <fragment.h>
#include <fragio.h>
#include <CRingFragmentItem.h>
#include <CRingBuffer.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iostream>

/* Local functions: */

static EVB::pFragment getFragment();
static bool           fragmentToStdout(EVB::pFragment);
static void           fragmentToRing(CRingBuffer&, EVB::pFragment);
/**
 *  teering  is analagous to the unix tee program however it
 *  tees between stdout and rings.  Input is stdin and consists of
 *  ring fragments.  Output is:
 *   - stdout - Exactly what came in.
 *   - ring   - Fragments encapsulated in CRingFragmentItem objects submitted to the
 *              destination ring.
 *
 */
int
main(int argc, char** argv) 
{

  // Process command line parameters:

  struct gengetopt_args_info args; // Parsed arguments.
  if(cmdline_parser(argc, argv, &args)) {
    exit(EXIT_FAILURE);
  }

  // Open the output ring

  CRingBuffer ring(args.ring_arg, CRingBuffer::producer);

  // Main loop:

  while(1) {
    EVB::pFragment pF = getFragment();
    if(!pF) break;

    if(!fragmentToStdout(pF)) break;
    fragmentToRing(ring, pF);
    freeFragment(pF);
  }
  exit(EXIT_SUCCESS);
}
/**
 * fragmentToRing
 *
 * Puts a new fragment in the target ring.
 *
 * @param ring - Reference to the ring.
 * @param pFrag - Pointer to the fragment.
 */
static void
fragmentToRing(CRingBuffer& ring, EVB::pFragment pFrag)
{
  CRingFragmentItem frag(pFrag->s_header.s_timestamp,
			 pFrag->s_header.s_sourceId,
			 pFrag->s_header.s_size,
			 pFrag->s_pBody,
			 pFrag->s_header.s_barrier);
  frag.commitToRing(ring);
}

/**
 * getFragment
 *    Reads a fragment from stdin.  Note that the fragment and its
 *    payload must eventually be freed via a call to freeFragment.
 *
 * @return EVB::pFragment 
 * @retval NULL - Read failed for some reason (the error is written to stderr), or eof.
 * @retval other - Pointer to the fragment read.
 */
static EVB::pFragment
getFragment()
{
  EVB::pFragment pResult;
  try {
    return CFragIO::readFragment(STDIN_FILENO);
  }
  catch (int e) {
    if (pResult) {
      freeFragment(pResult);
    }
    if (e) {			//  Only report true errors not eofs.
      std::cerr << "Read failed : " << strerror(e) << std::endl;
    }    
    return  0;

  }

}
/**
 * fragmentToStdout
 *
 * Write a flattened fragment to stdout.
 *
 * @param pFrag - Pointer to the fragment (not flattened).
 *
 */
static bool
fragmentToStdout(EVB::pFragment pFrag)
{
  try {
    CFragIO::writeFragment(STDOUT_FILENO, pFrag);
  }
  catch(int e) {
    if(e)  {
      std::cerr << "Write failed: " << strerror(e) << std::endl;
    }
    return false;
  }
  return true;
}
