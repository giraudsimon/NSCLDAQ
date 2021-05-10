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
# @file EVBStatisticsREST.tcl
# @brief Provide REST access to event builder statistics.
# @author Ron Fox <fox@nscl.msu.edu>
#

Url_PrefixInstall /statistics StatisticsHandler

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require restutils
package require json::write
package require EVBStatistics

#-----------------------------------------------------------------------------
#  Formatting utility procs.

##
# formatInputStatistics
#   Given a dict from EVBStatistics::getInputStatistics
#   Returns the correctly formatted JSON response
#   Adds:
#   - oldest - oldest queued fragment timestamp.
#   - newest - newest queued fragment timestamp.
#   - fragments - number of queued fragments.
#
# @parm stats - dict from EVBStatistics::getInputStatistics
#
proc formatInputStatistics {stats} {
    return [json::write object                            \
        status [json::write string OK]                    \
        message [json::write string ""]                   \
        oldest  [dict get $stats oldest]                  \
        newest  [dict get $stats newest]                  \
        fragments [dict get $stats fragments]             \
    ]
}
##
# formatQStatistics
#    Formats queue statistics. Adds an array with the key
#    queues.  Each element of the array is a JSON object
#    with the fields:
#     -  id - queue id.
#     -  depth Depth of the queue.
#     -  oldest - Timetamp of oldest fragment in the queue.
#     -  bytes  - Bytes in the queue.
#     -  dequeued - total number of bytes dequeued from the queue.
#     -  totalqueued - total number of bytes ever queued.
#
# @param data - list of dicts from EVBStatistics::getQueuStatistics.
#
proc formatQStatistics {data} {
    set resultList [list]
    foreach q $data {
        lappend resultList [json::write object                   \
            id [dict get $q id]                                 \
            depth [dict get $q depth]                           \
            oldest [dict get $q oldest]                         \
            bytes [dict get $q bytes]                           \
            dequeued [dict get $q dequeued]                     \
            totalqueued [dict get $q totalqueuded]              \
        ]
        
    }
    return [json::write object                             \
            status [json::write string "OK"]                   \
            message [json::write string ""]                    \
            queues  [json::write array {*}$resultList]          \
        ]
}
##
# formatOutputStatistics
#   Returns the normal stuff as well as:
#   - fragments - Total number of ouptput fragments.
#   - perqueue  - array of objects giving:
#        * id  - source id
#        * fragments - number of fragments output from that source id.
#
# @param data - data from EVBStatistics::getOutputStatistics
#
proc formatOutputStatistics {data} {
    set perq [dict get $data perQueue]
    set qList [list]
    foreach q $perq {
        lappend qList [json::write object                           \
            id [dict get $q id]                                     \
            fragments [dict get $q fragments]                       \
        ]
    }
    return [json::write object                                        \
        status [json::write string OK]                                \
        message [json::write string ""]                               \
        fragments [dict get $data fragments]                          \
        perqueue [json::write array {*}$qList]                        \
    ]
}
#-----------------------------------------------------------------------------
# REST handler proc.

##
#  Processes REST requests for orderer statistics.
#  The data are, on success, returned as an application/json object with the
#  following keys:
#  -  status - OK or ERROR depending on success or failure.
#  -  message - on error, contains a user readable error message.
#
#  Additional key/values are returned that depend on the specific
#  suffix, which selects the specific statistics set requested.
#  This is really a thin wrapper over the getter methods in the EVBStatistics
#  
#
#  The handler requests the appropriate statistics and then
#  asks various handlers to format the reply.
#
# @param sock - socket on which the reply is sent.
# @param suffix - stuff after the top level url.
#
proc StatisticsHandler  {sock suffix} {
    if {$suffix eq "/inputstats"} {
        set data [EVBStatistics::getInputStatistics]
        Httpd_ReturnData $sock application/json [formatInputStatistics $data]
    } elseif {$suffix eq "/queue"} {
        set data [EVBStatistics::getQueueStatistics]
        Httpd_ReturnData $sock application/json [formatQStatistics $data]
    } elseif {$suffix eq "/outputstats"} {
        set data [EVBStatistics::getOutputStatistics]
        Httpd_ReturnData $sock application/json [formatOutputStatistics $data]
    } else {
        ErrorReturn $sock "'$suffix' is not a supported/implemented operation"
    }
}