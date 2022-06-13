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
# @file ReadoutStatus.tcl
# @brief REST server servicing /status subdomain.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require restutils

##
#  This file provides REST handlers for
#    /status/state - fetch the current run state.
#    /status/title - fetch the run title.
#    /status/runnumber -fetch the run number.
#    /status/statistics - fetch the statistics.
#

Url_PrefixInstall /status StatusHandler

##
# _getTitle
#    If the 'title' global variable exists, that holds the title,
#    otherwise the fixed string "Not yet set" is returned.
#
# @return string
#
proc _getTitle { } {
    if {[info globals title] eq "title"} {
        return $::title
    } else {
        return "Not yet set"
    }
}

##
# _getRunNumber
#   If the global variable "run" is defined, it is returned, otherwise,
#   0 is returned.
#
# @return int
#
proc _getRunNumber { } {
    if {[info globals run] eq "run"} {
        return $::run
    } else {
        return 0
    }
}
##
# _statObj
#   Returns the key/value list of the statistics object that corresponds to the
#   triplet statistics info.
#
# @param stats - statistics triplet {trigger acceptedTriggers bytes}
# @returns list consisting of "triggers" triggers "acceptedTriggers"...etc.
#
proc _statObj {stats} {
    return [list                                                     \
        triggers [lindex $stats 0]                                   \
        acceptedTriggers [lindex $stats 1]                           \
        bytes    [lindex $stats 2]                                   \
    ]
}
    

    


##
# StatusHandler
#   Handles requests from the various status subdomains.
#   The value returned to the client is a JSON object with
#   -   status - "OK" on success "ERROR" on failure.
#   -   message - If status != "OK", provides a human readable error message.
#  The remainder of the object depends on the request:
#    - /status/state - state "statename"
#    - /status/title - title "title string"
#    - /status/runnumber - run - current run number.
#    - /status/statistics - an object that has cumulative and perRun statistics:
#      *  cumulative - current run statistics.
#      *  perRun  - Per run statistics.
#      Each statistics chunk is, itself, an object containing:
#      *  triggers - number of triggers.
#      *  acceptedTriggers - number of accepted triggers.
#      *  bytes    - Number of bytes of event data
#
# @param socket - socket on which we present our reply and on which the request
#                 came in.
# @param suffix - URL Suffix (e.g. "/state")
#
proc StatusHandler {sock suffix} {
    if {$suffix eq "/state"} {
        Httpd_ReturnData $sock application/json [json::write object        \
            status [json::write string OK]                                \
            message [json::write string ""]                               \
            state  [json::write string [runstate]]                        \
        ]
    } elseif {$suffix eq "/title"} {
        Httpd_ReturnData $sock application/json [json::write object        \
            status [json::write string OK]                                \
            message [json::write string ""]                               \
            title   [json::write string [_getTitle]]                      \
        ]
    } elseif {$suffix eq "/runnumber"} {
        Httpd_ReturnData $sock application/json [json::write object        \
            status [json::write string OK]                                \
            message [json::write string ""]                               \
            run  [_getRunNumber]                                          \
        ]
    } elseif {$suffix eq "/statistics"} {
        set stats [statistics]
        set cum [_statObj [lindex $stats 0]];   #key/value pairs
        set per [_statObj [lindex $stats 1]]
        Httpd_ReturnData $sock application/json [json::write object        \
            status [json::write string OK]                                \
            message [json::write string ""]                               \
            cumulative [json::write object {*}$cum]                       \
            perRun [json::write object {*}$per]                           \
        ]
    } else {
        ErrorReturn $socket "'$suffix' is not a supported/implemented operation"
    }
}
    
