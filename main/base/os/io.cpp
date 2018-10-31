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

/**
 * @file io.cpp
 * @brief Commonly used I/O methods.
 * @author Ron Fox.
 */

#include "io.h"

#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <set>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <CMutex.h>


static std::set<int>                 okErrors;	// Acceptable errors in I/O operations.

static CMutex csGaurd;
/**
 * Return true if an I/O errno is not an allowed one.
 * 
 * @param error - the  errno to check.
 *
 * @return bool - True if the error is a bad one.
 *
 * @note okErrors is a set that will contain the 'allowed' errors.
 */
static bool 
badError(int error)
{
  // Stock the okErrors set if empty:

  if (okErrors.empty())
  {
    okErrors.insert(EAGAIN);
    okErrors.insert(EWOULDBLOCK);
    okErrors.insert(EINTR);
  }

  // Not in the set -> true.

  return (okErrors.count(error) == 0);
}


namespace io {
/**
 * Write a block of data to a file descriptor.
 * As with getBuffer, multiple writes are done..until either the entire data
 * are written or
 * *  A write ends in an eof condition.
 * *  A write ends in an error condition that is not EAGAIN, EWOUDLDBLOCK or EINTR.
 *
 * @param fd    - file descriptor to which the write goes:
 * @param pData - Pointer to the data to write.
 * @param size  - Number of words of data to write.
 * 
 * @throw int 0 - I/O showed eof on output.
 * @throw int errno - An error and why.
 */

void writeData (int fd, const void* pData , size_t size)
{
  const uint8_t* pSrc(reinterpret_cast<const uint8_t*>(pData));
  size_t   residual(size);
  ssize_t  nWritten;

  while (residual) {
    nWritten = write(fd, pSrc, residual);
    if (nWritten == 0) {
      throw 0;
    }
    if ((nWritten == -1) && badError(errno)) {
      throw errno;
    }
    // If an error now it must be a 'good' error... set the nWritten to 0 as no data was
    // transferred:

    if (nWritten < 0)
    {
      nWritten = 0;
    }
    // adjust the pointers, and residuals:


    residual -= nWritten;
    pSrc     += nWritten;
  }

}
/**
 * Get a buffer of data from  a file descritor.
 * If necessary multiple read() operation are performed to deal
 * with potential buffering between the source an us (e.g. we are typically
 * on the ass end of a pipe where the pipe buffer may be smaller than an
 * event buffer.
 * @param fd      - File descriptor to read from.
 * @param pBuffer - Pointer to a buffer big enough to hold the event buffer.
 * @param size    - Number of bytes in the buffer.
 *
 * @return size_t - Number of bytes read (might be fewer than nBytes if the EOF was hit
 *                  during the read.
 *
 * @throw int - errno on error.
 */
  size_t readData (int fd, void* pBuffer,  size_t nBytes)
{
  uint8_t* pDest(reinterpret_cast<uint8_t*>(pBuffer));
  size_t    residual(nBytes);
  ssize_t   nRead;

  // Read the buffer until :
  //  error other than EAGAIN, EWOULDBLOCK  or EINTR
  //  zero bytes read (end of file).
  //  Regardless of how all this ends, we are going to emit a message on sterr.
  //

  while (residual) {
    nRead = read(fd, pDest, residual);
    if (nRead == 0)		// EOF
    {
      return nBytes - residual;
    }
    if ((nRead < 0) && badError(errno) )
    {
      throw errno;
    }
    // If we got here and nread < 0, we need to set it to zero.
    
    if (nRead < 0)
    {
      nRead = 0;
    }

    // Adjust all the pointers and counts for what we read:

    residual -= nRead;
    pDest  += nRead;
  }
  // If we get here the read worked:

  return nBytes;		// Complete read.
}


/**
 * closeUnusedFiles
 *     This function is intended for use by forked/execed processes..
 *     It is linux specific.  It uses /proc/pid/fd to
 *     close all file descriptors that are not in the set provided.
 *     Normally a forked/exece'd program will ask that all file descriptors
 *     other than STDIN_FILENO, STDOUT_FILENO and STDERR_FILENO be
 *     closed, but this method is policy agnostic.
 *
 *  @paraqm keepOpen - Set of file descriptors to keep open.
 */
void
closeUnusedFiles(std::set<int> keepOpen)
{
  CriticalSection c(csGaurd);
  errno =0;
  // construct the directory name /proc/mypid/fd.

  std::stringstream dirnameStr;
  dirnameStr << "/proc/" << getpid() << "/fd";
  std::string dirname = dirnameStr.str();
  
  // Construct the vector of fds to close.  Note that we use
  // readdir_r in the unlikely case that we're running in a threaded program
  // that also reads directories:
  
  struct dirent *nextEntry, *status;
  DIR* pDir = opendir(dirname.c_str());
  if (!pDir) throw errno;                 // Open failed.
  
  // We need to add the fd open on the dir to the
  // keepOpen set otherwise we may yank the directory stream out from underneath
  // us.
  
  keepOpen.insert(dirfd(pDir));
  do {
    nextEntry =  readdir(pDir);           // daqdev/NSCLDAQ#972
    if (!nextEntry && (errno != 0)) throw errno;               // Failure
    if (nextEntry) {                        // have a dir.
      int fd = atoi(nextEntry->d_name);       // FD the process has open.
      if (!keepOpen.count(fd)) {         // not in the list to keep open.
        close(fd);
      }
    }
  } while (nextEntry);
  
  closedir(pDir);                       // Also closes our dir fd.
}

}
