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
#   datalatestatistics - Get data late statistics.
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
    ##
    # outputstats
    #   Get the output statistics.
    #
    # @return dict containing:
    #     - fragments - total number of output fragments.
    #     - perqueue  - array of dicts, one per queue element containing:
    #       *  id   - Source id in the queue.
    #       *  fragments - Number of fragments output by the queue.
    #    
    method outputstats {} {
        return [$self _performGet outputstats]
    }
    ##
    # barrierstats
    #    Get top level barrier statistics counters.  See also
    #    completebarrierdetails and incompletebarrierdetails
    #
    # @return dict containing:
    #      -  complete - list of descriptions of complete barriers.
    #      -  incomplete list of descriptions of incomplete barreiers.
    #    Each barrier description is a dict containing:
    #         *  barriers - number of barriers of that type
    #         *  homogeneous - Number of homogeneous barriers.
    #         *  heterogeneous - Number of heterogeneous barriers..#
    #
    #
    method barrierstats {} {
        return [$self  _performGet barrierstats]
    }
    ##
    # completebarrierdetails
    #    Provide details about the complete barriers.
    #
    # @return dict - containing:
    #         -  bytype   - barrier types statistics, contains a list of dicts
    #                       with keys type  and count (barrier type and number
    #                       of times it occured).
    #         - bysource  - An array of dicts each with:
    #                     * id   - source id.
    #                     *  count - Number of barrier frags from source.
    #                     *  details - an array of dicts containng:
    #                        .  type - barrier type.
    #                        .  count - numberof times that barrier type was
    #                                   emitted from that source.
    method completebarrierdetails {} {
        return [$self _performGet completebarrierdetails]
    }
    ##
    # incompletebarrierdetails
    #
    # @return dict containing two keyes:
    #    * histogram - a list of dicts,each with keys number and count which
    #                  represent the counts that a specific number of sources was
    #                  missing from a barrier.
    #    *  bysource - a list of dicts containing the keys id and count where each
    #                  element of the list counts the number of times a specific
    #                  source id was missing from an incomplete barrier. 
    method incompletebarrierdetails {} {
        return [$self _performGet incompletebarrierdetails]
    }
    ##
    # datalatestatistics
    #   Give data late statistics
    #
    # @return dict containing:
    #       - count -number of data late fragments.
    #       - worst -Worst case dt from last not late fragment.
    #       - details - a list of dicts giving per source detail:
    #          *   id - source id.
    #          *   count - number of data lates.
    #          *   worst - Worst case dt.
    #
    method datalatestatistics {} {
        return [$self _performGet datalatestatistics]
    }
    ##
    # oostatistics
    #   Fetch the out of order statistics:
    #
    # @return dict containing:
    #   *   summary subdict with summary information:
    #      -   count  - number of out of order statistics.
    #      -   prior  - Timestamp of fragment prior to the most recent offender
    #      -   offending - Timestamp of the most recent offender.
    #  *  bysource - list of dicts giving per source out of order information:
    #      -  id  - sourceid.
    #      -  count - number of offending fragments.
    #      -   prior  - Timestamp of fragment prior to the most recent offender
    #      -   offending - Timestamp of the most recent offender.
    #
    method oostatistics {} {
        return [$self _performGet oostatistics]
    }
    ##
    # connections
    #    Provides information about the data sources currently connected to the
    #    orderer.
    #
    # @return list of dicts each dict describing a connection has the following
    #         keys:
    #         -   host   - host the connection comes from.
    #         -   description - descriptive text associated with the connection.
    #         -   state - connection state string.
    #         -   idle - boolean indicating if connection is idle.
    #
    method connections {} {
        return [$self _performGet connections]
    }
    ##
    # flowcontrol
    #    @return int which is nonzero if flow control is asserted
    method flowcontrol {} {
        set data [$self _performGet flowcontrol]
        return [dict get $data state]
    }
}

    
