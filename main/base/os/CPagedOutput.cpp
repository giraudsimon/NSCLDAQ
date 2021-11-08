/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CPagedOutput.cpp
 *  @brief: Implement output via paged file.
 */

#include "CPagedOutput.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>
#include <system_error>
#include <errno.h>
namespace io {
/**
 * constructor
 *    - Init the data to reasonable values.
 *    - Open the file.
 *    - Figure out the page size and use that to figure out the data size.
 *    - Map the first file region and set up all dependent bookkeeping.
 *
 *  @param filename   - name of the file to map - will be created.
 *  @param bufferSize - requested buffer size - will be rounded up, if needed
 *                      to the next page multiple.
 */
CPagedOutput::CPagedOutput(const char* filename, size_t bufferSize) :
    m_pData(nullptr), m_pCursor(nullptr), m_nDataSize(0), m_nBytesLeft(0),
    m_nFilesize(0), m_nFileOffset(0), m_nFd(-1)
{
   // Try to open the file:
    
    m_nFd = open(
        filename, O_CREAT | O_RDWR | O_TRUNC,
        S_IRUSR | S_IWUSR 
    );
    if (m_nFd < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput constructor Opening file"
        );
    }
    init(bufferSize);

}
/**
 * constructor
 *   - See above but the parameter is a file descriptor open on the output file.
 *
 *  @param fd - file descriptor to use as output file.  Note this file will
 *              be truncated if necessary.
 *  @param buffersSize - size of the buffer.
 *
 */
CPagedOutput::CPagedOutput(int fd, size_t bufferSize) :
    m_pData(nullptr), m_pCursor(nullptr), m_nDataSize(0), m_nBytesLeft(0),
    m_nFilesize(0), m_nFileOffset(0), m_nFd(fd)
{
    init(bufferSize);
}
/**
 * destructor
 *   - unmap the current segment which will write it.
 *   - truncate the file to the actual number of bytes written
 *   - close the file.
 */
CPagedOutput::~CPagedOutput() noexcept(false)
{
    if (munmap(m_pData, m_nDataSize) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput destructor unmapping from file."
        );
    }
    
    if(ftruncate(m_nFd, m_nFilesize) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput destructor truncating file to actual length."
        );
    }
    
    if (close(m_nFd) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput destructor closing output file."
        );
    }
}
/**
 * put
 *    Puts data into the file.
 *    - While the data size is larger than the remaining space in the segment,
 *      put what's left and slide to the next segment.
 *    - If after that there's anything left, put it in the current segment.
 *
 * @param pData - pointer to the buffer to put.
 * @param nBytes - Number of bytes to put.
 */
void
CPagedOutput::put(void* pData, size_t nBytes)
{
    uint8_t* p = static_cast<uint8_t*>(pData);   // easier to compute with.
    
    // Put while sliding is needed...
    
    while (nBytes >= m_nBytesLeft) {
        memcpy(m_pCursor, p, m_nBytesLeft);
        m_nFilesize += m_nBytesLeft;
        p           += m_nBytesLeft;
        nBytes      -= m_nBytesLeft;
        nextSegment();                  // extend the file and map the new extent.
    }
    // If there's still stuff left, put it in too:
    
    if (nBytes) {
        memcpy(m_pCursor, p, nBytes);
        m_pCursor    += nBytes;
        m_nBytesLeft -= nBytes;
        m_nFilesize  += nBytes;
    }
}
/**
 * flush
 *     Ensures that the file is up-to-date with what's been written.
 *     @note In general, the file will be longer than the amount of
 *          data that's been 'written' to it.  The unused part will be
 *          zero filled.  There's no way at present to get the actual used
 *          size of the file so it's not recommended to be reading from a file
 *          like this while it's being written from another process.
 *          We _could_ provide a mechanisms for _this_ process to know
 *          the used file size (it's just m_nFilesize after all) but I don't
 *          know a good use-case that actually justifies it.
 */
void
CPagedOutput::flush()
{
    if (msync(m_pData, m_nDataSize, MS_ASYNC) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput flush msync call failed."
        );
    }
}
//////////////////////////////////////////////////////////////////////////////
// Private utilities.

/**
 * nextSegment
 *    - munmaps the current segment
 *    - ftruncates to extend the file by m_nDataSize
 *    - mmaps the new segment of the file.
 *    - Naturally all the needed book-keeping is also done.
 */
void
CPagedOutput::nextSegment()
{
    // out with the old
    
    if (munmap(m_pData, m_nDataSize) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput next segment munmap failed."
        );
    }
    // Enlarge the file:
    
    m_nFileOffset  += m_nDataSize;        // New file offset.
    off_t newLength = m_nFileOffset + m_nDataSize;  // New file length.
    
    if (ftruncate(m_nFd, newLength) < 0)  {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput next segment file extension failed."
        );
    }
    // In with the new can re-use the old VM region:
    
    m_pData = static_cast<uint8_t*>(mmap(
        m_pData, m_nDataSize, PROT_READ | PROT_WRITE, MAP_SHARED,
        m_nFd, m_nFileOffset
    ));
    if (m_pData == MAP_FAILED) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput next segment mmap failed."
        );
    }
    // Update the bookkeeping stuff:
    
    m_pCursor = m_pData;
    m_nBytesLeft = m_nDataSize;
    
}
/**
 * computeBufferSize
 *   - Rounds the buffer size up to the next page if it's not an exact pagesize.
 *   - Sets m_nDataSize from the result.
 *
 * @param bufferSize - requested buffersize.
 */
void
CPagedOutput::computeBufferSize(size_t bufferSize)
{
    long pageSize = sysconf(_SC_PAGESIZE);
    
    // The code below assumes that pageSize is a power of 2 -- which
    // it most probably is.
    
    size_t actualBufferSize = (bufferSize + (pageSize - 1)) & (~(pageSize-1));
    m_nDataSize = actualBufferSize;
}

/**
 * Common initialization code used by all constructors.
 */
void
CPagedOutput::init(int bufferSize)
{
    computeBufferSize(bufferSize);    // SEts m_nDataSize.
    
 
    // Now set the initial file size and do the initial file mapping setting
    // all the book keeping stuff.
    
    if(ftruncate(m_nFd, m_nDataSize) < 0) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput constructor Setting file size"
        );
    }
    
    
    m_pData = static_cast<uint8_t*>(mmap(
        nullptr, m_nDataSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_nFd,
        m_nFileOffset
    ));
    if (m_pData == MAP_FAILED) {
        throw std::system_error(
            errno, std::generic_category(),
            "CPagedOutput constructor Mapping to file."
        );
    }
    m_pCursor = m_pData;
    m_nBytesLeft = m_nDataSize;
}

}                // namespace io.
