#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file s800fake.tcl
# @brief Very simple s800 simulator.  This simulato assumes everything is good.
# @author Ron Fox

##
# This simulator is used to be able to run unit tests on the S800Provider
# without actually using the S800.  In general the test software will run this
# as a subprocess
#

set state inactive

##
# get
#
#  Return some variable passed in as a parameter.
#
proc get var {

    set ::reply [set ::$var]

}
##
# begin
#   Starts a run by setting the state to active
#
proc begin {} {
    set ::state active
}
##
# end
# End a run by setting the state to inactive
#
proc end {} {
    set ::state inactive
}

proc init {} {
    set ::state idle
}

proc masterTransition {to} {
  switch $to {
    Halted   {set ::state inactive} 
    Active   {set ::state active }
    Paused   {set ::state inactive}
    NotReady {set ::state inactive}
  }
  set ::result {}
}

##
# ack
#   Send an acknowledgemnt msg
#
# @param fd - socket open on the comm link.
# @param tail - Stuff to tack on the tail end of the ack.
#
proc ack {fd msg} {
    puts $fd "OK $msg"
}


##
# inputHandler
#
# get input from the socket and execute it.
#
# @param fd - file descriptor (socket) on which data is readable.
#
proc inputHandler fd {
    if {[eof $fd]} {
        close $fd
        return
    }
    set line [gets $fd]
    if {$line eq ""} {
        return
    }
    set ::reply "";       # Can be modified by command handler.
    uplevel #0 $line
    ack $fd $::reply
}

##
# connectionHandler
#
# Called in response to a TCP/IP connection request.
# @param socket - the socket fd connected to the peer for data traffic.
# @param client - Client's IP address.
# @param port   - Client's port.
#
proc connectionHandler {socket client port} {

    fconfigure $socket -buffering line
    fileevent $socket readable [list inputHandler $socket]
}
#-----------------------------------------------------------------------------
#  Entry point just listen for connections on the servers socket that's passed
#  as a parameter.
#

set forever 0
set port [lindex $argv 0]
socket -server connectionHandler -myaddr localhost $port

fileevent stdin readable exit

vwait forever
exit 0
