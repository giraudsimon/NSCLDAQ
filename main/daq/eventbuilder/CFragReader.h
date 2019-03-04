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
 * @file CFragReader.h
 * @brief Fragment reader (from file number) class definition.
 */

#ifndef __CFRAGREADER_H
#define __CFRAGREADER_H

#ifndef __CRT_STDLIB_H
#include <stdlib.h>		/* for size_t */
#ifndef __CRT_STDLIB_H
#endif
#endif

/**
 * CFragReader
 *
 *  Class to read fragments from a file id.  
 *  To use, instantiate on a file id, and then invoke operator()
 *  to get the next fragment.  The fragment will be returned to you 
 *  via a pointer to malloc'd storage.
 */
class CFragReader
{
  // private storage:
private:
  int m_fd;			// file descriptor to read from via read(2).

  // canonicals we need:
public:
  CFragReader(int fd);

  // public methods:
public:
  void* operator()();

  // Private utilities:

private:
  void Read(size_t nBytes, void* pBuffer);
};

#endif
