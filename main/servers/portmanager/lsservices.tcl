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
# @file lsservices(.tcl)
# @brief  program to list the services that are advertised by a port manager
# @author Ron Fox <fox@nscl.msu.edu>
# 

if {[array names ::env DAQTCLLIBS] eq ""} {
    puts stderr "The NSCLDAQ environment variables have not been set up."
    puts stderr "source the daqsetup.bash script from the root directory"
    puts stderr "of the NSCLDAQ version you are using."
    exit -1
}
lappend auto_path $::env(DAQTCLLIBS)
package require portAllocator
package require struct::matrix
package require report
##
# Usage:
#    $DAQBIN/lsservices ?host ?username??
#
#
proc usage {msg} {
    puts stderr $msg
    puts stderr "Usage:"
    puts stderr "   \$DAQBIN/lsservices ?host ?username??"
    puts stderr "Where:"
    puts stderr "   host - If supplied is the host whose port manager is queried."
    puts stderr "   username - if suppled is a glob patter that must match the"
    puts stderr "              usernames of services that will be listed."
    
    exit -1
}

#-----------------------------------------------------------------
#  Entry point:
#

# Default of host and userpat:

set host localhost
set userpat *

if {$argc > 0} {
    set host [lindex $argv 0]
}
if {$argc == 2} {
    set userpat [lindex $argv 1]
}
if {$argc ni [list 0 1 2]} {
    usage "Invalid command line parameter count"
}

    
puts $host
puts $userpat
