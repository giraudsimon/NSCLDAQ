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

/** @file:  CGather.h
 *  @brief: Define a class that does gather operations.
 */
#ifndef CGATHER_H
#define CGATHER_H
#include <sys/uio.h>
#include <stdint.h>

/**s
 * @class CGather
 *    NSCLDAQ transports support a gather send.  In that send, the
 *    user provides an array of iovec items which describe the base
 *    and size of message components.  Some underlying transports have
 *    direct support for this (for example ZMQ transports treat each segment
 *    as a zmq::message_t), others do not.  This class provices
 *    a mechanism for gathering data specified by an iovec array into a
 *    single buffer.
 *
 *    Note that in order to reduce memory managemnt overhead, you should
 *    try to instantiate one of these and us it over and over again.  In this
 *    way the buffer created to hold the gathered data can be re-used and only
 *    grows as needed to accommodate larger requests.
 */
class CGather {
private:
    void*    m_pData;
    size_t   m_nBufferSize;
    size_t   m_nUsedBytes;
    
public:
    CGather();
    CGather(const iovec* parts, int nParts);
    virtual ~CGather();
    
    void gather(const iovec* parts, int nParts);
    
    operator void*();               // Get the pointer.
    size_t   size() const;                // bytes used.
private:
    size_t gatherSize(const iovec* parts, int nParts) const;
    uint8_t* gatherItem(const iovec& part, uint8_t* cursor);    
};

#endif