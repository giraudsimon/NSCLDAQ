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

#  Figure out how to add the NSCLDAQ TclLibs to the path:

if {[array names env DAQROOT] ne ""} {
    lappend auto_path [file join $env(DAQROOT) TclLibs]
} else {
    #  Our next best guess is that its at ../TclLibs:
    
    set libdir [file normalize [file join [file dirname [info script]] .. TclLibs]]
    lappend auto_path $libdir
}

package require ssh

##
# @file ddasReadout.tcl
# @brief Driver for DDASReadout/Sorter.
# @author Ron Fox <fox@nscl.msu.edu>
#

#-------------------------------------------------------------------------------
#  Input handling procs.

##
# onInput
#   Reacts to input on a subprogram's output.
#   -  If there's an end file condition, that means the program exited so
#       we set ::forever which will exit our program.
#   -  IF not we read the line from the pipe and relay it to stdout.
#
# @param fd  - file descriptor open on readout's output pipe.
#
proc onInput {fd who} {
    if {[eof $fd]} {
        puts "EOF on $who"
        set ::forever 1
    } else {
        set line [gets $fd]
        puts "$who: $line"
    }
}
##
# relayToReadout
#   Handles input on the stdin fd.
#   -   EOF means we're supposed to shutdown.
#   -   Data are relayed to the pipe passedin.
#
#  @param infd  - The fd on which input is awaiting.
#  @param relayfd - The fd to which input are relayed.
#
proc relayToReadout {infd relayfd} {
    if {[eof $infd]} {
        puts "End file"
        set ::forever 1
    } else {
        set line [gets $infd]
        puts "Got '$line'"
        puts $relayfd $line
        flush $relayfd
        puts "relayed"
    }
}
#------------------------------------------------------------------------------
#  Program entry point.

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
    {cratedir.arg ""   "Directory in which to run DDASReadout"}
    {sourceid.arg 0     "Source ID with which to tag the data"}
    {init-script.arg "" "DDASReadout initialization script run in main interpreter"}
    {log.arg ""         "DDASReadout log file"}
    {debug.arg 0      "DDASReadout debug level - controls log file contents"}
    {readouthost.arg ""   "Host in which to run the DDASReadout program"}
    {sortring.arg    ""   "Ringbuffer into which the sorter puts its data" }
    {sorthost.arg   ""   "Host in which the sorter runs"}
    {window.arg     10   "Sorting window in seconds."}
}

set mandatory [list readouthost sortring sorthost cratedir]

proc usage {} {
    puts stderr [::cmdline::usage $::options $::usage]
}


#  Parse the command line arguments:


set parsed [cmdline::getoptions argv $options $usage]
set parsed [dict create {*}$parsed]

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

set readoutDir [dict get $parsed cratedir]
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

set readoutInfo [ssh::sshpid  $readoutHost "\"(cd $readoutDir\; $readoutCmd)\"" ]
set readoutfd  [lindex $readoutInfo 1]
set readoutPid  [lindex $readoutInfo 0]
set sorterInfo [ssh::sshpid $sortHost    $sortCmd]
set sorterPid  [lindex $sorterInfo 0]
set sorterfd  [lindex $sorterInfo  1]

fconfigure $sorterfd -buffering line -blocking 0
fconfigure $readoutfd -buffering line -blocking 0
fconfigure stdin      -buffering line ;   # just in case.


# Set up file events for the various fds we care about:
#   stdin -- we'll relay command from that to readout's command input.
#   stdout/stderr for both programs gets relayed to our stderr.
#

fileevent $readoutfd readable  [list onInput $readoutfd DDASReadout]
fileevent $sorterfd  readable  [list onInput $sorterfd ddasSorter]
fileevent stdin      readable  [list relayToReadout   stdin $readoutfd]

puts "std fileevent: [fileevent stdin readable]"


#  Then we vwait forever to run allow the software to process events.

vwait forever;                  # Will get set when readoutfd is closed.

puts "Waited."

# Don't really have to do this since the event loop won't run again but...

fileevent $readoutfd readable ""
fileevent $sorterfd  readable ""
fileevent stdin      readable ""

#  Kill off the sorter -- we know the readout is dead because we got here
#  when it's pipe closed:

catch {exec kill -9 $sorterPid}
catch {puts $readoutfd exit; flush $readoutfd}
catch {exec kill -9 $readoutPid};          # Just in case it was the sorter.
## These can have errors...

catch {close $eadoutfd}
catch {close $sorterfd}

exit 0


