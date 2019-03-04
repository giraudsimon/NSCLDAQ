#!/bin/sh

# start tclsh: \
exec tclsh ${0} ${@}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2009.
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
# server.tcl
#
#   This file provides remote access to the vmusb package
#   via a server component.
#
# Author: Ron Fox
# Initial Date: July 6, 2012
#


# Global variables and their initial values:

set port 20000;			# default port.
set connected 0;		# true if there's a connection
set command "";			# current command string
set validRequests [list vmusb];	# Valid top level commands for the server.

# Packages used:

lappend auto_path [file join [file dirname [info script]] .. lib]
package require vmusbserver

##
# perform
#
#  Peform a request The first word of the command must be in 
#  ::validRequests.
#
# @param command - full command string
#
# @return string 
# @retval OK: resulting data from command.
# @retval FAIL: reason for failure.
#
# Implicits:
#    ::validRequest - read - the set of valid request keywords.
#
#
proc perform command {
    set command [string trim $command]
    set keyword [lindex $command 0]
    if {$keyword in $::validRequests} {
	if {[catch {{*}$command} msg]} {
	    return "FAIL: $msg"
	} else {
	    return "OK: $msg"
	}
    } else {
	return "FAIL: $keyword not a valid request"
    }

}

##
# request
#
#  Handles input ready on socket.
#  - eof - the close socket and set connected 0.
#  - absorb the available input and append it to command.
#    when the command represents a legal Tcl command,
#    pass it to the perform proc for analysis and execution.
#
# @param sock - The socket we are connected on.
# 
# Implicits:
#   ::command   - write the command being built up.
#   ::connected - write already connected flag.
#
proc request sock {
    if {[eof $sock]} { 
	close $sock
	set ::connected 0
    } else {
	append ::command [gets $sock]
	if {[info complete $::command]} {
	    puts -nonewline $sock [perform  $::command ]
	    flush $sock
	    set ::command ""
	}
    }
}

##
# connection
#
#  Handles inbound connections. 
#  - only one connection is allowed at a time.
#  - The transfer socket is set to line buffering and
#    a fileevent is set to fire the 'request' function when
#    the socket is readable.
#
# @param sock - The socket on which communication should occur.
# @param remoteaddre - host to which we are connected.
# @param remoteport  - Remote peer port of the connection
#
# Implicits:
#    ::connected - read/write
#
proc connection {sock remoteaddr remoteport} {
    if {$::connected} {
	puts $sock "FAIL: Client already connected"
	close $sock;		# Already got one.
    } else {
	set ::connected 1
	fconfigure $sock -blocking 1 -buffering full -encoding binary -translation binary
	fileevent $sock readable [list request $sock]
    }

    
}



#-----------------------------------------------------------------------
#
# Entry point:  The first parameter is the port on which to listen.
#               This defaults to 20000  This can be a server published
#               in the services database or a number.





if {[llength $argv] != 0} {
    set port [lindex $argv 0]
}

socket -server connection $port


 vwait forever
