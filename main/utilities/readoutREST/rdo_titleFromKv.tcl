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
# @file   rdo_titleFromKv
# @brief  Set the readout title from the KV title.
# @author Ron Fox <fox@nscl.msu.edu>
# @note the assumption is that because the server is in the event loop waiting for
#       the sequence containing this to end, the server can process another
#       request even though it's set to be singly threaded.
#
if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $::env(DAQTCLLIBS)
}
package require kvclient
package require rdoutils
package require ReadoutRESTClient
#------------------------------------------------------------------------------
#  Utility functions.
##
# usage
#   Prints out program usage  use this proc to understand the program command
#   line requirements.
#
# @param msg  - error message that will precede the usage.
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "rdo_titleFromKv host user program"
    puts stderr "    Sets the title of a readout program to the value of the "
    puts stderr "    title item in the key value store."
    puts stderr "Where:"
    puts stderr "    host  - is the host in which the DAQ  is running"
    puts stderr "    user  - is the user the DAQ manager is running under"
    puts stderr "    program - is the name of the readout program whose title we're setting."
     exit -1
}  

#-------------------------------------------------------------------------------
#  Entry point
#

if {[llength $argv] != 3} {
    usage "Incorrect argument count:"
}

set host [lindex $argv 0]
set user [lindex $argv 1]
set program [lindex $argv 2]

set programHost [getProgramHost $host $user $program];   #Where we send title request to.

#  Get the value of the title key value item:

KvClient kv -host $host -user $user
set title [kv getValue title]
kv destroy

#  Since the manager starts the Readout (we have to take their word this is a readout)
#  We can use the same user:

ReadoutRESTClient rest -host $programHost -user $user
if {[array names ::env SERVICE_NAME] ne ""} {
    rest configure -service $::env[SERVICE_NAME]
}

rest setTitle $title
rest destroy
