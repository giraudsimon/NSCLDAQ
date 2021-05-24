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
# @file      EVBStatisticsManager.tcl
# @brief     Package to maintain statistics of the event builder.
# @author Ron Fox <fox@nscl.msu.edu>
#

##
# In NSCLDAQ 12.0 and later, the event builder runs without a gui in the background
# controlled by the manager program.  Users will still want to see event builder
# statistics however and GUIS will be written to support this via a REST interface
# plugin for for the orderer.  This package periodically gathers and updates
# statistics information which in the GUI version was rather intertwined
# with views that included controller and model code.
# This package should be required by the background event builder startup
# script and then, after EVB::Start is invoked EVBStatistics::Start
# should be invoked to setup the statistics maintenance.
# Note that statistics maintenance relies on the event loop (e.g. after).
#
#

package provide EVBStatistics 1.0
package require ring


##
#  This namespace provides the statistics variables.  Clients, however
#  are encouraged to fetch statistics via the getter procs in this
#  package to support changes in the structure and contents of data
#  in a manner transparent to clients.  If you are reading this code,
#  internal procs (unsafe to be called by clients) have names beginning with
#  "_"  e.g. EVBStatistics::_gatherYeChildren  while those without the
#  leading underscore are considered publicly callable e.g.
#  EVBStatistics::gatherYeChildren.
#
namespace eval EVBStatistics {
    variable inputStatistics [list];       # EVB::InputStats results are stored here.
    variable outputStatistics [list];      # EVB::OutputStats results are stored here.
    variable barrierStatistics [list];     # EVB::barrierstats are stored here.
    variable dataLateStatistics [list];    # EVB::dLateStates are stored here.
    variable ooStatistics [list];          # EVB::getoostates are stored here.
    variable ringStatistics [list];        # Output ring statistics.
    variable connections    [list];        # Connection list.
    variable flowControlled 0;             # True if flow off asserted.   
}

#-------------------------------------------------------------------------------
#  Private procs.

###
#   Flow control is updated via callbacks:
#


##
# EVBStatistics::_xon
#    Callback for when flow control is turned off.
#
proc EVBStatistics::_xon { } {
    set ::EVBStatistics::flowControlled 0
}
##
# EVBStatistics::_xoff
#   Callback when flow control is turned on.
#
proc EVBStatistics::_xoff { } {
    set ::EVBStatistics::flowControlled 1
}
    


##
# EVBStatistics::_refresh
#    This is the proc in the package.  It is called by EVBStatistics::Start
#    which, in turn gets the ball rolling maintaining the statistics data.
#    _refresh uses after to self reschedule so called once, it never needs to
#    be called again.
#
# @param rate - the refresh rate in milliseconds.  Numbers like 1 or 2 are
#               pretty good values.
# @note in this implementation we just set namespaced variables with the raw
#       output of statistics getter methods.
# @note we assume that the global variable ::OutputRing contains the
#       name of the output ringbuffer; however if this has not been defined
#       then the rigStatistics are never updated.
#
proc EVBStatistics::_refresh {rate} {
    set EVBStatistics::inputStatistics [EVB::inputStats]
    set EVBStatistics::outputStatistics [EVB::outputStats get]
    set EVBStatistics::barrierStatistics [EVB::barrierstats]
    set EVBStatistics::dataLateStatistics [EVB::dlatestats]
    set EVBStatistics::ooStatistics [EVB::getoostats]
    set EVBStatistics::connections [EVB::getConnections]
    
    if {[info globals OutputRing] ne ""} {
        set EVBStatistics::ringStatistics [ringbuffer usage $::OutputRing]
    }
    
    after $rate EVBStatistics::_refresh $rate;    # Reschedule.
}

###
#   Utility procs.

