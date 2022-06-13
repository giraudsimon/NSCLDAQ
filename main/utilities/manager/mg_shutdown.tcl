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
# @file    mg_shutdown.tcl
# @brief   Command to shutdown the manager server.
# @author Ron Fox <fox@nscl.msu.edu>
#
if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}

package require stateclient

#----------------------------------------------------------------------------
#  Private procs:

##
# _usage
#    Output an error message and usage text to stderr before exiting.
#
# @param msg - the error message.
#
proc _usage {msg} {
   puts stderr $msg
   puts stderr "Usage:"
   puts stderr "   mg_shutdown host user"
   puts stderr "Shuts down the manager server.  If the current state is not"
   puts stderr "SHUTDOWN - the daq system is first made to do a SHUTDOWN transition"
   puts stderr "\nWhere"
   puts stderr "    host - is the host on which the manager is running"
   puts stderr "    user - is the user running the manager"
   
   exit -1
}
#------------------------------------------------------------------------------
# entry point

if {[llength $argv] != 2} {
   _usage "Incorrect number of command line parameters"
}

set host [lindex $argv 0]
set user [lindex $argv 1]

StateClient c -host $host -user $user
c kill
