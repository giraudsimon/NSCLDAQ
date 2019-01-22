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

/** @file:  ZMQContext.cpp
 *  @brief: ZMQ singleton context. wrapper.
 */

#include <ZMQContext.h>
#include <stdexcept>

int ZMQContext::m_nIoThreads(10);
zmq::context_t* ZMQContext::m_pInstance(0);

/**
 * setIoThreadCount
 *    Modifies the number of I/O threads that will be used to construct
 *    the context.  Note that this must be called before the first call
 *    to getContext.
 *
 * @param threads  new value.
 * @return Prior value.
 * @throws std::logic_error - if the context was already created.
 */
int
ZMQContext::setIoThreadCount(int threads)
{
    if (m_pInstance) {
        throw std::logic_error(
            "Too late to set the zmq i/o thread count. Context is already constructed"
        );
    }
    int value    = m_nIoThreads;
    m_nIoThreads = threads;
    return value;
}
/**
 * getIoThreadCount
 *   Return the current value of the I/O thread count.
 *   @return int
 */
int
ZMQContext::getIoThreadCount()
{
    return m_nIoThreads;
}
/**
 * getContext
 *    Returns the singleton context, createing it if necessary.
 *
 * @return zmq::context_t&
 */
zmq::context_t&
ZMQContext::getContext()
{
    if (!m_pInstance) {
        m_pInstance = new zmq::context_t(m_nIoThreads);
    }
    return *m_pInstance;
}