##
# EVBStatistics::_dlindex
#  Does lindex but if the index does not exist, provides a default value instead.
#
# @param list  - the list to interrogate.
# @param index - The index requested.
# @param default - the value returned if there is no element.  Since we're
#                  dealing with counters, for the most part, this is optional and
#                  defaults to 0.
#     
#
proc EVBStatistics::_dlindex {list index {default 0}} {
    set result $default
    if {[llength $list] > $index} {
        set result [lindex $list $index]
    }
    return $result
}
##
# EVBStatistics::_topBarrierCounters
#    The top level barrier statistics of both the complete and incomplete
#    barrier information have both the same format and should produce
#    the same ouptut dict.  This common code does that:
#
# @param stats - Statisitics to reformat.
# @return dict -  Dict version of the statistics:
#                 - barriers number of barriers processed.
#                 - homogeneous - number of homogeneous barrierss.
#                 - heterogeneous - number of heterogeneous barriers.
#
proc EVBStatistics::_topBarrierCounters {stats} {
    return [dict create                                       \
        barriers [EVBStatistics::_dlindex $stats 0]           \
        homogeneous [EVBStatistics::_dlindex $stats 1]        \
        heterogeneous [EVBStatistics::_dlindex $stats 2]      \
    ]
}
##
# EVBStatistics::_ooStatsDict
#    Make an out of order statistics dict from a list consisting of
#    total counts, prior timestamp, offending timestmp.
# 
# @param raw   - Raw statistics.
# @return dict - containing:
#                -   count - number of out of order events.
#                -   prior - Most recent good timestamp.
#                -   offending - most recent offending timestamp.
#
proc EVBStatistics::_ooStatsDict {raw} {
    return [dict create                                         \
        count [EVBStatistics::_dlindex $raw 0]                  \
        prior [EVBStatistics::_dlindex $raw 1]                  \
        offending [EVBStatistics::_dlindex $raw 2]              \
    ]
}
#------------------------------------------------------------------------------
# Public procs:
#



###
#  Procs that get statistics in some nice form.
#

##
# EVBStatistics::getInputStatistics
#   Returns a dict that represents the input statistics.
#
# @return dict  - input statistics.. This has the following keys:
#                 * oldest   - Oldest queued timestamp.
#                 * newest   - Newest queued timestamp.
#                 * fragments - Total queued fragment count.
#
# @note if by some quirk of fate, the statistics have not been gathered,
#       zeros will be filled in.
proc EVBStatistics::getInputStatistics { } {
    set raw $::EVBStatistics::inputStatistics
    
    return [dict create                                   \
            oldest [EVBStatistics::_dlindex $raw 0]       \
            newest [EVBStatistics::_dlindex $raw 1]       \
            fragments [EVBStatistics::_dlindex $raw 2]    \
    ]
}
##
# EVBStatistics::getQueueStatistics
#    Returns statistics for each queue.
#
# @return list  - each element of the list is a dict describing queue statistics:
#                - id     Queue id.
#                - depth  Number of queued fragments.
#                - oldest  Timestamp at the front of the queue (oldest timestamp).
#                - bytes  Bytes queued.
#                - dequeued Number of bytes dequeued from the queue.
#                - totalqueued - Total no. of bytes ever queued.
#
proc EVBStatistics::getQueueStatistics {} {
    set info [EVBStatistics::_dlindex $::EVBStatistics::inputStatistics 3 [list]]
    set result [list]
    foreach q $info {
        # We can pretty well safely know that if there are queues in the
        # raw statistics, they have all list elements
        
        lappend result [dict create                                         \
            id [lindex $q 0] depth [lindex $q 1] oldest [lindex $q 2]       \
            bytes [lindex $q 3] dequeued [lindex $q 4]                      \
            totalqueued [lindex $q 5]                                       \
        ]
    }
    
    
    return $result
}
##
# EVBStatistics::getOutputStatistics
#
# @return dict containing the following keys:
#         -  fragments- Total number of output fragments.
#         -  perQueue - list of per queue information.  Each element is a dict
#                       containing:
#                       - id Source id.
#                       - fragments - number of fragments.
#
proc EVBStatistics::getOutputStatistics { } {
    set raw $::EVBStatistics::outputStatistics
    set total [EVBStatistics::_dlindex $raw 0]
    set perQ  [EVBStatistics::_dlindex $raw 1 [list]]
    
    set result [dict create fragments $total perQueue [list]]
    foreach q $perQ {
        dict lappend result perQueue [dict create \
            id [lindex $q 0]  fragments [lindex $q 1]  \
        ]
    }
    
    return $result
}

