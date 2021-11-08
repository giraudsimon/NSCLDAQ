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

/** @file:  CPagedOutput.h
 *  @brief: Outputs to a file using memory mapped paged I/O
 */

#ifndef CPAGEDOUTPUT_H
#define CPAGEDOUTPUT_H
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @class CPagedOutput
 *    Outputs data to a file using page mapped I/O.  The idea is that
 *    we hold a region mapped to a file.  We plop data into it and
 *    the munmap actually flushes that data out to file.
 *    We use ftruncate to extend the file as needed.  Our buffersize
 *    is the number of pages of memory that can acccomodate the requested
 *    buffer size.
 *    -  When the file is closed, the file is truncated to the number of bytes
 *    actually written.
 *    -  flush results in an msync which ensures the data are transferred to disk.
 *
 *
 *   @note - programs that access the file while it's being written must
 *           be written to understand that there may be pages in the file
 *           that don't have valid data yet.  Those pages will, in general,
 *           be filled with zeroes as that's what ftruncate does when you
 *           expand a file.
 */
namespace io {
class CPagedOutput
{
private:
    uint8_t* m_pData;               // Current mapped region.
    uint8_t* m_pCursor;             // Current write position.
    size_t   m_nDataSize;           // Size of mapped region.
    size_t   m_nBytesLeft;          // Bytes not yet written in region.
    off_t    m_nFilesize;           // Complete size of file.
    off_t    m_nFileOffset;         // Offset of current segment in file.
    int      m_nFd;                 // file descriptor.
public:
    CPagedOutput(const char* filename, size_t bufferSize);
    CPagedOutput(int fd, size_t bufferSize);
    
    virtual ~CPagedOutput() noexcept(false);
    
    void put(void* pData, size_t nBytes);
    void flush();
private:
    void nextSegment();
    void computeBufferSize(size_t bufferSize);
    void init(int bufferSize);
};

}                          // Namespace io.
#endif