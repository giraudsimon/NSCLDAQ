/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014-2018.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CBufferedOutput.cpp
 * @author Ron Fox
 * @brief Provide buffered output to file descriptor.
 */

#include "CBufferedOutput.h"
#include <io.h>
#include <string.h>


namespace io {
/**
 * constructor
 *    - Create the buffer.
 *    - Save the fd.
 *    - Initialize all the book keeping data.
 *
 *  @param fd - file descriptor that must be open on the data sink prior to
 *              the first flush.
 *  @param nBytes - Buffer size.
 */
CBufferedOutput::CBufferedOutput(int fd, size_t nBytes) :
    m_nFd(fd), m_pBuffer(nullptr), m_pInsert(nullptr), m_nBytesInBuffer(0),
    m_nBufferSize(nBytes), m_nTimeout(0)
{
    m_pBuffer = new uint8_t[nBytes];
    reset();
}
/**
 * destructor
 *    If there's data in the buffer, it's written to fd.
 *    the data buffer is then released.
 *
 *   @note it's up to the client to actually close the fd.
 */
CBufferedOutput::~CBufferedOutput()
{
    if (m_nBytesInBuffer) flush();
    delete []m_pBuffer;
    m_lastFlushTime = time(nullptr);
}

/**
 * put
 *    Insert data into the buffer.
 *    - While there's data left to insert,
 *    - insert the data into the buffer.
 *    - If the buffer is full,flush it.
 *
 * @param pData - Pointer to the data.
 * @param nBytes - Number of bytes to insert.
 */
void
CBufferedOutput::put(const void* pData, size_t nBytes)
{
    const uint8_t*  p  = static_cast<const uint8_t*>(pData);
    while (nBytes) {
        size_t putCount = nBytes;
        size_t bytesRemaining = (m_nBufferSize - m_nBytesInBuffer);
        if (putCount > bytesRemaining) putCount = bytesRemaining;
        
        memcpy (m_pInsert, p, putCount);
        nBytes -= putCount;
        p      += putCount;
        m_nBytesInBuffer += putCount;
        m_pInsert += putCount;
        
        if (m_nBytesInBuffer == m_nBufferSize) flush();
    }
    // If there's a timeout and it's been exceeded, then flush regardless.
    // We do that here because then flushes forced by the buffer full
    // will have times close to now.
    
    if (m_nTimeout && ((time(nullptr) - m_lastFlushTime) > m_nTimeout)) {
        flush();
    }
}
/**
 *  setTimeout
 *     Set the buffer flush timeout.
 *  @param timeout - new timeout in seconds.  Note that timeout of 0 disables the timeout
 *                   logic.
 *  @return unsigned - prior timeout value.
 */
unsigned
CBufferedOutput::setTimeout(unsigned timeout)
{
    unsigned result = m_nTimeout;
    m_nTimeout = timeout;
    return result;
}
/**
 * flush
 *    Flush data to the output fd and reset the book keeping stuff.
 */
void
CBufferedOutput::flush()
{
    io::writeData(m_nFd, m_pBuffer, m_nBytesInBuffer);
    reset();
    m_lastFlushTime = time(nullptr);
}

/**
 * reset
 *    Mark the buffer as empty.
 */
void
CBufferedOutput::reset()
{
    m_pInsert = m_pBuffer;
    m_nBytesInBuffer = 0;
    
}


}
