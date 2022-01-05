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

    


# Get the list of ports for that system:

set mgr [portAllocator %AUTO% -hostname $host]
set services [$mgr listPorts]

# Now filter by those that match the userpat username pattern.

set servicelist [list]
foreach service $services {
    set user [lindex $service 2]
    if {[string match $userpat $user]} {
        lappend servicelist $service
    }
}
# service list is now the set of services we're going to
# List in tabular form.

::report::defstyle simpletable {} {
    data set [split "[string repeat "| "   [columns]]|"]
    top set  [split "[string repeat "+ - " [columns]]+"]
    bottom set [top get]
    top    enable 
    bottom enable
}

::report::defstyle captionedtable {{n 1}} {
    simpletable
    topdata    set [data get]
    topcapsep  set [top get]
    topcapsep  enable
    tcaption   $n
}


struct::matrix reportData
reportData add columns 3
reportData insert row 0 [list Service Port user]
foreach service $servicelist {
    reportData insert row end $service
}
::report::report r 3 style captionedtable 1
puts [r printmatrix reportData]
reportData destroy
r destroy


    

