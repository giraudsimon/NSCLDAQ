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
 * @file CFragReader.cpp
 * @brief Fragment reader (from file number) class implementation.
 */
#include "CFragReader.h"
#include "fragment.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <io.h>
#include <iostream>

#include <string>

/**
 * constructor
 *
 * @param fd - the file descriptor we read from. 
 */
CFragReader::CFragReader(int fd) :
  m_fd(fd)
{}

/**
 * operator()
 *
 * Read a fragment from the fd into storage that requries a free(3) call to release.
 * This pointer is returned to the caller and must be freed by them.
 *
 * @return void*
 * @retval Pointer to dynanmically allocated storage that contains the fragment.
 */
void*
CFragReader::operator()()
{
  // first read the header:

  EVB::FragmentHeader header;
  
  Read(sizeof(header), &header);

  // compute the size of the buffer, 
  // copy the header in, and compute a pointer to the payload:
  //

  size_t fragmentSize = sizeof(header) + header.s_size;
  uint8_t* pResult = reinterpret_cast<uint8_t*>(malloc(fragmentSize));
  if (pResult==0) {
    throw std::string("CFragReader:::operator()() Failed to allocate requested memory");
  }
  memcpy(pResult, &header, sizeof(header));
  uint8_t* pBody = pResult + sizeof(header);

  // Read the body and return the result:

  Read(header.s_size, pBody);

  return pResult;
}
/**
 * Read
 *
 * Read a block of data from the input file:
 *
 * @param nBytes - number of bytes to read.
 * @param pBufer - Pointer to the data buffer.
 *
 * @throw int if end of file at the beginning of the read.
 * @throw std::string if a read error of some sort.
 */
void
CFragReader::Read(size_t nBytes, void* pBuffer)
{
  int bytes=0;
  try {
    bytes = io::readData(m_fd, pBuffer, nBytes);
    if (bytes==0) {
      throw 0;
    }
  }
  catch (int e) {
    if (e == 0) {
      throw EXIT_SUCCESS;
    } else {
      throw std::string(strerror(e));
    }
  }

}
