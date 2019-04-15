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

package require cmdline

##
# @file ddasReadout.tcl
# @brief Driver for DDASReadout/Sorter.
# @author Ron Fox <fox@nscl.msu.edu>
#


if {[array names env DAQBIN] ne ""} {
    set bindir $env(DAQBIN)
} else {
    set bindir [file dirname [info script]]
}

# Program usage string:

set usage "ddasReadout option...\nDriver for the split DDASReadout/ddasSort programs\nOptions:"
set options {
    {port.arg "" "Enable DDASReadout TclServer functionality on specified port"}
    {readoutring.arg "" "Ring into which DDASReadout puts data"}
    {sourceid.arg 0     "Source ID with which to tag the data"}
    {init-script.arg "" "DDASReadout initialization script run in main interpreter"}
    {log.arg ""         "DDASReadout log file"}
    {debug.arg 0      "DDASReadout debug level - controls log file contents"}
    {readouthost.arg ""   "Host in which to run the DDASReadout program"}
    {sortring.arg    ""   "Ringbuffer into which the sorter puts its data" }
    {sorthost.arg   ""   "Host in which the sorter runs"}
    {window.arg     10   "Sorting window in seconds."}
}

set mandatory [list readouthost sortring sorthost]

proc usage {} {
    ::cmdline::usage $::options $::usage
}


#  Parse the command line arguments:


set parsed [cmdline::getoptions argv $options $usage]
set parsed [dict create {*}$parsed]

puts $parsed
foreach option $mandatory {
    if {[dict get $parsed $option] eq ""} {
        puts stderr "-$option is required"
        usage
        exit -1
    }
}

#------------------------------------------------------------------------
# Construct the command line to run DDASReadout:

set ddasOptionMap {
    {port --port} {readoutring --ring} {sourceid --sourceid}
    {init-script --init-script} {log --log} {debug --debug}
}

set readoutCmd [file join $bindir DDASReadout]
set readoutHost [dict get $parsed readouthost]
foreach optMapEntry $ddasOptionMap {
    set opt [lindex $optMapEntry 0]
    set mapping [lindex $optMapEntry 1]
    set optval [dict get $parsed $opt] 
    if {$optval ne ""} {
        append readoutCmd " $mapping=$optval"
    }
}
puts $readoutCmd
puts "to run in: $readoutHost"

#-------------------------------------------------------------------------
# Construct the command to run ddasSort:

set sortCmd [file join $bindir ddasSort]

# Figure out where we get data from:

set rdoRing $tcl_platform(user)
set suppliedRing [dict get $parsed readoutring]
if {$suppliedRing ne "" } {
    set rdoRing $suppliedRing
}
set rdoURI tcp://$readoutHost/$rdoRing
append sortCmd " --source=$rdoURI"
append sortCmd " --sink=[dict get $parsed sortring]"
append sortCmd " --window=[dict get $parsed window]"
set    sortHost [dict get $parsed sorthost]

puts $sortCmd
puts "To run in $sortHost"

# Start the two programs on SSH Pipes:
#   - we need the command input for readout.
#   - we need the output/error for readout.
#   - we need the output/error for the sorter.



# Set up file events for the various fds we care about:
#   stdin -- we'll relay command from that to readout's command input.
#   stdout/stderr for both programs gets relayed to our stderr.
#
#  Then we vwait forever to run allow the software to process events.
