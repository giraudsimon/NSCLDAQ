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
# @file   mg_startLoggers.tcl
# @brief  Program that can be put in sequences to start event loggers.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names ::env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}

package require loggerrestclient

#------------------------------------------------------------------------------
#  Utilities

##
# _usage
#   Output an error message and program usage before exiting.
#
# @param msg - the message.
# @note does not return.
#
proc _usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN/mg_startloggers host user"
    puts stderr "Starts all event loggers that should be started."
    puts stderr "Where:"
    puts stderr "    host - is the host in which the manager runs."
    puts stderr "    user - is the name of the user that ran the manager."
    puts stderr "NOTE:  This program should be run and the event loggers allowed"
    puts stderr "       to start prior to starting the run in any Reaodout program"
    
    exit -1
}

#----------------------------------------------------------------------------
# Entry point

if {[llength $argv] != 2} {
    _usage "Incorrect number of command line parameters."
}
set host [lindex $argv 0]
set user [lindex $argv 1]

LoggerRestClient c -host $host -user $user
c start
c destroy