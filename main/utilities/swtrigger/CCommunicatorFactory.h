/*
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file CCommunicatorFactory.h
# @brief ABC for factories that make transports for communications.
# @author Ron Fox <fox@nscl.msu.edu>
#
*/

#ifndef CCOMMUNICATORFACTORY_H

class CTransport;

/**
* @class CommunicatorFactory
*     The idea of this factory is that there are really only  a few
*     communication patterns that matter in parallel computing:
*     -  Fanout from a single source to multiple workers.
*     -  Fanin from multiple workers to a single 'farmer'.
*     -  Pipelines between single processes.
*
*  This factory provides an ABC that allows specific factories to
*  be produced for different communbications systems, e.g. ZMQ, TCP/IP
*  MPI ...
*
*  In specifying endpoints, we've dropped to the lowest common denominator
*  of integers endpoints ids.   If more interesting endpoints are supported
*  (e.g. in ZMQ URIS), each factory can have a corrrespondence between
*  ids and endpoint specifiers that is gotten at configuration time.
*/
class CCommunicatorFactory
{
public:
    virtual CTransport* createFanoutTransport(int endPointId) =0;
    virtual CTransport* createFanoutClient(int endpointId, int clientId) =0;
    virtual CTransport* createFanInSource(int endpointId) = 0;
    virtual CTransport* createFanInSink(int endpointId) = 0;
    virtual CTransport* createOneToOneSource(int endpointId) = 0;
    virtual CTransport* createOneToOneSink(int endpointid) = 0;
};


#endif