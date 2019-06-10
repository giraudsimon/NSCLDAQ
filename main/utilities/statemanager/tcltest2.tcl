#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

package require statemanager

##
# @file tcltest1.tcl
# @brief Test the state monitor tcl callbacks.
# @author Ron Fox <fox@nscl.msu.edu>


set reqURI [lindex $argv 0]
set subURI [lindex $argv 1]

statemanager::statemonitor start $reqURI $subURI


proc test {from to} {
    puts "Transition $from -> $to"
    puts "Title: '[statemanager::statemonitor gettitle]'"
    puts "Run:   '[statemanager::statemonitor getrun]'"
}

proc newTitle {title} {
    puts "------------- New title: '$title'"
}
proc newRun {run} {
    puts "----------------New run: '$run'"
}

proc record {state} {
    puts "New record state arg: $state"
    puts "From object: [statemanager::statemonitor getrecording]"
}
statemanager::statemonitor register NotReady test
statemanager::statemonitor register Ready    test
statemanager::statemonitor register Booting  test
statemanager::statemonitor titlecallback newTitle
statemanager::statemonitor runnumcallback newRun
statemanager::statemonitor recordingcallback record

vwait forever
