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

#include "fragio.h"
#include <fragment.h>
#include <io.h>
#include <errno.h>

/**
 * readFragment
 *
 *  Reads a fragment and returns a pointer to it.
 *  The fragment is dynamically allocated and must 
 *  be freed via a later call to freeFragment (see
 *  fragment.h).
 *
 * @param fd - File descriptor from which the fragment is
 *             read.
 * @throw int - errno on error or 0 if eof.
 */
EVB::Fragment* 
CFragIO::readFragment(int fd)
{
  EVB::pFragment pResult(0);
  
  EVB::FragmentHeader hdr;
  int nread;
  nread = io::readData(fd, &hdr, sizeof(EVB::FragmentHeader));
  if (!nread) return 0;
  pResult = allocateFragment(&hdr);
  if (!pResult) {
    throw errno;		// Allocation failed.
  }
  nread = io::readData(fd, pResult->s_pBody, hdr.s_size);
  if (!nread) return 0;

  return pResult;

    
}
/**
 * writeFragment
 *
 * Writes an event fragment to the specified file descriptor.
 *
 * @param fd - file descriptor.
 * @param pFrag - Pointer to the fragment to write.
 *
 * @throw int - errno on error or 0 if output closed on us
 *              (e.g. pipe closed).
 *
 */
void
CFragIO::writeFragment(int fd, EVB::Fragment* pFrag)
{
    io::writeData(STDOUT_FILENO, &pFrag->s_header, sizeof(EVB::FragmentHeader));
    io::writeData(STDOUT_FILENO, pFrag->s_pBody,   pFrag->s_header.s_size);
  
}
