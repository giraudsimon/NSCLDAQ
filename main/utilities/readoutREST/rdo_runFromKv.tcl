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
# @file  rdo_runFromKv
# @brief Set a readout program's run number fro the key value store.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require kvclient
package require rdoutils
package require ReadoutRESTClient

#------------------------------------------------------------------------------
# Utility functions.
#

##
# usage
#    Prints out program usage  preceded by an error message.
#
# @param msg - the error message
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "rdo_runFromKv host user program"
    puts stderr "    Sets the run of a readout program to the value of the "
    puts stderr "    run item in the key value store."
    puts stderr "Where:"
    puts stderr "    host  - is the host in which the DAQ  is running"
    puts stderr "    user  - is the user the DAQ manager is running under"
    puts stderr "    program - is the name of the readout program whose title we're setting."
     exit -1
}
#-------------------------------------------------------------------------------
# Entry point

if {[llength $argv] != 3} {
    usage "Incorrect argument count:"
}

set host [lindex $argv 0]
set user [lindex $argv 1]
set program [lindex $argv 2]

set programHost [getProgramHost $host $user $program];   #Where we send title request to.

#  Get the value of the title key value item:

KvClient kv -host $host -user $user
set run [kv getValue run]
kv destroy

ReadoutRESTClient rest -host $programHost -user $user
if {[array names ::env SERVICE_NAME] ne "" } {
    rest configure -service $::env(SERVICE_NAME)
}
rest setRunNumber $run
rest destroy
