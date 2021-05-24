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

set EVBDebug 1

if $EVBDebug {

set log [open evb.log w]
proc _log {text} {
    puts $::log $text
    flush $::log
}
} else {
    proc _log {text} {
    }
}
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
            totalqueued [dict get $q totalqueued]              \
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
##
# _formatBarrierType
#    Formats one of the dicts from EVBStatistics::getBarrierStatistics
#
# @param data - dict from that call.
#
proc _formatBarrierType {data} {
    return [json::write object                                  \
        barriers [dict get $data barriers]                      \
        homogeneous [dict get $data homogeneous]                \
        heterogeneous [dict get $data heterogeneous]            \
    ]
}
##
# formatBarrierStats
#    Format the reply for EVBStatistics::getBarrierStatistics
#    In addition to the normal stuff, barrier statistics have the following
#    attributes:
#     complete - complete barrier statistics which are an object see below.
#     incomplete - incomplete barrier statistics, which are an object,
#                  below.
#     Each of the objects above has the attributes:
#       - barriers - number of barriers of that type.
#       - homgeneious - numberof homogeneous barriers.
#       - heterogeneous - number of heterogenous barriers.
#
# @param stats - the statistics.
#
proc formatBarrierStats {stats} {
    set complete [_formatBarrierType [dict get $stats complete]]
    set incomplete [_formatBarrierType [dict get $stats incomplete]]
    
    return [json::write object                        \
        status [json::write string OK]                \
        message [json::write string ""]               \
        complete $complete incomplete $incomplete    \
    ]
                    
}
##
# formatCompleteBarrierDetails
#    Return the JSON object for complete barrier details.
# @param data - data from EVBStatistics::getCompleteBarrierDetails.
# @return JSON encoded object with status, data, bytype and bysource
#         attributes.  bytype contains information by barrier type and is
#         an array containing object with the fields:
#         type  - a barrier type id.
#         count - number of occurences of that type.
#         bysource is an array of dicts containing:
#         -   id -  - source/queue id.
#         -   count - number of barrier fragments from that source.
#         -   details an array of objects containing:
#             * type - barrier type.
#             * count - number of barrier types of that type emeitted.
#
proc formatCompleteBarrierDetails {data} {
    # We need the  by Type array:
    
    set typeList [list]
    foreach t [dict get $data byType] {
        lappend typeList [json::write object                  \
            type [dict get $t type]                           \
            count [dict get $t count]                         \
        ]
    }
    set byType [json::write array {*}$typeList]
    
    #  We need the bySource array which, in turn include details:
    
    set srcList [list]
    foreach s [dict get $data bySource] {
        set details [list]
        foreach d [dict get $s details] {
            lappend details [json::write object                   \
                type [dict get $d type] count [dict get $d count] \
            ]
        }
        lappend srcList [json::write object                      \
            id [dict get $s id]                       \
            count [dict get $s count]                \
            details [json::write array {*}$details]
        ]
    }
    set bySource [json::write array {*}$srcList]
    
    return [json::write object                         \
        status [json::write string OK]                \
        message [json::write string ""]               \
        bytype $byType                                \
        bySource $bySource                            \
    ]
}
##
# formatIncompleteBarrierDetails
#   @param data - data from EVBStatistics::getIncompleteBarrierDetails
#   @return JSON object containing the usual status, message fields as well as:
#   - histogram - an array of objects containing number and count
#          which are the count of times 'number' sources were missing.
#   - bysource - an array of objects containing id (source id) and count
#           - number of times that source was missing.
#
proc formatIncompleteBarrierDetails {data} {
    # Histogram objects:
    
    set histoList [list]
    foreach h [dict get $data histogram] {
        lappend histoList  [json::write object               \
            number [dict get $h number]                       \
            count  [dict get $h count]
        ]
    }
    # Source objectsl
    
    set srcList [list]
    foreach s [dict get $data bySource] {
        lappend srcList [json::write object                   \
            id [dict get $s id] count [dict get $s count]     \
        ]
    }
    
    return [json::write object                                 \
        status [json::write string OK]                         \
        message [json::write string ""]                        \
        histogram [json::write array {*}$histoList]            \
        bysource [json::write array {*}$srcList]               \
    ]
}
##
# formatDataLateStatistics
#    @param data - data from EVBStatistics::getDataLateStatistics
#    @return JSON object containing, in addition to status and message:
#       - count - number of data late fragments.
#       - worst - Worst case dt.
#       - details - array of JSON Objects that contain:
#             * id   - source id.
#             * count - number of occurences.
#             * worst  - Worst timestamp dt.
#
proc formatDataLateStatistics {data} {
    # Make the details list:
    
    set detailsList [list]
    foreach d [dict get $data details] {
        lappend detailsList [json::write object             \
            id  [dict get $d id]                            \
            count [dict get $d count]                       \
            worst [dict get $d worstDt]                     \
        ]
    }
    return [json::write object                              \
        status [json::write string OK]                      \
        message [json::write string ""]                     \
        count  [dict get $data count]                       \
        worst  [dict get $data worstDt]                     \
        details [json::write array {*}$detailsList]         \
     ]
}
##
# formatOOStatistics
#    Formats out of order statistics.
#
# @param data from EVBStatistics::getOutOfOrderStatistics.;
# @return JSON object with usual status and message fields as well as:
#     - summary - object containing:
#          * count - number of out of order statistics.
#          * prior - Prior timestamp of most recent offender.
#          * offending - most recent failure timestamp.
#     - bysource - array of objects containing:
#           * id    - Source id.
#           * count - Number of offending fragments.
#           * prior - Prior timestamp of most recent offender.
#           * offending - Timestamp of offending fragments.
#
proc formatOOStatistics {data} {
    # By source list
    
    set bySourceList [list]
    foreach s [dict get $data bySource] {
        lappend bySourceList [json::write object          \
            id    [dict get $s id]                        \
            count [dict get $s count]                     \
            prior [dict get $s prior ]                    \
            offending [dict get $s offending]             \
        ]
    }
    return [json::write object                                  \
        status [json::write string OK]                          \
        message [json::write string ""]                         \
        summary [json::write object                             \
            count [dict get $data summary count]                \
            prior [dict get $data summary prior]                \
            offending [dict get $data summary offending]        \
        ]                                                       \
        bysource [json::write array {*}$bySourceList]      \
    ]
}
##
# formatConnectionList
#
# @param data - connection lis of dicts from EVBStatistics::getConnectionList
# @return JSON object with the usual status and message fields and connections
#         which is an array of objects that have the following fields:
#         host - name of the host connected.
#         description - description string passed at connection negotiation time.
#         state - Connection state string.
#         idle  - boolean that's nonzero if the connection has been idle longer
#                 than the idle timeout.
proc formatConnectionList {data} {
    
    # build the list needed b the connections field:
    
    set connectionList [list]
    foreach c $data {
        lappend connectionList [json::write object                          \
            host [json::write string [dict get $c host]]                    \
            description [json::write string [dict get $c description]]      \
            state [json::write string [dict get $c state]]                  \
            idle  [dict get $c idle]                                        \
        ]
    }
    return [json::write object                                             \
        status [json::write string OK]                                     \
        message [json::write string ""]                                    \
        connections [json::write array {*}$connectionList]                \
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
    } elseif {$suffix eq "/barrierstats"} {
        set data [EVBStatistics::getBarrierStatistics]
        Httpd_ReturnData $sock application/json [formatBarrierStats $data]
    } elseif {$suffix eq "/completebarrierdetails"} {
        set data [EVBStatistics::getCompleteBarrierDetails]
        Httpd_ReturnData $sock application/json \
            [formatCompleteBarrierDetails $data]
    } elseif {$suffix eq "/incompletebarrierdetails"} {
        set data [EVBStatistics::getIncompleteBarrierDetails]
        Httpd_ReturnData $sock application/json \
            [formatIncompleteBarrierDetails $data]
    } elseif {$suffix eq "/datalatestatistics"} {
        set data [EVBStatistics::getDataLateStatistics]
        Httpd_ReturnData $sock application/json \
            [formatDataLateStatistics $data]
    } elseif {$suffix eq "/oostatistics"} {
        set data [EVBStatistics::getOutOfOrderStatistics]
        Httpd_ReturnData $sock application/json [formatOOStatistics $data]
    } elseif {$suffix eq "/connections"} {
        catch {
        set data [EVBStatistics::getConnectionList]
        Httpd_ReturnData $sock application/json [formatConnectionList $data]} msg
        _log "$msg \n $::errorInfo "
    } elseif {$suffix eq "/flowcontrol"} {
        set flow [EVBStatistics::isFlowControlled]
        Httpd_ReturnData $sock application/json [json::write object         \
            status [json::write string OK]                                  \
            message [json::write string ""]                                 \
            state $flow                                                     \
        ]
    } else {
        ErrorReturn $sock "'$suffix' is not a supported/implemented operation"
    }
}