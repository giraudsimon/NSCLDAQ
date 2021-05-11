#!/bin/sh
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
# @file   EVBRestClient.tcl
# @brief  Provide client package to the fragment orderer REST interface.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env "DAQTCLLIBS"] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package provide EVBRESTClient 1.0
package require snit
package require clientutils
package require http



##
# @class EVBRestClient
#    Provides a client to the event orderer REST interface.
#    as with the other REST clients in NSCLDAQ we lookup the service
#    each time so that if the EVB bounces between calls we can still
#    work as long as there is _currently_ a server.
#
# OPTIONS:
#   -host  - host on which the event builder orderer is running.
#   -user  - User under which the event builder is running.
#   -service - Service the REST server is advertising under. This will default
#              to EVBRestClient::service.
# METHODS:
#   inputstats - get input statistics.
#   queuestats - Get the queue statistics.
#   outputstats- Get the output statistics.
#   barrierstats- Get barrier statistics.
#   completebarrierdetails - Get detailed statistics on completed barriers.
#   incompletebarrierdetails - get detailed statistics of incomplete barriers.
#   datalatesstatistics - Get data late statistics.
#   oostatistics  - Get out of order fragment statistics.
#   connections   - Get connection list.
#   flowcontrol   - Get flow control state.
#
snit::type EVBRestClient {
    #--------------------------------------------------------------------------
    #   Class level data.
    #
    
    #
    #   domains exported by the server.
    #
    typevariable statistics "/statistics"
    
    #  Default service:
    
    typevariable defaultService "ORDERER_REST"
    
    #--------------------------------------------------------------------------
    # Object level stuff
    
    option -host
    option -user
    option -service
    
    constructor args {
        set options(-service) $defaultService; 
        $self configurelist $args
    }
    #---------------------------------------------------------------------
    # Private utilities:
    #
    
    ##
    # _performGet
    #   Perform a GET operation with the REST server.
    #
    # @param subdomain - subdomain we're requesting information from e.g. inputstats
    #                    (not /inputstats)
    #
    # @return dict with results from the server.  See the individual public
    #              methods to get information about the contents of these dicts.
    #
    method _performGet {subdomain} {
        set port [clientutils::getServerPort \
            $options(-host) $options(-user) $options(-service)   \
        ]
        set uri http://$options(-host):$port/$statistics/$subdomain
        
        set token [http::geturl $uri]
        return [clientutils::checkResult $token]
    }
    #--------------------------------------------------------------------------
    # Public utilities:
    #
    
    ##
    # inputstats
    #   Return the input statistics.
    #
    # @return dict  containing:
    #         - oldest - timestamp of oldest queued fragment.
    #         - newest = timestamp of newest queued fragment.
    #         - fragments - number of queued fragments.
    #
    method inputstats {} {
        return [$self _performGet inputstats]
    }
    ##
    # queuestats
    #    Return the queue statistics.
    # @return dict that contains:
    #      queues - a list of dicts, one per orderer queue that contains:
    #               *   id   - queue id.
    #               *   depth- Number of fragments queued in the queue.
    #               *   oldest - Timestamp of the oldest (front) element of the
    #                            queue.
    #               *   bytes - Bytes queued in the queue.
    #               *   dequeued - Total number of bytes dequeued from the queue.
    #               *   totalqueued - Total number of bytes ever queued in queue
    #
    method queuestats {} {
        return [$self _performGet queue]
    }
    method outputstats {} {}
    method barrierstats {} {}
    method completebarrierdetails {} {}
    method incompletebarrierdetails {} {}
    method datalatesstatistics {} {}
    method oostatistics {} {}
    method connections {} {}
    method flowcontrol {} {}
}

    
