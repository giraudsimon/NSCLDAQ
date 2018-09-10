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


#ifndef CBUFFEREDOUTPUT_H
#define CBUFFEREDOUTPUT_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/**
 * @file CBufferedOutput.h
 * @author Ron Fox
 * @brief Provide buffered output to file descriptor.
 */
namespace io {
/**
 * @class CBufferedOutput
 *    Provides low over head buffered output to a file descriptor.
 *    Data from the buffer is written to the file descriptor safely, using
 *    io::writeData.   Data can be forced out via a call to flush as well.
 *
 *    The client must connect a file descriptor do the data sink.
 *    See testBufferedOutput.cpp for unit tests.
 */
class CBufferedOutput
{
private:
    int      m_nFd;             // Data are written to this file descriptor
    uint8_t* m_pBuffer;         // From this bufffer.
    uint8_t* m_pInsert;         // New data are inserted at this point.
    size_t   m_nBytesInBuffer;  // Contains the number of bytes in the buffer.
    size_t   m_nBufferSize;     // Size of the data buffer.
    unsigned m_nTimeout;        // Seconds after which flush is done regardless.
    time_t   m_lastFlushTime;   // When last flushed.
    
public:
    CBufferedOutput(int fd, size_t nBytes);
    virtual ~CBufferedOutput();
    
    void put(const void* pData, size_t nBytes);
    void flush();
    unsigned setTimeout(unsigned timeout);


private:
    void reset();
};

// If you like CStreams style I/O this will put the bits of a type to
// file.... probably not a good idea to do this with any interesting class
// like object.

template <class T>
CBufferedOutput& operator<<(CBufferedOutput& b, T& data) {
    b.put(&data, sizeof(T));
    return b;
}

};
#endif