##
# EVBStatistics::getBarrierStatistics
#   Returns the top level barrier statistics
#
# @return  dict - keys are:
#       - complete  - A subdict containing:
#         *  barriers -  number of complete barriers.
#         *  homogeneous - Number of homogeneous barriers.
#         *  heterogeneous - Number of heterogeneous barriers.
#       - incomplete - A subict containing:
#         *  barriers - number of incomplete barriers.
#         *  homogeneous - Number of incomplete homogeneous barriers.
#         *  heterogeneous - number of incomplete heterogeneous barriers.
#
proc EVBStatistics::getBarrierStatistics { } {
    set complete \
        [EVBStatistics::_dlindex $EVBStatistics::barrierStatistics 0 [list]]
    set incomplete \
        [EVBStatistics::_dlindex $EVBStatistics::barrierStatistics 1 [list]]
    
    return [dict create                                                 \
        complete [EVBStatistics::_topBarrierCounters $complete]         \
        incomplete [EVBStatistics::_topBarrierCounters $incomplete]     \
    ]
}
##
# EVBStatistics::getCompleteBarrierDetails
#  @return dict   contains the following keys:
#                - byType -  list of dicts containing:
#                     *  type - barrier type id.
#                     *  count - Number of complete barriers of that type.
#                - bySource List of dicts containing:
#                     *  id   - source id.
#                     *  count - number of barrier fragments from that source
#                     *  details - list of dicts containing:
#                            > type - barrier type id.
#                            > count  - number of barriers of that type from id.
#
proc EVBStatistics::getCompleteBarrierDetails { } {
    set complete \
        [EVBStatistics::_dlindex $EVBStatistics::barrierStatistics 0 [list]]
    set byTypeInfo [EVBStatistics::_dlindex $complete 3 [list]]
    set bySrcInfo  [EVBStatistics::_dlindex $complete 4 [list]]
    
    
    set result [dict create byType [list] bySource [list]]
    
    #   Fill in the byType list:
    
    foreach s $byTypeInfo {
        dict lappend result byType [dict create \
            type [lindex $s 0]                  \
            count [lindex $s 1]                 \
        ]
    }
    #  Now the bySource:
    
    foreach s $bySrcInfo {
        set id [lindex $s 0]
        set count [lindex $s 1]
        set frags [lindex $s 2]
        set fraglist [list]
        foreach f $frags {
            lappend fraglist [dict create type [lindex $f 0] count [lindex $f 1]]
        }
        dict lappend result bySource [dict create \
           id $id count $count details $fraglist 
        ]
    }
    
    return $result
}
##
# EVBStatistics::getIncompleteBarrierDetails
#
#   @return dict  - containing:
#         - histogram - list of dict containing:
#                * number   - Number of missing sources
#                * count    - Number of times that number of sources was missing.
#         - bySource - list of dicts containing:
#               * id   - Source id.
#               * count - number of times that source was missing from the barrier.
#
proc EVBStatistics::getIncompleteBarrierDetails { } {
    set result [dict create histogram [list] bySource [list]]
    set incomplete \
        [EVBStatistics::_dlindex $EVBStatistics::barrierStatistics 1 [list]]
    set histInfo [lindex $incomplete 3]
    set srcInfo  [lindex $incomplete 4]
    
    foreach h $histInfo {
        dict lappend result histogram \
            [dict create number [lindex $h 0]  count [lindex $h 1]]
    }
    foreach s $srcInfo {
        dict lappend result bySource \
            [dict create id [lindex $s 0] count [lindex $s 1]]
    }
    return $result
}

