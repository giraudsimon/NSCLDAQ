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
# @file mg_startManager.tcl
# @brief Program to start the manager server.
# @author Ron Fox <fox@nscl.msu.edu>
#

lappend auto_path $::env(DAQTCLLIBS)
package require portAllocator

set SERVICE DAQManager
set DBFILEVAR DAQ_EXPCONFIG


##
# mg_startManager:
#   Starts a DAQ manager registered to the current user as
#   DAQManager on the port manager on the current host.
#
# Usage:
#     mg_startManager  configuration-file
# Where:
#    configuration-file is the experiment configuration database
#    the server should use.
#

proc _usage {msg} {
    puts stderr $msg
    puts stderr "\nUsage:\n"
    puts stderr "    \$DAQBIN/mg_startManager configuration-file"
    puts stderr "Starts the experiment manager server"
    puts stderr "\nWhere"
    puts stderr "    configuration-file is the experiment configuration database"
    exit -1
}

if {[llength $argv] != 1} {
    _usage {Incorrect number of command line parameters}
}

set ::env($DBFILEVAR) [lindex $argv 0]
if {![file writable $::env($DBFILEVAR)]} {
    _usage "The database file $::env($DBFILEVAR) must be read/writable and isn't"
}

#  If this user already has a manager running  don't let another one start:

set pm [portAllocator %AUTO%]
set port [$pm findServer $SERVICE]
$pm destroy
if {$port ne ""} {
    _usage "This user is already running a DAQ manager in this host"
}

#  Ok now we can start it all off.

exec [file join $::env(DAQBIN) tclhttpdlaunch] $SERVICE  \
    [file join $::env(DAQTCLLIBS) manager_server] &
