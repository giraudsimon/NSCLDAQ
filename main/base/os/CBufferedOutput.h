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
#include <CBufferQueue.h>
#include <CSynchronizedThread.h>

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
public:
    typedef struct {
        size_t s_nBufferSize;
        size_t s_nBytesInBuffer;
        void* s_pBuffer;
    } QueueElement, *pQueueElement;
    
    typedef CBufferQueue<QueueElement*> BufferQueue;
    
private:
    int      m_nFd;             // Data are written to this file descriptor
    uint8_t* m_pBuffer;         // From this bufffer.
    uint8_t* m_pInsert;         // New data are inserted at this point.
    size_t   m_nBytesInBuffer;  // Contains the number of bytes in the buffer.
    size_t   m_nBufferSize;     // Size of the data buffer.
    unsigned m_nTimeout;        // Seconds after which flush is done regardless.
    time_t   m_lastFlushTime;   // When last flushed.
    QueueElement* m_pCurrent;   // Current queue element.
    
    BufferQueue m_freeBuffers;   // Buffers available for use.
    BufferQueue m_queuedBuffers; // Buffers that are awaiting output.
    
    Thread*   m_pOutputThread;   // The output thread.
    bool     m_halting;          // True when output thread should stop.
public:
    CBufferedOutput(int fd, size_t nBytes);
    virtual ~CBufferedOutput();
    
    virtual void put(const void* pData, size_t nBytes);
    virtual void flush();
    unsigned setTimeout(unsigned timeout);
    int getFd() const {return m_nFd;}

    // While public these are intended to be used only by us
    // and our output thread.  Use by external forces will result in
    // undefined (probably program failure) behavior.

    void queueBuffer(QueueElement* buffer);         // Queue Buffer for read.
    void freeBuffer(QueueElement* buffer);          // Return buffer to free queue.
    QueueElement* getFreeBuffer();                  // Get buffer for data.
    QueueElement* getQueuedBuffer();                // Get buffer to write.


private:
    void reset();
    
    void createFreeBuffers();
    void freeBuffers();
    void startOutputThread();
    void stopOutputThread();
    
    // This nested class does the actual output providing 'simple' double buffering.
    // A buffer queue is used to exchange data from the CBufferedOutput main
    // class to the output thread.
    
    class COutputThread : public CSynchronizedThread
    {
        CBufferedOutput& m_producer;
    public:
        COutputThread(CBufferedOutput& producer);
        void operator()();
    };
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
