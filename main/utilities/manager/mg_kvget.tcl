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
# @file   mg_kvget.tcl
# @brief Get the value of a key from the key value store via server.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}

package require kvclient

#------------------------------------------------------------------------------
#  Utilities:
#

##
# usage
#   Output an error message and the program usage.  See this proc to get actual
#   program usage information.
#
# @param msg - error message to lead off with.
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "  \$DAQBIN/mg_kvget host user key"
    puts stderr "Where:"
    puts stderr "   host - is the host a manager server is running on."
    puts stderr "   user - Is the user who started the manager server"
    puts stderr "   key  - Is a variable key."
    puts stderr "The value associated with the key will be output to stdout."
    exit -1
    
}
#-----------------------------------------------------------------------------
# Entry point:

if {[llength $argv] != 3} {
    usage "Invalid number of command line parameters"
}
set host [lindex $argv 0]
set user [lindex $argv 1]
set key [lindex $argv 2]

KvClient c -host $host -user $user
puts [c getValue $key]
c destroy