##
# EVBStatistics::getDataLateStatistics
#     @return dict containing:
#            -   count   - Count of data late fragments.
#            -   worstDt - Worst case time difference.
#            -   details  - list of dicts, each dict contains:
#                       * id   - Source id information describefs.
#                       * count - Numberof data lates from this source.
#                       * worstDt - worst case time difference from this source.
#
proc EVBStatistics::getDataLateStatistics {} {
    set stats $EVBStatistics::dataLateStatistics
    set result [dict create                                     \
        count [EVBStatistics::_dlindex $stats 0]                \
        worstDt [EVBStatistics::_dlindex $stats 1]              \
        details [list]
    ]
    foreach d [EVBStatistics::_dlindex $stats 2 ] {
        dict lappend result details [dict create                   \
            id  [lindex $d 0]                                      \
            count [lindex $d 1]                                    \
            worstDt [lindex $d 2]                                  \
        ]
    }
    
    return $result
}
##
# EVBStatistics::getOutOfOrderStatistics
#
#   @return dict containing:
#      - summary    - Dict containing:
#             *  count - number of out of order fragments.
#             *  prior - For most recent error prior timestamp.
#             *  offending - for most recent failure the bad timestamp ( < prior).
#      - bySource   - list of dicts. Each dict is like the summary but has
#                   an additional id key that is the id fo the source being
#                   described.
#
proc EVBStatistics::getOutOfOrderStatistics { } {
    set result [dict create summary \
        [EVBStatistics::_ooStatsDict \
            [EVBStatistics::_dlindex $EVBStatistics::ooStatistics 0]]]
    set details [lindex $EVBStatistics::ooStatistics 1]
    set bysource [list]
    foreach s $details {
        set detail [EVBStatistics::_ooStatsDict [lrange $s 1 end]]
        dict set detail id [lindex $s 0]
        lappend bysource $detail
    }
    dict set result bySource $bysource
    
    return $result
}
##
# EVBStatistics::getConnectionList
#
#    Return information about the connections established by the data sources
#    to the orderer.
#
# @return list of dicts. Each dict describes a single connection and has the
#         following keys:
#         -   host  - Client's host.
#         -   description - description of the connection.
#         -   State string
#         -   idle  - boolean true if the connection has not sent data in a while.
#
proc EVBStatistics::getConnectionList {} {
    set result [list]
    
    foreach c $EVBStatistics::connections {
        set idle [expr {[lindex $c 3] eq "yes"}]
        lappend result [dict create                         \
            host [lindex $c 0] description [lindex $c 1]    \
            state [lindex $c 2]  idle $idle                 \
        ]
    }
    
    return $result
}
##
# EVBStatistics::isFlowControlled
#   @return - bool - true if the system has flow control asserted.
#
proc EVBStatistics::isFlowControlled { } {
    return $EVBStatistics::flowControlled
}

###
#   Prerequisite call:

##
# EVBStatistics::Start
#   Start the entire update process.
#   Note that at some point, it's up to the caller setting up the event builder
#   enter an event loop to allow the refresh reschedules to work.
# @note if $::StatisticsRefreshRate is defined, it will be used as the number
#      of milliseconds between statistics updates. Thuse e.g.:
#
# <code>
#    set ::StatisticsRefreshRate 5000
#    EVBStatistics::Start
# </code>
#
#   Starts statistics maintenance with an update rate of 5 seconds.
#   Note that this variable is only read once in this implementation.
#
proc EVBStatistics::Start { } {
    # Set the Xon/Xoff callbacks:
    
    EVB::onflow add EVBStatistics::_xon EVBStatistics::_xoff
    
    # Figure out the update rate:
    
    if {[info globals StatisticsRefreshRate] ne ""} {
        set refreshRate $::StatisticsRefreshRate
    } else {
        set refreshRate 1000
    }
    
    EVBStatistics::_refresh $refreshRate
}
    
