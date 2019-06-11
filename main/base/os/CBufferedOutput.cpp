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
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdexcept>
#include <stdio.h>
#include <unistd.h>

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
    m_nBufferSize(nBytes), m_nTimeout(0), m_pOutputThread(nullptr), m_halting(false)
{
    createFreeBuffers();         // Creates the free buffer pool.
    reset();                     // gets a buffer and resets all the book keeping.
    startOutputThread();         // get the output thread going.
    m_lastFlushTime = time(nullptr);
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
    stopOutputThread();           // This call will join with the output thread.
    freeBuffers();
    usleep(1000);                 // Let the thread cleanup too.
    delete m_pOutputThread;
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
  if(m_nBytesInBuffer > 0) {
    m_pCurrent->s_nBytesInBuffer = m_nBytesInBuffer;
    queueBuffer(m_pCurrent);
    reset();                           // Gets a new buffer.
  }
  m_lastFlushTime = time(nullptr); // reset timeout even if nothing's written.
  pthread_yield();                 // try to allow the writer to run.
}
/**
 * sync
 *    Intended for testing flushes our buffers and the file systsem buffers to
 *    disk...
 */
void
CBufferedOutput::sync()
{
    flush();
    fsync(m_nFd);            // flush fs buffers to disk.
}

/**
 * reset
 *    Mark the buffer as empty.
 */
void
CBufferedOutput::reset()
{
    // Get a free output buffer (blocking if needed) and setup thre
    // internal pointers from its queue element..
    
    m_pCurrent = getFreeBuffer();
    
    m_pBuffer  = static_cast<uint8_t*>(m_pCurrent->s_pBuffer);
    m_nBufferSize = m_pCurrent->s_nBufferSize;    
    m_pInsert = m_pBuffer;
    m_nBytesInBuffer = 0;
    
}
/*------------------------------------------------------------------------------
 *    Methods that communicate buffers of data between the
 *    threads.
 */
/**
 * queueBuffer
 *    Puts a buffer in the output queue so that the output thread
 *    can get it and write it.
 *
 * @param buffer - pointer to the queue element to commit.
 */
void
CBufferedOutput::queueBuffer(QueueElement* buffer)
{
    m_queuedBuffers.queue(buffer);
}
/**
 * freeBuffer
 *    Puts a buffer descriptor into the free queue where the main class
 *    can get it and use it for buffering while the output thread may
 *    be actually doing output on another buffer.
 *
 * @param buffer - pointer to the queue element.
 */
void
CBufferedOutput::freeBuffer(QueueElement* buffer)
{
    m_freeBuffers.queue(buffer);
}
/**
 * getFreeBuffer
 *    Get a buffer from the free queue (called by CBufferQueue).  This
 *    will block if needed until a buffer is available for output.
 *
 *
 *  @return CBufferedOutput::QueueElement*  - pointer to the received element.
 */
CBufferedOutput::QueueElement*
CBufferedOutput::getFreeBuffer()
{
    return m_freeBuffers.get();
}
/**
 * getQueuedBuffer
 *    Gets the next buffer to write (called by CBufferedOutput::COutputThread).
 *    We want to be sensitive to requests for exit so we
 *    will alternate between waiting to be woken up with a timeout (wait on the
 *    output queue) and doing a  check for the halting flag.
 *    The assumption is that when the halting flag has been set,
 *    all bufferes have been queued that ever will be queued.
 *
 * @return QeueuElement* - pointer ot the dequeued queue element.
 * @retval nullptr       - if there will never by any more queue elements.
 */

CBufferedOutput::QueueElement*
CBufferedOutput::getQueuedBuffer()
{
    CBufferedOutput::QueueElement* pResult(nullptr);
    while (1) {
        if (m_queuedBuffers.getnow(pResult) || m_halting) {
            return pResult;
        }
        // Not halting and there was no immediately available buffer.
        
        m_queuedBuffers.wait(10);       // Wait at most 10ms for buffer.
    }
}
/**
 * createFreeBuffers
 *     Create a bunch of free buffers.  The buffersizes are set by
 *     m_nBufferSize.  The number of buffers is a hardwired constant.
 *     The queue elements associated with the buffers are put into the
 *     free queue.
 */
void
CBufferedOutput::createFreeBuffers()
{
    
    for (int i =0; i < 10; i++) {        // Probably really only need 2...
        QueueElement* pEl = new QueueElement;
        pEl->s_nBufferSize = m_nBufferSize;
        pEl->s_nBytesInBuffer = 0;
        pEl->s_pBuffer = malloc(m_nBufferSize);
        
        freeBuffer(pEl);               // Add to the free queue.
    }
}
/**
 *  freeBuffers
 *     Free all of the buffers.  If the logic worked properly ,by the time
 *     this is called, all buffers are in the free queue; because we
 *     called this after requesting the output thread exit and joining with it.
 */
void
CBufferedOutput::freeBuffers()
{
    QueueElement* pEl;
    while (m_freeBuffers.getnow(pEl)) {
        free(pEl->s_pBuffer);
        delete pEl;
    }
}
/**
 * startOutputThread
 *    Starts the output thread.
 */
void
CBufferedOutput::startOutputThread()
{
    m_pOutputThread = new COutputThread(*this);
    m_pOutputThread->start();
}
/**
 * stopOutputThread
 *    Sets the halt flag and joins the output thread.
 */
void
CBufferedOutput::stopOutputThread()
{
    m_halting = true;
    m_pOutputThread->join();           // Blocks until output thread exits.
}

/*--------------------------------------------------------------------------
 *  CBufferedOutput::COutputThread implementationn.
 */

 /**
  * constructor
  *    Save a reference to the CBufferedOutput object that started us so that
  *    we can use it's queue management methods.
  * @param producer - the object that's producing stuff to be output.
  */
 CBufferedOutput::COutputThread::COutputThread(CBufferedOutput& producer) :
    m_producer(producer) {}
    
/**
 * operator()
 *    The thread's entry point.
 *    - Get buffers queued for output and output them.
 *    - Return said buffers to the free queue.
 *    - Rinse and repeat until we don't get buffers.
 */
void
CBufferedOutput::COutputThread::operator()()
{
    QueueElement* p;
    try {
    while(p = m_producer.getQueuedBuffer()) {
        io::writeData(
            m_producer.getFd(), p->s_pBuffer, p->s_nBytesInBuffer
        );
        m_producer.freeBuffer(p);
    }
    }
    catch (std::exception& e) {
        std::cerr << "Output thread exception: " << e.what() << std::endl;
    }
    catch (int e) {
        perror("Output thread write failed");
    }
}


}                      // io namespace.