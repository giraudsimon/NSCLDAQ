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
# @file  rdo_control.tcl
# @brief Command line Readout control program.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require ReadoutRESTClient
package require report
package require struct



##
# Usage:
#    rdo_control host user subcommand ?...?
#  Where:
#     host - is the host on which Readout is running.
#     user - is the user running the Readout.
#     subcommand is what we want the Readout to do.
#     the rest are subcommand specific parameters.
#
#  subcommands (and their parameters) are:
#    - begin  - begin a run.
#    - end    - End a run.
#    - init   - Initialize hardware.
#    - shutdown - Shutdown the program.
#    - setRun n - Set a new run number.
#    - setTitle title words - set a new title.
#    - getRun   - Get the run number (to stdout).
#    - getTitle - Get title (to stdout)
#    - getState - Get state of Readout.
#    - getStatistics - formats statistics nicely to stdout.
#

#------------------------------------------------------------------------------
#  Utility procs:


proc usage msg {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "    \$DAQBIN/rdo_control host user subcommand ?...?"
    puts stderr "Where:"
    puts stderr "   host - is the host on which Readout is running"
    puts stderr "   user - Is the user running Readout."
    puts stderr "   subcommand - is what you want Readout to do. Any remaining"
    puts stderr "          parameters are specific to the subcommand.  Legal"
    puts stderr "          subcommands are:"
    puts stderr "   - begin  - begin a run."
    puts stderr "   - end    - End a run."
    puts stderr "   - init   - Initialize hardware."
    puts stderr "   - shutdown - Shutdown the program."
    puts stderr "   - incRun   - Increment run number."
    puts stderr "   - setRun n - Set a new run number."
    puts stderr "   - setTitle title words - set a new title.  All subsequent"
    puts stderr "               words on the command line are assembled into a title"
    puts stderr "               string with a space between each word"
    puts stderr "   - getRun   - Get the run number (to stdout)."
    puts stderr "   - getTitle - Get title (to stdout)"
    puts stderr "   - getStatistics - formats statistics nicely to stdout."
    exit -1
}   
##
# begin
#   Begin a run.
# @param host   - Host Readout is runningi n
# @param user   - User running Readout
# @param args   - Empty command tail.
#
proc begin {host user args} {
    if {[llength $args] != 0} {
        usage "begin subcommand must not have any addition command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    c begin
    c destroy
}
##
# end
#   End a run
#
# @param host   - Host Readout is runningi n
# @param user   - User running Readout
# @param args   - Empty command tail.
#
proc end {host user args} {
    if {[llength $args] != 0} {
        usage "end subcommand must not have any addition command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    c end
    c destroy
}
##
# init
#    Initialize readout hardware
#
#
# @param host   - Host Readout is runningi n
# @param user   - User running Readout
# @param args   - Empty command tail.
#
proc init {host user args} {
    if {[llength $args] != 0} {
        usage "init subcommand must not have any addition command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    c init
    c destroy
}
##
# shutdown
#   Shutdown a readout program
#
#
# @param host   - Host Readout is runningi n
# @param user   - User running Readout
# @param args   - Empty command tail.
#
proc shutdown {host user args} {
    if {[llength $args] != 0} {
        usage "shutdown subcommand must not have any addition command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    c shutdown
    c destroy
}
##
# incRun
#   Increment the run number.
#
# @param host  - host Readout is running in
# @param user  - User readout is running under.
# @param args  - Empty command tail.
#
proc incRun {host user args} {
    if {[llength $args] != 0} {
        usage "incrun subcommand must not have any addition command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    set run [c getRunNumber]
    incr run
    c setRunNumber $run
    c destroy
}
##
# setRun
#    Set the run number.
#
# @param host  - host where Readout is running.
# @param user  - user readout is running under.
# @param args  - one element list containing a run number.
#
proc setRun {host user args} {
    if {[llength $args] != 1} {
        usage "setRun requires a run number; no more no less."
    }
    if {![string is integer -strict [lindex $args 0]]} {
        usage "setRun requires an integer run number"
    }
    if {[lindex $args 0] < 0} {
        usage "setRun's run  number must be at least 0"
    }
    ReadoutRESTClient c -host $host -user $user
    c setRunNumber [lindex $args 0]
    c destroy
}
##
# setTitle
#   Set the title
#
# @param host - host on which Readout is running.
# @param user - User running Readout.
# @param args - Words in the title.
#
proc setTitle {host user args} {
    if {[llength $args] == 0} {
        usage "setTitle subcommand requires a title."
    }
    ReadoutRESTClient c -host $host -user $user
    c setTitle "$args"
    c destroy
}
##
# getRun
#    output run number to stdout.
#
# @param host host running readout.
# @param user user running readout.
# @param args - empty command tail.
#
proc getRun {host user args} {
    if  {[llength $args] != 0} {
        usage "getRun has no additional command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    puts [c getRunNumber]
    c destroy
}
##
# getTitle
#   Gets the title from the readout and outputs it to stdout.
#
# @param host - host Readout runs in.
# @param user - User running Readout.
# @param args - command tail (empty).
#
proc getTitle {host user args} {
    if {[llength $args] != 0} {
        usage "getTitle must not have any additional command parameters."
    }
    ReadoutRESTClient c -host $host -user $user
    puts [c getTitle]
    c destroy
}
##
# getState
#    Return the readout state.
#
# @param host - host Readout runs in.
# @param user - User running Readout.
# @param args - empty command tail
#
proc getState {host user args} {
    if {[llength $args] != 0} {
        usage "getState must not have any additional command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    puts [c getState]
    c destroy
}
##
# getStatistics
#   Produces  a statistics report to stdout.
#
# @param host - host Readout runs in.
# @param user - User running Readout.
# @param args - command tail (empty).
#
proc getStatistics {host user args} {
    if {[llength $args] != 0} {
        usage "getStatistics must not have any additional command parameters"
    }
    ReadoutRESTClient c -host $host -user $user
    set stats  [c getStatistics]
    c destroy
    
    struct::matrix s
    s add columns 4
    s add row [list "" "Triggers " "Accepted Triggers " Bytes]
    foreach stat [list perRun cumulative] {
        set statistic [dict get $stats $stat]
        s add row [list                                    \
            "$stat " "[dict get $statistic triggers] "     \
            "[dict get $statistic acceptedTriggers] "      \
            [dict get $statistic bytes]                    \
        ]
    }
    report::report r [s columns]
    puts [r printmatrix s]
    r destroy
    s destroy
    
}

#------------------------------------------------------------------------------
# entry

if {[llength $argv] < 3} {
    usage "Too few command line parameters"
}
set host [lindex $argv 0]
set user [lindex $argv 1]
set subcommand [lindex $argv 2]
set tail [lrange $argv 3 end];   # command tail.

if {$subcommand in
    [list begin end init shutdown incRun setRun setTitle getRun getTitle getState getStatistics]} {
    $subcommand $host $user {*}$tail
} else {
    usage "Invalid subcommand: '$subcommand'"
}
