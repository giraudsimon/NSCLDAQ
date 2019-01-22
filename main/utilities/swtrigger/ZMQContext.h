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

/** @file:  ZMQContext.h
 *  @brief: Interface for a ZMQ context singleton wrapper.
 */

#ifndef ZMQCONTEXT_H
#define ZMQCONTEXT_H
#include <zmq.hpp>

/**
 * @class ZMQContext
 *    This is a wrapper for an application singleton ZMQ context.
 *    (zmq::context_t). At application initialization, one needs to
 *    configure it with the number of threads that will be passed into the
 *    zmq::context_t constructor.  Note that a default value for the number
 *    of context I/O threads is also set at program load time.
 *
 *    once the number of I/O threads has been established; getContext
 *    can be called to return a reference to the singleton zmq context.
 *    Presumably zmq protects copy construction from happening but the
 *    normal bit of code you need is:
 *
 *    zmq::context_t& ctx(ZMQContext::getContext());
 *
 *    where the resulting context should be passed around either by
 *    reference or by pointer only.
 */
class ZMQContext {
private:
    static int             m_nIoThreads;
    static zmq::context_t* m_pInstance;
public:
    static int setIoThreadCount(int threads);
    static int getIoThreadCount();
    static zmq::context_t& getContext();
};

#endif