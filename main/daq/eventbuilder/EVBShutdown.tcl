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
# @file EVBShutdown.tcl
# @brief Command utility to shutdown an event builder running with the REST plugin.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require EVBRESTClient

#-------------------------------------------------------------------------------
#  Utility methods:

##
# _usage
#   Output an error message followed with the program usage. See this
#   procedure to get the actual program usage.
#
# @param msg - Error message to precede the output.
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "    \$DAQBIN/EVBShutdown host user ?delay? ?service?"
    puts stderr "\nShuts down an event builder who's orderer stage is running the"
    puts stderr "REST plugin.  Note that only the orderer is shutdown. The"
    puts stderr "Remaining pipeline elements should shutdown due to seeing an"
    puts stderr "EOF on their stdins.\n"
    puts stderr "Where:"
    puts stderr "   host   - is the host the event builder runs in."
    puts stderr "   user   - User under which the event builder is running"
    puts stderr "   delay  - Seconds the orderer will delay before exiting"
    puts stderr "            This defaults to 2 seconds if not supplied"
    puts stderr "   service - If provided this is the server the REST interface"
    puts stderr "             runs under."
    puts stderr "NOTE:"
    puts stderr "   At present, the delay parameter, although accepted by this"
    puts stderr "   program is ignored by the server which uses a fixed delay"
    exit -1
}

#-------------------------------------------------------------------------
#  Entry point:

set nargs [llength $argv]
if {($nargs < 2) || ($nargs > 4)} {
    _usage "Incorrect number of program parameters"
}

set service ""
set delay    2
set host [lindex $argv 0]
set user [lindex $argv 1]
if {$nargs >= 3} {
    set delay [lindex $argv 2]
}
if {$nargs == 4} {
    set service [lindex $argv 3]
}

#  Delay must be a positive integer:

if {(![string is integer -strict $delay]) || ($delay < 0)} {
    _usage "The delay parameter must be a positiv integer and was '$delay'"
}

#  How we construct the client depends on whether or not the service is empty:

if {$service eq ""} {
    set client [EVBRestClient %AUTO% -host $host -user $user]
} else {
    set client [EVBRestClient %AUTO% -host $host -user $user -service $service]
}

$client shutdown $delay
exit 0
